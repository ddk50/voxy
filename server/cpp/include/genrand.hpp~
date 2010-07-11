
#include		<stdlib.h>
#include		<stdio.h>
#include		<time.h>

class GenRand {
private:
  GenRand();
  GenRand(const GenRand&);
  GenRand& operator = (const GenRand&);
  static GenRand *instance_;
  unsigned int	seed;

public:
  static GenRand *instance();
  int gennum(void);
};
