

#ifndef __RFBCLIENT_H
#define __RFBCLIENT_H

#include "thread.h"

#include <string>
#include <iostream>

using namespace mythread;

class RFBClient : public thread {
  
public:
  RFBClient();
  virtual ~RFBClient(void);
  
  virtual void mouse_event(int x, int y, int button_mask) = 0;
  virtual void keyboard_event(int flag, int key) = 0;
  //virtual void send_rfb_que(int sockfd, void *cliaddr, int addrlen) = 0;
  virtual void get_rfb_que(int *cnt, std::string &out) = 0;
  virtual void set_port(int port) = 0;
  virtual int vnc_connect(void) = 0;
  virtual int getsrvinfo(std::string &out) = 0;
  virtual int chgblocklen(int len) = 0;
};

#endif

