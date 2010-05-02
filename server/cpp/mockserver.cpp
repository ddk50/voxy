
#include <cstdlib>
#include <iostream>

#include <boost/gil/image.hpp>
#include <boost/gil/typedefs.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/io/png_io.hpp>

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;
using namespace boost::gil;
using namespace boost;

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

const int max_length = 1024;

typedef shared_ptr<tcp::socket> socket_ptr;

void session(socket_ptr sock)
{
  try {
	while (1) {
	  char data[max_length];	  
	  boost::system::error_code  error;	  
	  size_t length = sock->read_some(asio::buffer(data), error);	  
	  
	  if (error == asio::error::eof) {		
		break; // Connection closed cleanly by peer.
	  } else if (error) {		
		throw boost::system::system_error(error); // Some other error.		
	  }
	  
	  asio::write(*sock, asio::buffer(data, length));	  
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
  mtrace();  
  
  function_requires<PixelLocatorConcept<locator_t> >();  
  gil_function_requires<StepIteratorConcept<locator_t::x_iterator> >();

  start_server(argc, argv);  

  //  point_t dims(1280, 720);  
  //  my_virt_view_t mandel(dims, locator_t(point_t(0, 0), point_t(1, 1), deref_t(dims, rgb8_pixel_t(255, 0, 255), rgb8_pixel_t(0, 255, 0))));
  //  my_virt_view_t mandel(dims, locator_t(point_t(-2.0, 2.0), point_t(1.0, 1.7), deref_t(dims, rgb8_pixel_t(255, 160, 0), rgb8_pixel_t(0, 0, 0))));  
  // 2..1, -1.5..1.5
  
  //  png_write_view("out-mandelbrot.png", mandel);

  muntrace();  

  return 0;
}
