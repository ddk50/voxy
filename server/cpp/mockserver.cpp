
#include <cstdlib>
#include <iostream>
#include <deque>

#include <boost/gil/image.hpp>
#include <boost/gil/typedefs.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/io/png_io.hpp>

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include <boost/math/common_factor.hpp>

#include <string.h>

#include "sexpr.hpp"
#include "global.hpp"
#include "common.hpp"

class imgtile
{
private:
    my_virt_view_t view;

public:
    string fname;
    int x, y;
    int width, height;
    
public:
    void write_png(void)        
    {        
        png_write_view(fname, subimage_view(view, x, y, width, height));        
    }
    
    imgtile(my_virt_view_t view,            
            int x, int y,
            int width, int height)        
    {
        this->x = x; this->y = y;
        this->width = width; this->height = height;        
        this->view = view;        
        fname = boost::str(boost::format("%d.png") % rand());        
    }        
    ~imgtile() {};    
};

typedef shared_ptr<imgtile> imgtile_ptr;

static mutex thread_sync;

void safe_popfront(mainque_ptr deque)    
{
    mutex::scoped_lock lock(thread_sync);
    deque->pop_front();    
}

void safe_pushback(mainque_ptr deque,                   
                   string str)    
{
    mutex::scoped_lock lock(thread_sync);    
    deque->push_back(str);    
}

void safe_pushback(mainque_ptr deque,                   
                   const char *str)    
{
    mutex::scoped_lock lock(thread_sync);
    string t = str;    
    deque->push_back(t);    
}

void safe_clear(mainque_ptr deque)    
{
    mutex::scoped_lock lock(thread_sync);    
    deque->clear();    
}

struct command_map
{  
    int (*function)(session_ptr&);    
    const char* command;    
};

int cmd_updatetile(session_ptr &s)    
{
    deque<string>& rest_token = s->rest_token;    
    
    int x = lexical_cast<int>(rest_token[0]);    
    int y = lexical_cast<int>(rest_token[1]);
    int w = lexical_cast<int>(rest_token[2]);
    int h = lexical_cast<int>(rest_token[3]);    
    
    logging(boost::str(format("UPDATETILE (x, y, w, h) = (%d, %d, %d, %d)") % x % y % w % h));    
  
    rest_token.pop_front();  
    rest_token.pop_front();
    rest_token.pop_front();
    rest_token.pop_front();    
  
    return rest_token.size();    
}

int cmd_chngresol(session_ptr &s)    
{
    deque<string>& rest_token = s->rest_token;    
    
    if (rest_token.size() >= 2) {	  
        int w = lexical_cast<int>(rest_token[0]);
        int h = lexical_cast<int>(rest_token[1]);
  
        logging(boost::str(format("CHNGRESOL (w, h) = (%d, %d)") % w % h));        
        
        rest_token.pop_front();
        rest_token.pop_front();        
    } else {
        throw "too few argument for chngresol\n";        
    }
  
    return rest_token.size();    
}

int cmd_mouseevent(session_ptr &s)    
{
    deque<string>& rest_token = s->rest_token;    
    
    if (rest_token.size() >= 2) {	
        int x = lexical_cast<int>(rest_token[0]);
        int y = lexical_cast<int>(rest_token[1]);
	
        logging(boost::str(format("MOUSEEVENT (x, y) = (%d, %d)") % x % y));        

        rest_token.pop_front();
        rest_token.pop_front();        
	
        return rest_token.size();        
    } else {
        throw "too few argument for chngresol\n";        
    }
}

int cmd_keyevent(session_ptr &s)    
{
    deque<string>& rest_token = s->rest_token;    
    
    logging(boost::str(format("KEYVALUES values: %s") % rest_token[0]));    
    
    rest_token.pop_front();
    trace(DBG_INIT, "test %d\n", rest_token.size());    
    return rest_token.size();    
}

int cmd_vncconnect(session_ptr &s)    
{
    deque<string>& rest_token = s->rest_token;    
    string hoststr = rest_token[0];    
    string password = rest_token[1];
    char buffer[1000];
    const char *delim = ":";    
    char *ctx;    
    char *host_ptr, *port_ptr;

    std::strcpy(buffer, hoststr.c_str());

    host_ptr = strtok_r(buffer, delim, &ctx);    
    port_ptr = strtok_r(NULL, delim, &ctx);    

    string host(host_ptr);    
    int port = lexical_cast<int>(port_ptr);

    logging("VNCCONNECT ... ");    

    if (s->VNCConnect(host, port, password)) {
        /* connect vnc */
        logging(boost::str(format("  establish VNCCONNECT (host: %s, port: %d, password: %s)")
                           % host % port % password));        
    } else {
        logging(boost::str(format("  Could not establish VNCCOONECT (host: %s, port: %d, password: %s)")
                           % host % port % password));        
    }    

    rest_token.pop_front();    
    rest_token.pop_front();    

    return rest_token.size();    
}

int cmd_vncdisconnect(session_ptr &s)    
{
    deque<string>& rest_token = s->rest_token;    

    s->VNCdisconnect();    
    
    /* connect vnc */    
    logging(boost::str(format("VNCDISCONNECT")));    
    
    return rest_token.size();    
}

int cmd_getupdate(session_ptr &s)    
{
    deque<string>& rest_token = s->rest_token;
    VNCClient_ptr vnc;    
    
    if (!s->vncconnected) {        
        logging(boost::str(format("GETUPDATE Nothing to do")));        
        goto getout;
    } else {
        logging(boost::str(format("GETUPDATE")));        
    }    

 getout:    
    return rest_token.size();    
}

static struct command_map cmap[] = {   
    { cmd_getupdate,  ":GETUPDATA"  },    
    { cmd_updatetile, ":UPDATATILE" },  
    { cmd_chngresol,  ":CHNGRESOL"  },
    { cmd_mouseevent, ":MOUSEEVENT" },
    { cmd_keyevent,   ":KEYEVENT"   },
    { cmd_vncconnect, ":VNCCONNECT" },
    { cmd_vncdisconnect, ":VNCDISCONNECT" },    
};

void branching(session_ptr &s)    
{
    int canonical;
    
    try {
    once:
        canonical = 0;	
        BOOST_FOREACH(struct command_map& e, cmap) {            
            if (e.command == s->rest_token[0]) {                
                canonical = 1;                
                s->rest_token.pop_front();                
                if (e.function(s) > 0)                    
                    goto once;                
                break;                
            }            
        }        
    } catch (std::exception& e) {
        cout << "illegal command"<< e.what() << endl;        
        canonical = 0;        
    }
    
    if (!canonical) {
        error("unrecognized command: %s", s->rest_token[0].c_str());        
        safe_clear((*s).main_que);        
    }
    
    s->rest_token.clear();    
}


const int max_length = 1024 * 2;

/*  
void sendback_mesg(socket_ptr sock,                   
                   mainque_ptr& main_que)    
{
    mutex::scoped_lock lock(thread_sync);
    if (main_que->size() > 0) {        
        main_que->push_back("\n");        
        BOOST_FOREACH(string& v, *main_que) {            
            asio::write(*sock, asio::buffer(v.c_str(), strlen(v.c_str())));            
        }        
    } else {
        string ret = "(:DONE)\n";        
        asio::write(*sock, asio::buffer(ret.c_str(), strlen(ret.c_str())));        
    }
    main_que->clear();    
}
*/

void socksession(session_ptr& s)
{    
    sparser_ptr sp = sparser_ptr(new sparser());    
    string message_chunk;
    char  data[max_length];    

    message_chunk.resize(max_length, ' ');
    
    try {        
        while (s->sockconnected) {            
            boost::system::error_code  error;	  
            size_t length = s->sock->read_some(asio::buffer(data), error);            

            if (error == asio::error::eof) {		
                break; // Connection closed cleanly by peer.                
            } else if (error) {		
                throw boost::system::system_error(error); // Some other error.                
            }
            
            if (data[length - 1] == '\n') {                
                message_chunk.append(data, length); {                    
                    sp->read_expression(message_chunk, s->rest_token);                    
                    branching(s);                    
                }; message_chunk.erase();                
                /* sending message  */                
                //                sendback_mesg(sock, main_que);                
            } else {                
                message_chunk.append(data, length);                
                continue;                
            }            
        }        
    } catch (std::exception& e) {        
        cerr << "Exception in thread: " << e.what() << endl;        
    }
    s->sockconnected = false;    
}

void vncsession(session_ptr& s)    
{
    while (s->sockconnected) {        
        s->DisplayLoop();        
    }    
}

void server(asio::io_service& io_service, short port)    
{    
    tcp::acceptor acc(io_service, tcp::endpoint(tcp::v4(), port));
    
    while (1) {
        session_ptr s = session_ptr(new session(io_service));        
        acc.accept(*((*s).sock));        
        boost::thread sock_thread(boost::bind(socksession, s));        
        boost::thread vnc_thread(boost::bind(vncsession, s));        
    }    
}

int start_server(int argc, char *argv[])    
{
    try {	
        if (argc != 2) {
            error("Usage: blocking_tcp_echo_server <port>");            
            return 0;            
        }        
        boost::asio::io_service io_service;        	
        server(io_service, std::atoi(argv[1]));        
    } catch (std::exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return 0;        
    }
  
    return 1;    
}

void generator(mainque_ptr main_que)    
{
    function_requires<PixelLocatorConcept<locator_t> >();  
    gil_function_requires<StepIteratorConcept<locator_t::x_iterator> >();
    point_t dims(IMAGE_WIDTH, IMAGE_HEIGHT);    

    mt19937             gen(static_cast<unsigned long>(time(0)));    
    uniform_smallint<>  dst(1, 30);
    variate_generator<mt19937&, uniform_smallint<> > rand(gen, dst);    
    
    int c_width  = boost::math::gcd(IMAGE_WIDTH, IMAGE_HEIGHT);    
    int c_height = boost::math::gcd(IMAGE_WIDTH, IMAGE_HEIGHT);

    cout << boost::format("tile width: %d\ntile height: %d\n") % c_width % c_height;    

    deque<imgtile_ptr>  imglist;    
    my_virt_view_t      mandel(dims, locator_t(point_t(-2.0, 2.0),
                                               point_t(1.0, 1.7),
                                               deref_t(dims,
                                                       rgb8_pixel_t(255, 160, 0),
                                                       rgb8_pixel_t(0, 0, 0))));    

    for (int x = 0 ; x < IMAGE_WIDTH ; x += c_width) {        
        for (int y = 0 ; y < IMAGE_HEIGHT ; y += c_height) {            
            cout << boost::format("tile [%d, %d]\n") % (x / c_width) % (y / c_height);            
            imglist.push_back(imgtile_ptr(new imgtile(mandel, x, y, c_width, c_height)));            
        }        
    }
    
    while (1) {        
        BOOST_FOREACH(imgtile_ptr& v, imglist) {            
            (*v).write_png();
            safe_pushback(main_que,
                          boost::str(boost::format("(:UPDATETILE \"%s\" %d %d %d %d)")
                                     % (*v).fname % (*v).x % (*v).y % (*v).width % (*v).height));            
        }        
        sleep(1);        
    }    
}

#ifdef __DEBUG
int start_generator(int argc, char *argv[],
                    mainque_ptr main_que)    
{    
    boost::thread t(boost::bind(generator, main_que));    
    return 1;    
}
#endif

int main(int argc, char *argv[])
{

#ifdef __DEBUG    
    /*      
    if (!start_generator(argc, argv, main_que)) {        
        error("Could not start generator");        
        exit(-1);        
    }
    */
#endif   
    
    if (!start_server(argc, argv)) {        
        error("Could not start server");        
        exit(-1);        
    }

    return 0;    
}

void test()  
{
    sparser_ptr sp = sparser_ptr(new sparser());  
    deque<string> tokens;  
    string test_pattern =	
        "(:UPDATETILE \"test.png\" 1 2 3 4)\n"	
        "(:CHNGRESOL 5 6)\n"	
        "(:MOUSEEVENT x y btnnum)\n"	
        "(:KEYEVENT \"hello world\")\n"	
        "(:VNCCONNECT \"tertes.homelinux.com\" \"hogefoo\")\n"	
        "(:VNCDISCONNECT)\n";    
  
    sp->read_expression(test_pattern, tokens);  
    BOOST_FOREACH(string& v, tokens) {
        cout << v << endl;        
    }  
}

/*  
  {
  view_t cropped(subimage_view(view(lowpass), CROP_X, CROP_Y, CROP_W, CROP_H));
  const view_t& v(cropped);
  color_converted_view_type<view_t, gray8_pixel_t, convert_to_gray8>::type ccv(color_converted_view<gray8_pixel_t, view_t, convert_to_gray8>(v, convert_to_gray8()));

  ProgressDisp d("writing preprocessed output");
  png_write_view("lowpass.png", ccv);
  }  
*/

/*
class server
{
private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    boost::threadpool::pool tp;
    
public:
    server(boost::asio::io_service& io_service, short port) : io_service_(io_service),
          acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
          tp(4)                                                              
    {
        shared_ptr<session> new_session(new session(io_service_));
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&server::handle_accept, this,
                                           new_session,
                                           boost::asio::placeholders::error));        
    }

    void handle_accept(shared_ptr<session> current_session, const
                       boost::system::error_code& error)
    {        
        if (error)            
            return;        

        tp.schedule(boost::bind(&session::processRequest,current_session));        

        shared_ptr<session> new_session(new session(io_service_));
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&server::handle_accept, this,
                                           new_session,
                                           boost::asio::placeholders::error));        
    }
};
*/
