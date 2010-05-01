
CPP = g++
CPPFLAGS = -Wall -O0
LDFLAGS = -lboost_thread -lpthread -lpng -ljpeg
OBJS = mockserver.o
MSERVER = mserver

mserver: $(OBJS)
	$(CPP) $(LDFLAGS) -o $(MSERVER) $(OBJS)

.cpp.o:
	$(CPP) $(CPPFLAGS) -c $<

clean:
	rm -f *.o
	rm -f $(MSERVER)

