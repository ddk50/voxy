bin_PROGRAMS = voxyproxy

voxyproxy_CPPFLAGS = -I$(srcdir)/include @BOOST_CPPFLAGS@
voxyproxy_LDFLAGS = @BOOST_LDFLAGS@ -g
voxyproxy_LDADD = @BOOST_SYSTEM_LIB@ @BOOST_THREAD_LIB@ -lpthread -lpng -ljpeg -lX11
voxyproxy_SOURCES = \
	include/common.h \
	include/d3des.h \
	include/global.hpp \
	include/imgtile.h \
	include/rfbproto.hpp \
	include/sexpr.h \
	include/socket.hpp \
	include/vncauth.h \
	include/vncclient.hpp \
	include/vncsockets.hpp \
	d3des.cpp \
	global.cpp \
	mockserver.cpp \
	rfbproto.cpp \
	sexpr.cpp \
	socket.cpp \
	vncauth.cpp \
	vncclient.cpp \
	vncsockets.cpp
