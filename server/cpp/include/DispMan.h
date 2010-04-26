
#ifndef	_DISP_MAN_H_
#define	_DISP_MAN_H_

#include		<vector>
#include		<iostream>

#define DISP_MAX_SIZE	  10

typedef int atomic_t;

class DispMan {
private:
  DispMan();
  DispMan(const DispMan&);
  DispMan& operator = (const DispMan&);
  static DispMan *instance_;
  
  std::vector<int>   vect;
  
  inline void atomic_inc(volatile atomic_t *v);

public:
  static DispMan *instance();
  int get_disp(void);
  void free_disp(int disp_num);
};

#endif
