
#ifndef	_UDPSERVERSOCK_H
#define	_UDPSERVERSOCK_H

#include		<sys/socket.h>
#include		<netinet/in.h>
#include		<arpa/inet.h>
#include		<iostream>

#include		"thread.h"
#include		"userdata.h"

using namespace mythread;

class UdpServerSock : public thread {
  
public:
  int	sockfd;
  struct sockaddr_in			servaddr;
  struct sockaddr_in			cliaddr;
  socklen_t						len;
  unsigned short				udp_port;
  
public:
  UdpServerSock(unsigned short port);
  virtual ~UdpServerSock(void);
  
  void init_server(void);
  
};

#endif
