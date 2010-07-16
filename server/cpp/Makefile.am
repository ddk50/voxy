bin_PROGRAMS = voxyproxy

voxyproxy_CPPFLAGS = -I$(srcdir)/include @BOOST_CPPFLAGS@
voxyproxy_LDFLAGS = @BOOST_LDFLAGS@ -g
voxyproxy_LDADD = @BOOST_SYSTEM_LIB@ @BOOST_THREAD_LIB@ -lpthread -lpng -ljpeg -lX11
voxyproxy_SOURCES = \
	include/ImgTile.hpp \
	include/common.hpp \
	include/config.h.in \
	include/d3des.h \
	include/genrand.hpp \
	include/global.hpp \
	include/imgtile.h \
	include/parse.h \
	include/rfbproto.hpp \
	include/sexpr.hpp \
	include/socket.hpp \
	include/types.h \
	include/vncauth.h \
	include/vncclient.hpp \
	include/vncsockets.hpp \
	ImgTile.cpp \
	d3des.cpp \
	genrand.cpp \
	global.cpp \
	mockserver.cpp \
	rfbproto.cpp \
	sexpr.cpp \
	socket.cpp \
	vncauth.cpp \
	vncclient.cpp \
	vncsockets.cpp
