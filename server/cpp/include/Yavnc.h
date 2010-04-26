
#ifndef __YAVNC_H
#define __YAVNC_H

#include "SExpr.h"
#include "RFBClient.h"
#include "vncclient.hpp"
#include "ImgTile.h"

#define KEYBOARD_BUF_SIZE 100

class Yavnc : public RFBClient {

protected:
  int run();
  
private:
  VNCClient *client;
  MosaicMan *mosaic;
  SExpr *sque;
  char *keyboard_buffer;
  
  int port_num;
  int connect_time;
  int connected;
  int new_tilelen;
  
  int connect_seq();
  
public:
  Yavnc(const char *host, int timeout_value);
  ~Yavnc(void);
  
  void mouse_event(int x, int y, int button_mask);
  void keyboard_event(int flag, int key);
  //void send_rfb_que(int sockfd, void *cliaddr, int addrlen);
  void get_rfb_que(int *cnt, std::string &out);
  void set_port(int port);
  int vnc_connect(void);
  int getsrvinfo(std::string &out);
  int chgblocklen(int len);
  
};

#endif
