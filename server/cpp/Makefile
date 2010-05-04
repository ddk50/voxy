CPP = gcc
CPPFLAGS = -Wall -O0 -g
LDFLAGS = -lboost_thread -lboost_system -lpthread -lpng -ljpeg -g
INCLUDE = -I./include
OBJS = mockserver.o sexpr.o
MSERVER = mserver

mserver: $(OBJS)
	$(CPP) $(LDFLAGS) -o $(MSERVER) $(OBJS)

.cpp.o:
	$(CPP) $(CPPFLAGS) $(INCLUDE) -c $<

clean:
	rm -f *.o
	rm -f $(MSERVER)

sexpr.o: include/sexpr.h
mockserver.o: include/sexpr.h
