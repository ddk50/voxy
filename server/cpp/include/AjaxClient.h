
#ifndef	__AJAXCLIENT_H
#define	__AJAXCLIENT_H

#include		<iostream>
#include		<iterator>
#include		<map>

#include		"UdpServerSock.h"
#include		"userdata.h"

class AjaxClient : public UdpServerSock {

protected:
  std::map<int, UserData*>		usrlst;

protected:
  virtual int run();

public:
  UserData *findusrdata(int id);
  inline void add_user(int id, UserData *usr);

  AjaxClient(unsigned short port);
  ~AjaxClient(void);
  
};

#endif
