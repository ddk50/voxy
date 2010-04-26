
#include		<pthread.h>

#ifndef	_THREAD_H
#define	_THREAD_H

namespace mythread
{
  class thread {
	
  private:
    static void* start_routine(void *);
	
  public:
    thread();
    virtual ~thread();
    virtual int start();
    virtual int join();
	virtual int cancel();
	virtual void exit();
	
	//bool is_pthread_cancel();
	
  protected:
    virtual int run();
	
  protected:
    pthread_t m_threadid;
	int terminated;
	
  };
};

#endif
