
#ifndef _COMMON_H_
#define _COMMON_H_

#include <cstdlib>
#include <iostream>
#include <deque>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/time_facet.hpp>

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include <boost/math/common_factor.hpp>
#include <boost/thread/condition.hpp>

#include <X11/Xlib.h>

#include "vncclient.hpp"
#include "ImgTile.hpp"
#include "sexpr.hpp"

using boost::asio::ip::tcp;
using namespace boost;
using namespace std;

#define IMAGE_WIDTH  1280
#define IMAGE_HEIGHT 720

typedef shared_ptr<deque<string> >          mainque_ptr;
typedef shared_ptr<VNCClient>               VNCClient_ptr;

typedef shared_ptr<tcp::socket> socket_ptr;
typedef shared_ptr<sparser> sparser_ptr;

class session : public MosaicMan
{
public:
    mainque_ptr main_que;
    socket_ptr sock;
    VNCClient_ptr vncclient;    
    
    deque<string> rest_token;

    mutex thread_sync;
    condition thread_cond;    

    bool sockconnected;    
    bool vncconnected;    
    
    bool incremental;
    
public:
    session(asio::io_service& io_service)        
    {
        main_que = mainque_ptr(new deque<string>);
        sock = socket_ptr(new tcp::socket(io_service));        
        incremental = false;
        vncconnected = false;
        sockconnected = true;        
    }
    ~session() { VNCdisconnect(); }
    
    void DisplayLoop(void)        
    {        
        while (vncconnected) {            
            if (incremental) {                
                vncclient->sendIncrementalFramebufferUpdateRequest();                
            } else {
                notice("started VNC main loop");                
                vncclient->sendFramebufferUpdateRequest(0, 0, vncclient->fbWidth,
                                                        vncclient->fbHeight, false);                
                incremental = true;                
            }            
            
            if (vncclient->handleRFBServerMessage()) {                
                switch(vncclient->srvmsg) {                    
                case rfbFramebufferUpdate :                    
                    {                        
                        int x, y;
                        int w, h;                        
                        x = vncclient->recv_rect.r.x;
                        y = vncclient->recv_rect.r.y;                        
                        w = vncclient->recv_rect.r.w - vncclient->recv_rect.r.x;
                        h = vncclient->recv_rect.r.h - vncclient->recv_rect.r.y;
                        update_mosaic(x, y, w, h, vncclient->framebuffer, x, y);
                        export_mosaic();                        
                    }                    
                }                
            } else {                
                trace(DBG_VNC, " disconnect vnc\n");                
            }            
        }        
    }

    int export_mosaic(void)        
    {
        mutex::scoped_lock lock(thread_sync);        
        pImgTile tgt_tile;
        string val;        
        int	i = 0;        
  
        for (int y = 0 ; y < ytile_cnt ; ++y) {
            for (int x = 0 ; x < xtile_cnt ; ++x) {
                tgt_tile = tile[y][x];
                tgt_tile->expose();                
                val = boost::str(format("(:UPDATETILE \"%s\" %d %d %d %d)")                                 
                                 % tgt_tile->get_onlyfname()
                                 % tgt_tile->xpos  % tgt_tile->ypos
                                 % tgt_tile->width % tgt_tile->height);                
                main_que->push_back(val);                
                ++i;                
            }        
        }  
        return i;        
    }

    void SendKey(int key, int flag)        
    {
        KeySym ks = key;
        
        if (!vncconnected)
            return;                
        vncclient->rfbproto.sendKeyEvent(ks, flag);        
    }

    void MouseEvent(int x, int y, int btnmask)        
    {
        if (!vncconnected)            
            return;        
        vncclient->rfbproto.sendPointerEvent(x, y, btnmask);        
    }    
    
    bool VNCConnect(string host, int port, string password)        
    {        
        if (vncconnected)
            return false;

        try {            
            vncclient = VNCClient_ptr(new VNCClient(host.c_str(), port, "./passfile"));        
            vncconnected = vncclient->VNCInit();            
        } catch (std::exception& e) {
            cout << "VNCConnect: " << e.what() << endl;            
        }

        if (vncconnected) {
            alloctile(vncclient->fbWidth, vncclient->fbHeight, 24, 0);
            incremental = false;            
        }               
        return vncconnected;        
    }

    void VNCdisconnect(void)
    {
        if (vncconnected) {            
            vncconnected = false;            
            vncclient->VNCClose();            
            main_que->clear();            
        }        
    }    
};

typedef shared_ptr<session>  session_ptr;

#endif
