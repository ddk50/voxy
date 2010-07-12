
#ifndef _COMMON_H_
#define _COMMON_H_

#include <cstdlib>
#include <iostream>
#include <deque>

#include <boost/gil/image.hpp>
#include <boost/gil/typedefs.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/io/png_io.hpp>

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

#include "vncclient.hpp"
#include "ImgTile.hpp"
#include "sexpr.hpp"

using boost::asio::ip::tcp;
using namespace boost::gil;
using namespace boost;
using namespace std;

#define IMAGE_WIDTH  1280
#define IMAGE_HEIGHT 720

/* Models a Unary Function */
template <typename P>   /* Models PixelValueConcept */
struct mandelbrot_fn
{  
    typedef point2<ptrdiff_t>    point_t;
    typedef mandelbrot_fn        const_t;
    typedef P                    value_type;
    typedef value_type           reference;
    typedef value_type           const_reference;
    typedef point_t              argument_type;
    typedef reference            result_type;
  
    BOOST_STATIC_CONSTANT(bool, is_mutable = false);  
  
    value_type                    _in_color,_out_color;
    point_t                       _img_size;  
  
    static const int MAX_ITER = 100;  /* max number of iterations */ 

    mandelbrot_fn() {}
    mandelbrot_fn(const point_t& sz, const value_type& in_color, const value_type& out_color) :
        _in_color(in_color), _out_color(out_color), _img_size(sz){}  

    result_type operator()(const point_t& p) const        
    {        
        // normalize the coords to (-2..1, -1.5..1.5)
        // (actually make y -1.0..2 so it is asymmetric, so we can verify some view factory methods)
        double t = get_num_iter(point2<double>(p.x / (double)_img_size.x * 3.0 - 2,
                                               p.y / (double)_img_size.y * 3.0 - 1.0f));//1.5f));	
        t = pow(t, 0.2);	

        value_type ret;	
        for (int k = 0 ; k < num_channels<P>::value ; ++k) {	  
            ret[k] = (typename channel_type<P>::type)(_in_color[k]*t + _out_color[k]*(1-t));            
        }
	
        return ret;
    }
    
private:
    double get_num_iter(const point2<double>& p) const
    {        
        point2<double> Z(0, 0);	
        for (int i = 0 ; i < MAX_ITER ; ++i) {	  
            Z = point2<double>(Z.x*Z.x - Z.y*Z.y + p.x, 2.0*Z.x*Z.y + p.y);	  
            if (Z.x * Z.x + Z.y * Z.y > 4.0) {		
                return i / (double)MAX_ITER;                
            }	  
        }
        return 0;	
    }
};

typedef mandelbrot_fn<rgb8_pixel_t>         deref_t;
typedef deref_t::point_t                    point_t;
typedef virtual_2d_locator<deref_t, false>  locator_t;
typedef image_view<locator_t>               my_virt_view_t;

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
    
    bool vncconnected;
    bool incremental;
    bool sockconnected;    
    
public:
    session(asio::io_service& io_service)        
    {
        main_que = mainque_ptr(new deque<string>);
        sock = socket_ptr(new tcp::socket(io_service));        
        vncconnected = false;
        incremental = false;
        sockconnected = true;        
    }
    ~session() { VNCdisconnect(); }

    void safe_popfront(void)        
    {        
        mutex::scoped_lock lock(thread_sync);
        main_que->pop_front();        
    }

    void safe_clear(void)        
    {
        mutex::scoped_lock lock(thread_sync);    
        main_que->clear();        
    }    

    void safe_pushback(string str)        
    {        
        mutex::scoped_lock lock(thread_sync);    
        main_que->push_back(str);        
    }    

    void safe_pushback(const char *str)        
    {
        mutex::scoped_lock lock(thread_sync);
        string t = str;    
        main_que->push_back(t);        
    }    
    
    void DisplayLoop(void)        
    {        
        while (vncconnected) {            
            if (incremental) {                
                vncclient->sendIncrementalFramebufferUpdateRequest();                
            } else {
                notice("started VNC main loop");                
                vncclient->sendFramebufferUpdateRequest(0, 0, vncclient->fbWidth, vncclient->fbHeight, 0);                
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
                                 % tgt_tile->get_filename()
                                 % tgt_tile->xpos  % tgt_tile->ypos
                                 % tgt_tile->width % tgt_tile->height);                
                main_que->push_back(val);                
                ++i;                
            }        
        }  
        return i;        
    }    
    
    bool VNCConnect(string host, int port, string password)        
    {        
        if (vncconnected)
            return false;
        
        vncclient = VNCClient_ptr(new VNCClient(host.c_str(), port, "./passfile"));        
        vncconnected = vncclient->VNCInit();        

        if (vncconnected) {
            alloctile(vncclient->fbWidth, vncclient->fbHeight, 24, 0);            
            incremental = false;            
        }               
        return vncconnected;        
    }

    void VNCdisconnect(void)
    {        
        if (vncconnected) {            
            vncclient->VNCClose();
            vncconnected = false;
            safe_clear();            
        }        
    }
};

typedef shared_ptr<session>  session_ptr;

#endif
