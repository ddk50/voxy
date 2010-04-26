
#include		<iostream>

#include		"Sentinel.h"
#include		"png.h"
#include		"genrand.h"
#include        "global.hpp"

Sentinel::Sentinel(pImgTile **tile, int xcnt, int ycnt) {
  this->do_flag = 0;
  this->tile = tile;

  this->ycnt = ycnt;
  this->xcnt = xcnt;

  this->sque = NULL;
}

Sentinel::~Sentinel() {
}

int Sentinel::run() {

  ImgTile *p;  
  GenRand *rand = GenRand::instance();
  int val;
  
  while ( !this->terminated ) {
	for (int y = 0 ; y < this->ycnt ; ++y) {
	  for (int x = 0 ; x < this->xcnt ; ++x) {
		p = this->tile[y][x];

		if ( p->disburse() ){
		  if (this->sque == NULL) {
			std::cout << "drop task" << "\n";
			goto oops;
		  }

		  val = rand->gennum();
		  
		  snprintf(this->sexpr, SEXPR_SIZE, "(rectdata \"%s?%d\" %d %d %d %d %d )",
				   p->get_filename(),
				   val,
				   p->id,
				   p->xpos, p->ypos,
				   p->width, p->height);
		  
		  trace(DBG_VNC, "que: %s", this->sexpr);
		  
		  this->sque->add_val(this->sexpr);
		  
		}
	  }
	}
	// 100000 = 100ms
	//usleep( 1000 );
	sleep( 0 );
  oops:
	sleep( 0 );
  }

  return 1;
}

void Sentinel::assign_que(SExpr *queexpr){
  this->sque = queexpr;
}


