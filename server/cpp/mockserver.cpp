
#include <boost/gil/gil_all.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/typedefs.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <math.h>

using namespace std;
using namespace boost;

template <typename P>
struct mandelbrot_fn {
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
  static const int MAX_ITER = 100;        // max number of iterations  

  mandelbrot_fn() {}
  mandelbrot_fn(const point_t& sz, const value_type& in_color, const value_type& out_color) :
	_in_color(in_color), _out_color(out_color), _img_size(sz)
  {
	
  }  

  result_type operator()(const point_t& p) const
  {	
	// normalize the coords to (-2..1, -1.5..1.5)
	double t = get_num_iter(point2<double>(p.x / (double)_img_size.x * 3 - 2, p.y / (double)_img_size.y * 3 - 1.0f));	
	t = pow(t, 0.2);	

	value_type ret;	
	for (int k = 0; k < num_channels<P>::value; ++k) {	  
	  ret[k] = (typename channel_type<P>::type)(_in_color[k] * t + _out_color[k] * (1 - t));	  
	}	
	return ret;	
  }

private:
  double get_num_iter(const point2<double>& p) const
  {	
	point2<double> Z(0, 0);
	
	for (int i = 0; i < MAX_ITER ; ++i) {	  
	  Z = point2<double>(Z.x * Z.x - Z.y * Z.y + p.x, 2 * Z.x * Z.y + p.y);	  
	  if (Z.x * Z.x + Z.y * Z.y > 4)		
		return i / (double)MAX_ITER;	  
	}
	return 0;	
  }
};

int main(int argc, char *argv[])
{
  //  thread thr_hello(&PrintHello);  
  //  thread thr_world(&PrintWorld);  

  //  thr_hello.join();
  //  thr_hello.join();  

  return 0;  
}
