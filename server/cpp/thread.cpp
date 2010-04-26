
// #include <exception>
// #include <cstdlib>
// #include <cxxabi.h>
// #include <bits/c++config.h>
// #include <typeinfo>
// #include <cstddef>
// #include <unwind.h>
//__cxxabiv1

#include "thread.h"

using namespace __cxxabiv1;
using namespace mythread;

thread::thread(){
  this->m_threadid = 0;
  this->terminated = 1;
}

thread::~thread(){
  if ( !this->terminated ) {
	this->terminated = 1;
	pthread_cancel(this->m_threadid);
  }
}

void* thread::start_routine(void* arg){

  thread* p = (thread*)arg;
    
  if(arg == 0){
    return 0;
  }
  
  p->terminated = 0;
  p->run();
  
  return arg;
}

int thread::start(){
  return::pthread_create(&this->m_threadid, 0, thread::start_routine, this);
}

int thread::join(){
  return::pthread_join(this->m_threadid, 0);
}

int thread::run(){
  return 0;
}

int thread::cancel(){
  
  int ret = 0;
  
  if ( !this->terminated ) {
	this->terminated = 1;
	ret = pthread_cancel(this->m_threadid);
  }

  return ret;
}

void thread::exit(){
  pthread_exit(NULL);
}

//
// fix
// fix for call pthread_cancel in try block;
//

// bool thread::is_pthread_cancel() {
  
//   __cxa_eh_globals *globals = __cxa_get_globals();
//   if (globals) {
// 	__cxa_exception *exception = globals->caughtExceptions;
// 	while (exception) {
// 	  const char CXX_ID[] = {'C', '+', '+', '\0'};
// 	  for (int i = 3; i >= 0; i--) {
// 		char c = (exception->unwindHeader.exception_class & (0xFF << (i\
// 																	  * 8))) >> (i * 8);
// 		if (CXX_ID[3 - i] != c) {
// 		  return true;
// 		}
// 	  }
// 	  exception = exception->nextException;
// 	}
//   }
//   return false;
// }


