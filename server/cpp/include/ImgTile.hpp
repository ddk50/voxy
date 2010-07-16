
#ifndef	_IMGTILE_H
#define	_IMGTILE_H

#define MAX_NAME		256

#include <stdio.h>
#include <boost/smart_ptr.hpp>

#include "vncclient.hpp"

class ImgTile
{    
private:
    char filename[MAX_NAME];
    FILE *fp;    
  
    char shadow_fname[MAX_NAME];
    char shadow_fnameonly[MAX_NAME];    
  
    unsigned char **imgbuf;
    int	bits_per_pixel;
  
    void write_png(FILE *out);
    void gen_filename(char *out);
    void gen_id(void);    
    void openfile(char *fname, FILE **out);

    void swap(void);

    void init_shadowfname(void);    
  
public:
    int	dirty;
    int	xpos, ypos;
    int	width, height;
    int	id;

    int expose(void);    
  
    void update_tile(int offset_x, int offset_y, int width, int height,
                     unsigned char **src, int src_x, int src_y);
  
    //   void update_tile(int offset_x, int offset_y, int width, int height,
    // 				   VNCRGB *src, int src_x, int src_y);
  
    void update_tile(int offset_x, int offset_y,
                     int width, int height,
                     VNCRGB *src, int src_x, int src_y,
                     int root_w, int root_h);

  
    char *get_filename(void); /* it feeds back full path */
    char *get_onlyfname(void);    
  
    ImgTile(int x, int y, int width, int height, int bits_per_pixel);
    ~ImgTile();
};

typedef ImgTile *pImgTile;
typedef boost::shared_ptr<ImgTile> ImgTile_ptr;

class MosaicMan
{
protected:    
    pImgTile **tile;
    int	xtile_cnt, ytile_cnt;
    int	tile_len;
    int	canvas_width, canvas_height;
    int bits_per_pixel;

    bool resource_allocated;    

    void release();
    void alloctile(int r_w, int r_h,
                   int bits_per_pixel,
                   int blocklen = 0);    
  
public:

    virtual int export_mosaic(void);    
    
    int update_mosaic(int x, int y, int width, int height,
                      unsigned char **src, int src_x, int src_y);    
  
    //override for VNCRGB
    int update_mosaic(int x, int y, int width, int height,
                      VNCRGB *src, int src_x, int src_y);
  
    int change_tilelen(int len);  
    //    void assign_que(SExpr *queexpr);    
    MosaicMan(int canvas_width, int canvas_height,
              int bits_per_pixel, int blocklen = 0);    
    MosaicMan();
    
    virtual ~MosaicMan();    
};

typedef boost::shared_ptr<MosaicMan> MosaicMan_ptr;

#endif

