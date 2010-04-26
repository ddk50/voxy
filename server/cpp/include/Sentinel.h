
#ifndef	_SENTINEL_H
#define	_SENTINEL_H

#include "thread.h"
#include "ImgTile.h"
#include "SExpr.h"

#define	SEXPR_SIZE		1000

class ImgTile;
typedef ImgTile* pImgTile;

class Sentinel : public mythread::thread {
public:
  Sentinel(pImgTile **tile, int xcnt, int ycnt);
  ~Sentinel();

  void assign_que(SExpr *queexpr);

protected:
  int run();
  
private:
  int	do_flag;
  pImgTile **tile;
  int ycnt, xcnt;

  SExpr *sque;
  char	sexpr[SEXPR_SIZE];
};

#endif
