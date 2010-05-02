
CPP = gcc
CPPFLAGS = -Wall -O0 -g
LDFLAGS = -lboost_thread -lboost_system -lpthread -lpng -ljpeg -g
OBJS = mockserver.o
MSERVER = mserver

mserver: $(OBJS)
	$(CPP) $(LDFLAGS) -o $(MSERVER) $(OBJS)

.cpp.o:
	$(CPP) $(CPPFLAGS) -c $<

clean:
	rm -f *.o
	rm -f $(MSERVER)

