
#include		<stdio.h>
#include		<string.h>

#include		<iostream>
#include		<sstream>
#include		<fstream>
#include		<iomanip>
#include		<map>

#include		"UdpServerSock.h"
#include		"common.h"
#include		"userdata.h"
#include		"CVncErr.h"

UdpServerSock::UdpServerSock(unsigned short port) {
  this->udp_port = port;
  this->sockfd = -1;
}

UdpServerSock::~UdpServerSock(void) {

//   std::map<int, UserData*>::iterator itr = this->usrlst.begin();
//   std::map<int, UserData*>::iterator itrEnd = this->usrlst.end();

//   for (; itr != itrEnd ; itr++ ) {
// 	delete itr->second;
//   }

  if (this->sockfd > 0) {
	close(this->sockfd);
  }
  
}

void UdpServerSock::init_server(void) {

  int	ret, except;

  this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family			= AF_INET;
  servaddr.sin_addr.s_addr		= htonl(INADDR_ANY);
  servaddr.sin_port				= htons(this->udp_port);

  ret = bind(sockfd, (struct sockaddr*)&this->servaddr, sizeof(this->servaddr));
  
  if(ret != 0){
	except = -1;
	throw CVncErr("Can not start server\n");
  }
}
