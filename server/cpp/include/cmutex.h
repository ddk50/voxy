
#ifndef	_CMUTEX_H
#define	_CMUTEX_H

#include		<pthread.h>

template<typename T>
class scoped_lock_ {
  T& m_;

private:
  const scoped_lock_& operator=( const scoped_lock_& );
  scoped_lock_( scoped_lock_& );
  
public:
  explicit scoped_lock_(T& m) : m_(m) {
	m_.lock();
  }
  ~scoped_lock_() throw() {
	m_.unlock();
  }
};

#define	   STATIC_MUTEX_INIT { PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP }

struct static_mutex {
  pthread_mutex_t		m_;
  void lock() { pthread_mutex_lock(&m_); }
  void unlock() { pthread_mutex_unlock(&m_); }
  typedef scoped_lock_<static_mutex> scoped_lock;
};

#endif
