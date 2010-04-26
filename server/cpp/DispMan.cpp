
#include "DispMan.h"
#include "cmutex.h"
#include "CVncErr.h"

namespace { static_mutex m = STATIC_MUTEX_INIT; }

DispMan* DispMan::instance_ = NULL;

DispMan::DispMan() {
  for (int i = 0 ; i < DISP_MAX_SIZE ; ++i) {
	this->vect.push_back( 0 );
  }
}

DispMan* DispMan::instance() {
  if (instance_ == NULL) {
	instance_ = new DispMan();
  }
  return instance_;
}

void DispMan::atomic_inc(volatile atomic_t *v) {
  __asm__ volatile ( "lock;" "incl %0"
					 : "=m" (v)
					 : "m" (v) );
}

void DispMan::free_disp(int disp_num) {
  static_mutex::scoped_lock sync(m);

  if ( (unsigned)disp_num >= this->vect.size() ) {
	throw CVncErr("Invalid display number");
  } else {
	this->vect.at(disp_num) = 0;
  }
}

int DispMan::get_disp(void) {
  static_mutex::scoped_lock sync(m);
  int	i;
  
  for (i = 0 ; i < (int)this->vect.size() ; ++i) {
	if ( !(this->vect.at(i)) ) {
	  this->vect.at(i) = 1;
	  break;
	}
  }

  if (i >= (int)this->vect.size()) {
	throw CVncErr("Emulator connection limit error. (Max Connection Error)\n");
  }

  return i;
}
