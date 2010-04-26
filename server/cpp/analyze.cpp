
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

double getrusage_sec() {
  
  struct rusage t;
  struct timeval tv;
  getrusage(RUSAGE_SELF, &t);
  tv = t.ru_utime;
  return tv.tv_sec + tv.tv_usec * 1e-6;
}
