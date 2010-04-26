
#include "Yavnc.h"
#include "global.hpp"
#include "rfbproto.hpp"
#include "global.hpp"

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <X11/Xlib.h>

char *passwd_file = "/home/ddk/programs/tertes_client_new/passwd";

Yavnc::Yavnc(const char *host, int timeout_value)
  : RFBClient()
{
  
  this->connect_time = timeout_value;
  this->port_num = 5900;
  this->connected = 0;
  this->new_tilelen = 0;
  
  this->keyboard_buffer = NULL;
  this->client = NULL;
  this->mosaic = NULL;
  this->sque = NULL;
  this->sque = new SExpr();

  this->client = new VNCClient(const_cast<char*>(host), this->port_num, passwd_file);
  this->keyboard_buffer = new char[KEYBOARD_BUF_SIZE];
  
}

Yavnc::~Yavnc(void) {
  //this->client->VNCClose();
  //this->cancel();
  delete this->mosaic;
  delete this->sque;
  delete this->client;
  delete[] this->keyboard_buffer;
}

void Yavnc::mouse_event(int x, int y, int button_mask) {
  
  client->rfbproto.sendPointerEvent(x, y, button_mask);
}

void Yavnc::keyboard_event(int flag, int key) {

  KeySym ks = key;
  client->rfbproto.sendKeyEvent(ks, flag);
  
}

// void Yavnc::send_rfb_que(int sockfd, void *cliaddr, int addrlen) {
  
//   std::string val;
//   int	cnt;

//   if (this->sque != NULL) {
	
// 	this->sque->first("rfbque");
// 	cnt = this->sque->out(val, 50);
  
// 	if (cnt > 0) {
// 	  sendto(sockfd, val.c_str(), val.size(), 0, (struct sockaddr*)cliaddr, addrlen);
// 	}
//   }
// }

void Yavnc::get_rfb_que(int *cnt, std::string &out) {
  
  if ( this->sque != NULL ) {
	this->sque->first("rfbque");
	*cnt = this->sque->out(out, 50);
	//if (cnt > 0) {
	//sendto(sockfd, val.c_str(), val.size(), 0, (struct sockaddr*)cliaddr, addrlen);
	//}
  }
}

void Yavnc::set_port(int port) {
  this->port_num = port;
}

int Yavnc::connect_seq() {

  int cnt = this->connect_time;

  if ( this->connected ) return 0;
  
  for(int retry = 0 ; cnt ; --cnt) {

	bool ret;
	
	retry ?
	  trace(DBG_VNC, " retry\n") :
	  trace(DBG_VNC, " [server]: trying to connect...");
	
	ret = client->VNCInit();
	if (ret) {
	  
	  delete mosaic;
	  mosaic = NULL;
	  mosaic = new MosaicMan(client->fbWidth, client->fbHeight, 24, this->new_tilelen);
	  
	  sque->clear();
	  mosaic->assign_que(sque);

	  trace(DBG_VNC, " done\n");

	  this->connected = 1;
	  return 1;
	} else {
	  trace(DBG_VNC, " false\n");
	}

	sleep(1);
	retry = 1;
  }

  return 0;
  
}

int Yavnc::run() {

  int wait_usec = (int)((1.0f / 10.0f) * 1000000.0f);
  
 start:
  //  try {
	
	client->sendFramebufferUpdateRequest(0, 0,
										 client->fbWidth,
										 client->fbHeight, 1);
	  
	while ( !this->terminated ) {
	  
	  if ( client->handleRFBServerMessage() ) {
		switch( client->srvmsg ) {
		case rfbFramebufferUpdate :
		  {

			int x, y;
			int w, h;
			
			x = client->recv_rect.r.x;
			y = client->recv_rect.r.y;
			w = client->recv_rect.r.w - client->recv_rect.r.x;
			h = client->recv_rect.r.h - client->recv_rect.r.y;
			  
			this->mosaic->update_mosaic(x, y, w, h, client->framebuffer, x, y);
			
			trace(DBG_VNC, " mosaic: x = %d, y = %d, w = %d, h = %d\n\n", x, y, w, h);
		  }
		}
	  } else {
		trace(DBG_VNC, " reconnect vnc\n");
		this->connected = 0;
		this->connect_seq();
		goto start;
	  }
	  // 100000 = 100ms
	  usleep( wait_usec );
	}
	
	// } catch (...) {
	// for anti pthread_cancel in try block
	//if ( this->is_pthread_cancel() ) {
	//throw;
	  //} else {
	  //trace(DBG_VNC, "run error\n");
	  //	}
	//}
  
  return 0;
}

int Yavnc::vnc_connect(void) {
  
  if ( this->connect_seq() ) {
	start();
	return 1;
  }
  return 0;
}

int Yavnc::getsrvinfo(std::string &out) {
  
  return 0;
}

int Yavnc::chgblocklen(int len) {

  this->new_tilelen = len;
  
  cancel();
  join();
  
  std::cout << "chgblocklen\n";

  this->connected = 0;
  vnc_connect();
  
  return 1;
}
