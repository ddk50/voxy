
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
        notice("too few argument for chngresol\n");        
    }
  
    return rest_token.size();    
}

int cmd_mouseevent(session_ptr &s)    
{
    deque<string>& rest_token = s->rest_token;    
    
    if (rest_token.size() >= 3) {        
        int x = lexical_cast<int>(rest_token[0]);
        int y = lexical_cast<int>(rest_token[1]);
        int btnst = lexical_cast<int>(rest_token[2]);        
	
        logging(boost::str(format("MOUSEEVENT (x, y, state) = (%d, %d, %d)") % x % y % btnst));

        s->MouseEvent(x, y, btnst);        

        rest_token.pop_front();
        rest_token.pop_front();
        rest_token.pop_front();        
	
        return rest_token.size();        
    } else {
        notice("too few argument for mouseevent\n");        
    }

    return rest_token.size();    
}

int cmd_keyevent(session_ptr &s)    
{
    deque<string>& rest_token = s->rest_token;
    int key;    

    if (rest_token.size() >= 1) {
        key = lexical_cast<int>(rest_token[0]);        
        logging(boost::str(format("KEYVALUES values: %d") % key));
        s->SendKey(key, 1);        
        rest_token.pop_front();        
    } else {
        notice("too few argument for keyevent\n");        
    }
    
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

    if (rest_token.size() >= 2) {        
        std::strcpy(buffer, hoststr.c_str());        

        host_ptr = strtok_r(buffer, delim, &ctx);    
        port_ptr = strtok_r(NULL, delim, &ctx);

        assert(host_ptr != NULL);
        assert(port_ptr != NULL);        

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
    } else {
        notice("too few argument for vncconnect");        
    }    
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
        s->safe_clear();        
    }
    
    s->rest_token.clear();    
}


const int max_length = 1024 * 2;

void sendback_mesg(session_ptr& s)    
{
    mutex::scoped_lock lock(s->thread_sync);    
    if (s->main_que->size() > 0) {        
        s->main_que->push_back("\n");        
        BOOST_FOREACH(string& v, *(s->main_que)) {            
            asio::write(*(s->sock), asio::buffer(v.c_str(), strlen(v.c_str())));            
        }        
    } else {
        string ret = "(:DONE)\n";        
        asio::write(*(s->sock), asio::buffer(ret.c_str(), strlen(ret.c_str())));        
    }
    s->main_que->clear();    
}

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
                logging("Connection closed cleanly by peer");                
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
                sendback_mesg(s);                
            } else {                
                message_chunk.append(data, length);                
                continue;                
            }                
        }
    } catch (std::exception& e) {
        cerr << "Exception in thread: " << e.what() << endl;        
    }
    s->sockconnected = false;
    s->VNCdisconnect();    
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

int main(int argc, char *argv[])
{    
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

