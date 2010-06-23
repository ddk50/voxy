CPP = g++
CPPFLAGS = -Wall -O0 -g
LDFLAGS = -lboost_thread -lboost_system -lpthread -lpng -ljpeg -lX11 -g
INCLUDE = -I./include
OBJS = mockserver.o sexpr.o rfbproto.o vncsockets.o global.o vncauth.o d3des.o socket.o vncclient.o
MSERVER = mserver

mserver: $(OBJS)
	$(CPP) $(LDFLAGS) -o $(MSERVER) $(OBJS)

.cpp.o:
	$(CPP) $(CPPFLAGS) $(INCLUDE) -c $<

clean:
	rm -f *.o
	rm -f $(MSERVER)
	rm -f *.png

sexpr.o: include/sexpr.h
mockserver.o: include/sexpr.h
d3des.o: include/d3des.h
global.o: include/global.hpp
vncauth.o: include/global.hpp include/vncauth.h
socket.o: include/global.hpp include/socket.hpp
rfbproto.o: include/global.hpp include/rfbproto.hpp
vncsockets.o: include/global.hpp include/vncsockets.hpp include/socket.hpp
vncclient.o: include/global.hpp include/vncclient.hpp

