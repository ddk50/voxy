
CPP = g++
CPPFLAGS = -Wall -O0
LDFLAGS = -lboost_thread -lpthread
OBJS = mockserver.o

mockserver: $(OBJS)
	$(CPP) $(LDFLAGS) -o mockserver $(OBJS)

.cpp.o:
	$(CPP) -c $<

clean:
	rm -f *.o
