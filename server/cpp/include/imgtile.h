
#include <string>
#include <queue>
#include <deque>
#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include <boost/gil/image_view.hpp>

#include "common.h"

using namespace std;
using namespace boost;
using namespace boost::gil;

class imgtile
{
private:
    string fname;
    int x, y;
    int width, height;
    view_t &view;    
    
public:

    void write_png(void)        
    {        
        png_write_view(fname, subimage_view(view, x, y, width, height));        
    }
    
    imgtile(view_t &view,            
            int x, int y,
            int width, int height)        
    {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        this->view = view;        
        fname = boost::str(boost::format("%d.png") % rand());        
    }    
    
    virtual ~imgtile() {};
    
};

