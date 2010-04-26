
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>

#include "global.hpp"

int debug = 0;

void trace(int dbgmask, const char *s,...) {
  
  va_list ap;

  if (debug & (1 << dbgmask) || dbgmask == DBG_FORCE) {
    va_start(ap, s);
    vfprintf(stderr, s, ap);
    va_end(ap);
    fprintf(stderr, "\n");
  }
}

void error(const char *s,...) {
  
  va_list ap;

  va_start(ap, s);
  vfprintf(stderr, s, ap);
  va_end(ap);
  fprintf(stderr, "\n");
}

void notice(const char *s,...) {
  
  va_list ap;
  char buf[BUFSIZ];

  va_start(ap, s);
  vsprintf(buf, s, ap);
  va_end(ap);
  //GUI::writeMessage("notice", NULL, buf);
  printf(" [notice]: %s\n", buf);
  
}
