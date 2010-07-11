
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>

#include "global.hpp"

using namespace boost;
using boost::posix_time::ptime;
using boost::posix_time::us_dst;
using boost::posix_time::second_clock;

int debug = 0;

void trace(int dbgmask, const char *s,...)
{  
    va_list ap;
    if (debug & (1 << dbgmask) || dbgmask == DBG_FORCE) {
        va_start(ap, s);
        vfprintf(stderr, s, ap);
        va_end(ap);
        fprintf(stderr, "\n");
    }
}

void error(const char *s,...)
{  
    va_list ap;    
    va_start(ap, s);
    vfprintf(stderr, s, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

void error(std::string& val)
{
    std::cerr << val << std::endl;    
}

typedef boost::date_time::c_local_adjustor<ptime> local_adj;
void logging(std::string cmdtype)    
{
    ptime t = second_clock::universal_time();    
    ptime tt = local_adj::utc_to_local(t);    

    std::cout << "[" << tt << "]" << " ACCEPT " << cmdtype        
              << std::endl;    
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
