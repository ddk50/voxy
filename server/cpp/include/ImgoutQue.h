
#ifndef	_IMGOUTQUE_H
#define	_IMGOUTQUE_H

typedef struct tagImginfo {
  unsigned int	x_pos;
  unsigned int	y_pos;
  unsigned int	height;
  unsigned int	width;
} __attribute__ ((packed)) ImgInfo;

class ImgoutQue : public SExpr {

public:
  ImgoutQue();
  ~ImgoutQue();

  void add_val(ImgInfo& info);
};

#endif
