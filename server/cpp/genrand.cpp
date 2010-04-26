
#include		"genrand.h"

GenRand* GenRand::instance_ = NULL;

GenRand* GenRand::instance() {
  if (instance_ == NULL) {
	instance_ = new GenRand();
  }
  return instance_;
}

int GenRand::gennum(void) {
  return rand_r(&this->seed);
}

GenRand::GenRand() {
  this->seed = (unsigned)time(NULL);
}
