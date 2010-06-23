
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

typedef mandelbrot_fn<rgb8_pixel_t>         deref_t;
typedef deref_t::point_t                    point_t;
typedef virtual_2d_locator<deref_t, false>  locator_t;
typedef image_view<locator_t>               my_virt_view_t;
typedef shared_ptr<imgtile*>                imgtile_ptr;

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

#endif
