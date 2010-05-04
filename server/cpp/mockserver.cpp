
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

#include "sexpr.h"

using boost::asio::ip::tcp;
using namespace boost::gil;
using namespace boost;
using namespace std;

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

struct command_map
{  
  int   (*function)(deque<string>&);  
  const char*   command;  
};

int cmd_updatetile(deque<string>& rest_token)  
{  
  int x = lexical_cast<int>(rest_token[0]);
  int y = lexical_cast<int>(rest_token[1]);
  int w = lexical_cast<int>(rest_token[2]);
  int h = lexical_cast<int>(rest_token[3]);  
  
  cout << boost::format("ACCEPT UPDATETILE (x, y, w, h) = (%d, %d, %d, %d)") % x % y % w % h
	   << endl;
  
  rest_token.pop_front();  
  rest_token.pop_front();
  rest_token.pop_front();
  rest_token.pop_front();  
						
  return rest_token.size();  
}

int cmd_chngresol(deque<string>& rest_token)  
{
  int w = lexical_cast<int>(rest_token[0]);
  int h = lexical_cast<int>(rest_token[1]);
  
  cout << boost::format("ACCEPT CHNGRESOL (w, h) = (%d, %d)") % w % h
	   << endl;  

  rest_token.pop_front();
  rest_token.pop_front();
  
  return rest_token.size();  
}

int cmd_mouseevent(deque<string>& rest_token)  
{
  int x = lexical_cast<int>(rest_token[0]);
  int y = lexical_cast<int>(rest_token[1]);
  
  cout << boost::format("ACCEPT MOUSEEVENT (x, y) = (%d, %d)") % x % y
	   << endl;  

  rest_token.pop_front();
  rest_token.pop_front();  
  
  return rest_token.size();  
}

int cmd_keyevent(deque<string>& rest_token)  
{
  cout << "ACCEPT KEYEVENT" << " keyvalues: "	
	   << rest_token[0] << endl;
  rest_token.pop_front();  
  return rest_token.size();  
}

struct command_map cmap[] = {
  { cmd_updatetile, ":UPDATATILE" },  
  { cmd_chngresol,  ":CHNGRESOL"  },
  { cmd_mouseevent, ":MOUSEEVENT" },
  { cmd_keyevent,   ":KEYEVENT"   },  
};

void branching(deque<string>& token_list)  
{
  int canonical = 0;  
 once:  
  BOOST_FOREACH(struct command_map& e, cmap) {	
	if (e.command == token_list[0]) {
	  canonical = 1;	  
	  token_list.pop_front();	  
	  if (e.function(token_list) > 0)		
		goto once;	  
	}	
  }

  if (!canonical) {
	cout << "unrecognized command: " << token_list[0] << endl;	
  }  
}

const int max_length = 1024 * 2;
const char *donemsg  = "(:DONE)\n";
typedef shared_ptr<tcp::socket> socket_ptr;
typedef shared_ptr<sparser> sparser_ptr;

void session(socket_ptr sock)  
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
		  branching(tokens);
		}; message_chunk.erase();		
		asio::write(*sock, asio::buffer(donemsg, strlen(donemsg)));		
	  } else {
		message_chunk.append(data, length);
		continue;		
	  }	  	 
	}
  } catch (std::exception& e) {	
	std::cerr << "Exception in thread: " << e.what() << "\n";
  }  
}

void server(asio::io_service& io_service, short port)  
{
  tcp::acceptor acc(io_service, tcp::endpoint(tcp::v4(), port));  
  while (1) {	
	socket_ptr sock(new tcp::socket(io_service));
	acc.accept(*sock);	
	boost::thread t(boost::bind(session, sock));	
  }  
}

int start_server(int argc, char *argv[])
{
  try {	
	if (argc != 2) {
	  std::cerr << "Usage: blocking_tcp_echo_server <port>\n";	  
	  return 1;	  
	}	
	boost::asio::io_service io_service;
	
	server(io_service, std::atoi(argv[1]));
  } catch (std::exception& e) {
	std::cerr << "Exception: " << e.what() << "\n";	
  }
  
  return 0;  
}

typedef mandelbrot_fn<rgb8_pixel_t>         deref_t;
typedef deref_t::point_t                    point_t;
typedef virtual_2d_locator<deref_t, false>  locator_t;
typedef image_view<locator_t>               my_virt_view_t;

int main(int argc, char *argv[])  
{  
  function_requires<PixelLocatorConcept<locator_t> >();  
  gil_function_requires<StepIteratorConcept<locator_t::x_iterator> >();
  
  start_server(argc, argv);  

  //  point_t dims(1280, 720);  
  //  my_virt_view_t mandel(dims, locator_t(point_t(0, 0), point_t(1, 1), deref_t(dims, rgb8_pixel_t(255, 0, 255), rgb8_pixel_t(0, 255, 0))));
  //  my_virt_view_t mandel(dims, locator_t(point_t(-2.0, 2.0), point_t(1.0, 1.7), deref_t(dims, rgb8_pixel_t(255, 160, 0), rgb8_pixel_t(0, 0, 0))));  
  // 2..1, -1.5..1.5  
  
  //  png_write_view("out-mandelbrot.png", mandel);

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
