
#ifndef	_IMGTILE_H
#define	_IMGTILE_H

#define MAX_NAME		256

#include		<stdio.h>

#include		"SExpr.h"
#include		"Sentinel.h"
#include        "vncclient.hpp"

class ImgTile {
private:
  char	filename[MAX_NAME];
  FILE	*fp;
  
  char  shadow_fname[MAX_NAME];
  
  unsigned char **imgbuf;
  int	bits_per_pixel;
  
  void write_png(FILE *out);
  void gen_filename(char *out);
  void gen_id();
  void openfile(char *fname, FILE **out);

  void swap();
  
public:
  int	dirty;
  int	xpos, ypos;
  int	width, height;
  int	id;
  
  int disburse();
  
  void update_tile(int offset_x, int offset_y, int width, int height,
				   unsigned char **src, int src_x, int src_y);
  
//   void update_tile(int offset_x, int offset_y, int width, int height,
// 				   VNCRGB *src, int src_x, int src_y);
  
  void update_tile(int offset_x, int offset_y,
				   int width, int height,
				   VNCRGB *src, int src_x, int src_y,
				   int root_w, int root_h);

  
  char *get_filename(void);
  
  ImgTile(int x, int y, int width, int height, int bits_per_pixel);
  ~ImgTile();
};

typedef ImgTile* pImgTile;
class Sentinel;

class MosaicMan {

private:
  pImgTile **tile;
  int	xtile_cnt, ytile_cnt;
  int	tile_len;
  int	canvas_width, canvas_height;
  int   bits_per_pixel;
  
  Sentinel *sentl;

  void release();
  void alloctile(int r_w, int r_h, int bits_per_pixel, int blocklen = 0);
  
public:
  int gcd(int m, int n);
  int export_mosaic(void);

  int update_mosaic(int x, int y, int width, int height,
					unsigned char **src, int src_x, int src_y);
  
  //override for VNCRGB
  int update_mosaic(int x, int y, int width, int height,
					VNCRGB *src, int src_x, int src_y);
  
  int change_tilelen(int len);  
  void assign_que(SExpr *queexpr);
  
  MosaicMan(int canvas_width, int canvas_height, int bits_per_pixel, int blocklen = 0);
  ~MosaicMan();
};

#endif

