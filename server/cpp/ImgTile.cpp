
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <png.h>
#include <assert.h>

#include <boost/smart_ptr.hpp>
#include <boost/format.hpp>
#include <boost/math/common_factor.hpp>

#include "ImgTile.hpp"
#include "genrand.hpp"
#include "global.hpp"
#include "config.h"

#define PNGFILE_PATH DATADIR "/voxy/pub_www/pics"

using namespace boost;
using namespace std;

/*
 * ImgTile 
 */
ImgTile::ImgTile(int x, int y, int width, int height, int bits_per_pixel)
{  
    char tmp[100];
  
    this->xpos			= x;
    this->ypos			= y;
    this->width			= width;
    this->height		 = height;
    this->bits_per_pixel = bits_per_pixel;    
  
    this->dirty	= 0;
    this->imgbuf = NULL;    
  
    this->imgbuf = new unsigned char*[this->height];    
    for (int y = 0; y < this->height ; ++y) {
        this->imgbuf[y] = new unsigned char[this->width * (this->bits_per_pixel >> 3)];
    }

    gen_id();    

    snprintf(tmp, sizeof(tmp) - 1, "  [%4d, %4d] ( %d ) bits_per_pixel: %d",
             this->xpos, this->ypos, this->id, this->bits_per_pixel);
  
    std::cout << tmp << "\n";

    fp = NULL;
    
    init_shadowfname();    
}

ImgTile::~ImgTile()
{
    
    if (this->imgbuf != NULL) {
        for (int i = 0 ; i < this->width ; ++i) {
            delete[] this->imgbuf[i];
        }
        delete[] this->imgbuf;
    }
  
    if(this->fp != NULL){
        fclose(this->fp);
    }

    unlink(shadow_fname);    
}

//in[y + size->height][x + size->width]
void ImgTile::update_tile(int offset_x, int offset_y,
						  int width, int height,
						  unsigned char **src, int src_x, int src_y)
{
    int	u_w, u_h;
    int	mul_val = this->bits_per_pixel >> 3;
    int	e_x, e_y;
    int	x = 0, y = 0;    
    int	s_x, s_y;    
  
    if (width >= this->width) {
        u_w = this->width;
    } else {
        u_w = width;
    }

    if (height >= this->height) {
        u_h = this->height;
    } else {
        u_h = height;
    }

    e_x = offset_x + u_w;
    e_y = offset_y + u_h;
  
    for (y = offset_y, s_y = src_y ; y < e_y ; ++y, ++s_y) {
        for (x = offset_x * mul_val, s_x = src_x * mul_val;
             x < (e_x * mul_val) ;
             x += mul_val, s_x += mul_val) {
            this->imgbuf[y][x + 0] = src[s_y][s_x + 0];
            this->imgbuf[y][x + 1] = src[s_y][s_x + 1];
            this->imgbuf[y][x + 2] = src[s_y][s_x + 2];            
        }
    }
  
    printf(" offset_x: %d, offset_x: %d, x: %d, y: %d\n",
           offset_x, offset_y, x, y);
  
    this->dirty = 1;
  
}


//override for VNCRGB
//in[y + size->height][x + size->width]
void ImgTile::update_tile(int offset_x, int offset_y,
						  int width, int height,
						  VNCRGB *src, int src_x, int src_y,
						  int root_w, int root_h)    
{
    int	u_w, u_h;
    int	mul_val = this->bits_per_pixel >> 3;
    int	e_x, e_y;
    int	x, y;
    int	s_x, s_y;
  
    if (width >= this->width) {
        u_w = this->width;
    } else {
        u_w = width;
    }   
  
    if (height >= this->height) {
        u_h = this->height;
    } else {
        u_h = height;
    }

    e_x = offset_x + u_w;
    e_y = offset_y + u_h;

    for (y = offset_y, s_y = src_y ; y < e_y ; ++y, ++s_y) {
        for (x = offset_x * mul_val, s_x = src_x ; x < (e_x * mul_val) ; x += mul_val, ++s_x) {
            this->imgbuf[y][x + 0] = src[(s_y * root_w) + s_x].Red;
            this->imgbuf[y][x + 1] = src[(s_y * root_w) + s_x].Green;
            this->imgbuf[y][x + 2] = src[(s_y * root_w) + s_x].Blue;
        }
    }
  
    //printf(" offset_x: %d, offset_x: %d, e_x: %d, e_y: %d\n",
    //		 offset_x, offset_y, e_x, e_y);

    dirty = 1;    
}

int ImgTile::expose(void)    
{
    if (dirty) {        
        gen_filename(filename);        
        openfile(filename, &fp);        
        write_png(fp); /* bug point */        
        fclose(fp);        
	
        swap();
	
        fp = NULL;        
        dirty = 0;        
        return 1;        
    }

    return 0;
}

void ImgTile::openfile(char *fname, FILE **out)
{    
    *out = fopen(fname, "wb");
    assert(*out != NULL);    
}

void ImgTile::swap(void)    
{    
    rename(filename, shadow_fname);    
}

void ImgTile::write_png(FILE *out)
{    
    png_structp     png_ptr;
    png_infop       info_ptr;
  
    //fseek(this->fp, 0, SEEK_SET);
    
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                      NULL, NULL, NULL);    
    info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, out);
  
    //   png_set_IHDR(png_ptr, info_ptr, width, height,
    // 			   8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
    // 			   PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  
    png_set_IHDR(png_ptr, info_ptr, this->width, this->height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  
    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, this->imgbuf);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
  
  
    return;
}

void ImgTile::init_shadowfname(void)    
{
    GenRand *rand = GenRand::instance();    
    int val = rand->gennum();    
  
    snprintf(shadow_fname, MAX_NAME - 1, "%s/%d.png", PNGFILE_PATH, val);
    snprintf(shadow_fnameonly, MAX_NAME - 1, "%d.png", val);    
}

void ImgTile::gen_filename(char *out)
{  
    GenRand *rand = GenRand::instance();
    int val = rand->gennum();    
  
    snprintf(out, MAX_NAME - 1, "%s/%d.png", PNGFILE_PATH, val);    
}

void ImgTile::gen_id(void)    
{   
    GenRand *rand = GenRand::instance();    
    id = rand->gennum();    
}

char *ImgTile::get_filename(void)
{    
    return shadow_fname;    
}

char *ImgTile::get_onlyfname(void)
{
    return shadow_fnameonly;    
}

MosaicMan::MosaicMan(int canvas_width, int canvas_height,
					 int bits_per_pixel, int blocklen)
{    
    trace(DBG_VNC,
          " MosaicMan: width = %d, height = %d\n, bits per pixel = %d\n",
          canvas_width, canvas_height, bits_per_pixel);    
    alloctile(canvas_width, canvas_height, bits_per_pixel, blocklen);
}

MosaicMan::MosaicMan()
{
    tile = NULL;    
    ytile_cnt = xtile_cnt = 0;
    tile_len = 0;
    canvas_width = canvas_height = 0;    
    bits_per_pixel = 0;    
    resource_allocated = false;    
}

MosaicMan::~MosaicMan() { release(); }

int MosaicMan::export_mosaic(void)    
{    
    pImgTile tgt_tile;    
    int	i = 0;    
  
    for (int y = 0 ; y < ytile_cnt ; ++y) {
        for (int x = 0 ; x < xtile_cnt ; ++x) {            
            tgt_tile = tile[y][x];            
            tgt_tile->expose();            
            ++i;            
        }        
    }
  
    return i;    
}

int MosaicMan::update_mosaic(int x, int y, int width, int height,
							 unsigned char **src, int src_x, int src_y)
{    
  
    int tgt_s_x = x / this->tile_len;
    int tgt_s_y = y / this->tile_len;
  
    int tgt_e_x = ((x - 1) + width) / this->tile_len;
    int tgt_e_y = ((y - 1) + height) / this->tile_len;

    int	draw_x, draw_y;
    int	d_w, d_h;

    if (((x + width) > this->canvas_width) ||
        ((y + height) > this->canvas_height)) {
        return 0;
    }
  
    for (int i = tgt_s_y ; i <= tgt_e_y ; ++i) {
        for (int j = tgt_s_x ; j <= tgt_e_x ; ++j) {

            int		offset_x, offset_y;
            pImgTile	tgt_tile;

            tgt_tile = this->tile[i][j];

            offset_x = (x - tgt_tile->xpos) <= 0 ? 0 : x - tgt_tile->xpos;
            offset_y = (y - tgt_tile->ypos) <= 0 ? 0 : y - tgt_tile->ypos;

            draw_x = (offset_x > 0) ? x : tgt_tile->xpos;
            draw_y = (offset_y > 0) ? y : tgt_tile->ypos;

            if ((tgt_tile->xpos + tgt_tile->width) > (x + width)) {
                d_w = (x + width) - draw_x;
            } else {
                d_w = (tgt_tile->xpos + tgt_tile->width) - draw_x;
            }

            if ((tgt_tile->ypos + tgt_tile->height) > (y + height)) {
                d_h = (y + height) - draw_y;
            } else {
                d_h = (tgt_tile->ypos + tgt_tile->height) - draw_y;
            }

            // 	  std::cout << "(" << tgt_tile->xpos << "," << tgt_tile->ypos << ")\n";
            // 	  std::cout << " offset_x: " << offset_x << " offset_y: " << offset_y << "\n"
            // 				<< " d_w: " << d_h << " d_h: " << d_w << "\n"
            // 				<< " draw_x: " << draw_x << " draw_y: " << draw_y << "\n"
            // 				<< " src_x: " << draw_x - x << " src_y: " << draw_y - y << "\n\n";
		
            tgt_tile->update_tile(offset_x, offset_y,
                                  d_w, d_h,
                                  src,
                                  src_x + (draw_x - x),
                                  src_y + (draw_y - y));
	  
            // 	  snprintf(this->sexpr, SEXPR_SIZE, "(rectdata \"%s\" %d %d %d %d )",
            // 			   tgt_tile->get_filename(),
            // 			   tgt_tile->xpos, tgt_tile->ypos,
            // 			   tgt_tile->width, tgt_tile->height);

            // 	  this->sque->add_val(this->sexpr);
        }
    }
  
    return 1;
  
}

//override for VNCRGB
int MosaicMan::update_mosaic(int x, int y, int width, int height,
							 VNCRGB *src, int src_x, int src_y)
{
    int tgt_s_x = x / this->tile_len;
    int tgt_s_y = y / this->tile_len;
  
    int tgt_e_x = ((x - 1) + width) / this->tile_len;
    int tgt_e_y = ((y - 1) + height) / this->tile_len;

    int	draw_x, draw_y;  
    int	d_w, d_h;
    
    if (((x + width) > this->canvas_width) ||
        ((y + height) > this->canvas_height)) {
        return 0;
    }
  
    for (int i = tgt_s_y ; i <= tgt_e_y ; ++i) {
        for (int j = tgt_s_x ; j <= tgt_e_x ; ++j) {

            int		offset_x, offset_y;
            pImgTile	tgt_tile;

            tgt_tile = this->tile[i][j];

            offset_x = (x - tgt_tile->xpos) <= 0 ? 0 : x - tgt_tile->xpos;
            offset_y = (y - tgt_tile->ypos) <= 0 ? 0 : y - tgt_tile->ypos;

            draw_x = (offset_x > 0) ? x : tgt_tile->xpos;
            draw_y = (offset_y > 0) ? y : tgt_tile->ypos;
	  
            if ((tgt_tile->xpos + tgt_tile->width) > (x + width)) {
                d_w = (x + width) - draw_x;
            } else {
                d_w = (tgt_tile->xpos + tgt_tile->width) - draw_x;
            }

            if ((tgt_tile->ypos + tgt_tile->height) > (y + height)) {
                d_h = (y + height) - draw_y;
            } else {
                d_h = (tgt_tile->ypos + tgt_tile->height) - draw_y;
            }
	  
            //src_offset = src + ((draw_y - y) * this->canvas_width) + (draw_x - x);
	  
            // 	  std::cout << "(" << tgt_tile->xpos << "," << tgt_tile->ypos << ")\n";
            // 	  std::cout << " offset_x: " << offset_x << " offset_y: " << offset_y << "\n"
            // 				<< " d_w: " << d_h << " d_h: " << d_w << "\n"
            // 				<< " draw_x: " << draw_x << " draw_y: " << draw_y << "\n"
            // 				<< " src_x: " << draw_x - x << " src_y: " << draw_y - y << "\n\n";
	  
            tgt_tile->update_tile(offset_x, offset_y,
                                  d_w, d_h,
                                  src,
                                  src_x + (draw_x - x),
                                  src_y + (draw_y - y),
                                  this->canvas_width,
                                  this->canvas_height);
	  
	  
            // 	  snprintf(this->sexpr, SEXPR_SIZE, "(rectdata \"%s\" %d %d %d %d )",
            // 			   tgt_tile->get_filename(),
            // 			   tgt_tile->xpos, tgt_tile->ypos,
            // 			   tgt_tile->width, tgt_tile->height);
            // 	  this->sque->add_val(this->sexpr);
        }
    }
  
    return 1;
  
}

/*
void MosaicMan::assign_que(SExpr *queexpr)
{    
    this->sentl->assign_que(queexpr);
}
*/

void MosaicMan::release()
{
    for (int y = 0 ; y < this->ytile_cnt ; ++y) {
        for (int x = 0 ; x < this->xtile_cnt ; ++x) {
            delete this->tile[y][x];
        }
    }
    if (this->tile != NULL) {
        for (int i = 0 ; i < this->ytile_cnt ; ++i) {
            delete[] this->tile[i];            
        }
        delete[] this->tile;
    }
    resource_allocated = false;    
}

void MosaicMan::alloctile(int r_w, int r_h, int bits_per_pixel, int blocklen)    
{
    if (resource_allocated)
        release();    
    
    if (blocklen <= 0)        
        this->tile_len = boost::math::gcd(r_w, r_h);    
    else        
        this->tile_len = blocklen;    
  
    this->xtile_cnt = r_w / this->tile_len;
    this->ytile_cnt = r_h / this->tile_len;

    this->canvas_width = r_w;
    this->canvas_height = r_h;

    std::cout << "Tile len: " << this->tile_len << ", xtile_cnt: "
              << this->xtile_cnt << ", ytile_cnt: " << this->ytile_cnt
              << "\n";
  
    this->tile = NULL;
    this->tile = new pImgTile*[this->ytile_cnt];
  
    for (int y = 0 ; y < this->ytile_cnt ; ++y) {
        this->tile[y] = new pImgTile[this->xtile_cnt];
    }

    for (int y = 0 ; y < this->ytile_cnt ; ++y ) {
        for (int x = 0 ; x < this->xtile_cnt ; ++x) {
            this->tile[y][x] = new ImgTile(x * this->tile_len,
                                           y * this->tile_len,
                                           this->tile_len,
                                           this->tile_len,
                                           bits_per_pixel);
        }
    }
    
    resource_allocated = true;    
}

int MosaicMan::change_tilelen(int len)    
{    
    if ((this->canvas_width % len == 0) &&        
        (this->canvas_height % len == 0)) {        
        this->release();
        this->alloctile(this->canvas_width,
                        this->canvas_height,
                        this->bits_per_pixel,
                        len);        
        notice(" changing tile length");        
        return 1;
    }    

    return 0;
}

