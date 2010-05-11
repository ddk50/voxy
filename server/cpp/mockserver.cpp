
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

#include "sexpr.h"

using boost::asio::ip::tcp;
using namespace boost::gil;
using namespace boost;
using namespace std;

using boost::posix_time::ptime;
using boost::posix_time::us_dst;
using boost::posix_time::second_clock;

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

typedef shared_ptr<std::deque<string> > mainque_ptr;

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
    int (*function)(mainque_ptr, deque<string>&);    
    const char* command;    
};

typedef boost::date_time::c_local_adjustor<ptime> local_adj;
void logging(string cmdtype)    
{
    ptime t = second_clock::universal_time();    
    ptime tt = local_adj::utc_to_local(t);    

    cout << "[" << tt << "]" << " ACCEPT " << cmdtype        
         << endl;    
}

int cmd_updatetile(mainque_ptr main_que,                   
                   deque<string>& rest_token)    
{
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

int cmd_chngresol(mainque_ptr main_que,                  
                  deque<string>& rest_token)    
{
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

int cmd_mouseevent(mainque_ptr main_que,                   
                   deque<string>& rest_token)    
{
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

int cmd_keyevent(mainque_ptr main_que,                 
                 deque<string>& rest_token)    
{
    logging(boost::str(format("KEYVALUES values: %s") % rest_token[0]));    
    
    rest_token.pop_front();
    printf("test %d\n", rest_token.size());    
    return rest_token.size();    
}

int cmd_vncconnect(mainque_ptr main_que,                   
                   deque<string>& rest_token)    
{    
    string host = rest_token[0];  
    string password = rest_token[1];    

    /* connect vnc */
    logging(boost::str(format("VNCCONNECT host: %s password: %s") % host % password));    

    rest_token.pop_front();    
    rest_token.pop_front();    

    return rest_token.size();    
}

int cmd_vncdisconnect(mainque_ptr main_que,                      
                      deque<string>& rest_token)    
{    
    /* connect vnc */    
    logging(boost::str(format("VNCDISCONNECT")));    
    return rest_token.size();    
}

int cmd_getupdate(mainque_ptr main_que,                  
                  deque<string>& rest_token)    
{    
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

void branching(mainque_ptr main_que,               
               deque<string>& token_list)    
{
    int canonical;        
    try {
    once:
        canonical = 0;	
        BOOST_FOREACH(struct command_map& e, cmap) {            
            if (e.command == token_list[0]) {                
                canonical = 1;                
                token_list.pop_front();                
                if (e.function(main_que, token_list) > 0)                    
                    goto once;                
                break;                
            }            
        }        
    } catch (std::exception& e) {
        cout << "illegal command"<< e.what() << endl;        
        canonical = 0;        
    }
    
    if (!canonical) {
        cout << "unrecognized command: " << token_list[0] << endl;        
        safe_clear(main_que);        
    }    
    
    token_list.clear();    
}


const int max_length = 1024 * 2;
typedef shared_ptr<tcp::socket> socket_ptr;
typedef shared_ptr<sparser> sparser_ptr;

void sendback_mesg(socket_ptr sock,                   
                   mainque_ptr& main_que)    
{
    mutex::scoped_lock lock(thread_sync);
    cout << "size: " << main_que->size() << endl;    
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

void session(socket_ptr sock, mainque_ptr main_que)    
{    
    sparser_ptr sp = sparser_ptr(new sparser());    
    string message_chunk;    

    message_chunk.resize(max_length, ' ');    
  
    try {
        while (1) {
            char  data[max_length];	  
            deque<string>  tokens;            
            boost::system::error_code  error;	  
            size_t length = sock->read_some(asio::buffer(data), error);            

            if (error == asio::error::eof) {		
                break; // Connection closed cleanly by peer.                
            } else if (error) {		
                throw boost::system::system_error(error); // Some other error.                
            }            

            if (data[length - 1] == '\n') {                
                message_chunk.append(data, length); {                    
                    sp->read_expression(message_chunk, tokens);                    
                    branching(main_que, tokens);                    
                }; message_chunk.erase();                
                /* sending message  */                
                sendback_mesg(sock, main_que);                
            } else {                
                message_chunk.append(data, length);                
                continue;                
            }            
        }
    } catch (std::exception& e) {        
        cerr << "Exception in thread: " << e.what() << endl;        
    }    
}

void server(asio::io_service& io_service,            
            short port, mainque_ptr main_que)    
{
    tcp::acceptor acc(io_service, tcp::endpoint(tcp::v4(), port));  
    while (1) {	
        socket_ptr sock(new tcp::socket(io_service));	
        acc.accept(*sock);        
        boost::thread t(boost::bind(session, sock, main_que));        
    }  
}

int start_server(int argc, char *argv[],
                 mainque_ptr main_que)    
{
    try {	
        if (argc != 2) {
            std::cerr << "Usage: blocking_tcp_echo_server <port>\n";	  
            return 0;            
        }        
        boost::asio::io_service io_service;        	
        server(io_service, std::atoi(argv[1]), main_que);        
    } catch (std::exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return 0;        
    }
  
    return 1;    
}

typedef mandelbrot_fn<rgb8_pixel_t>         deref_t;
typedef deref_t::point_t                    point_t;
typedef virtual_2d_locator<deref_t, false>  locator_t;
typedef image_view<locator_t>               my_virt_view_t;

void generator(mainque_ptr main_que)    
{
    function_requires<PixelLocatorConcept<locator_t> >();  
    gil_function_requires<StepIteratorConcept<locator_t::x_iterator> >();

    point_t dims(1280, 720);
    
    while (1) {
        cout << "main" << endl;        
        my_virt_view_t mandel(dims, locator_t(point_t(-2.0, 2.0),
                                              point_t(1.0, 1.7),
                                              deref_t(dims,
                                                      rgb8_pixel_t(255, 160, 0),
                                                      rgb8_pixel_t(0, 0, 0))));        
        // 2..1, -1.5..1.5
        png_write_view("out-mandelbrot.png", mandel);
        cout << "size: " << main_que->size() << endl;        
        safe_pushback(main_que, "(:UPDATETILE \"test.png\" 1 2 3 4)");
        sleep(1);        
    }
}

int start_generator(int argc, char *argv[],
                    mainque_ptr main_que)    
{    
    boost::thread t(boost::bind(generator, main_que));
    return 1;    
}


int main(int argc, char *argv[])  
{
    mainque_ptr main_que(new deque<string>);    

    if (!start_generator(argc, argv, main_que)) {        
        cout << "Could not start generator" << endl;
        exit(-1);        
    }
    
    if (!start_server(argc, argv, main_que)) {
        cout << "Could not start server" << endl;
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
