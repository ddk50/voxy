
#ifndef	_CVNCERR_H
#define	_CVNCERR_H

#include		"CVncErr.h"

CVncErr::CVncErr(std::string err_val){
  this->err_reason = err_val;
}

CVncErr::~CVncErr(){

}

std::string CVncErr::what(){
  return this->err_reason;
}

void CVncErr::set_errval(std::string ret){
  
}

#endif
