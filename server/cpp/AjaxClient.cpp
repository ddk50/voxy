
#include		"AjaxClient.h"
#include		"CVncErr.h"

struct lex_info {
  int	usrid;
  char	*ret_mesg;
  char  *saveptr;
  int	size;
};

struct index_val {
  int	(*func)(char*, AjaxClient*, struct lex_info*);
  char	*cmd;
};

static int trmp_addusr(char*, AjaxClient*, struct lex_info*);
static int trmp_execemu(char*, AjaxClient*, struct lex_info*);
static int trmp_listen(char*, AjaxClient*, struct lex_info*);
static int trmp_mouse(char*, AjaxClient*, struct lex_info*);
static int trmp_key(char*, AjaxClient*, struct lex_info*);
static int trmp_release(char*, AjaxClient*, struct lex_info*);
static int trmp_syscmd(char*, AjaxClient*, struct lex_info*);

struct index_val func_lst[] = {
  { &trmp_addusr,		"getuser" },
  { &trmp_execemu,		"execemu" },
  { &trmp_listen,		"listen" },
  { &trmp_mouse,		"mouse" },
  { &trmp_key,			"key" },
  { &trmp_release,      "release"},
  { &trmp_syscmd,       "syscmd"}
};

static int trmp_addusr(char *tp,
					   AjaxClient *ac,
					   struct lex_info *info) {
  
  UserData		*newusr = new UserData();
  
  info->usrid = newusr->getuserid();
  ac->add_user(info->usrid, newusr);

  std::cout << "new user was issued: " << info->usrid << "\n";
  
  snprintf(info->ret_mesg, info->size,
		   "(setuser %d)", info->usrid);

  return 1;
}

static int trmp_execemu(char *tp,
						AjaxClient *ac,
						struct lex_info *info) {

  UserData *p;
  
  info->usrid = atoi( strtok_r( NULL, " ", &info->saveptr ));
  p = ac->findusrdata( info->usrid );
  
  std::cout << "new emulator num: " << p->new_emu() << "\n";
  std::cout << "execemu: " << info->usrid << "\n";
  
  if ( p->execemu( 0 ) ) {
	snprintf(info->ret_mesg, info->size, "(done)");
  } else {
	throw CVncErr("Could not start vnc session");
  }

  return 1;
}

static int trmp_listen(char *tp,
					   AjaxClient *ac,
					   struct lex_info *info) {

  UserData *p;
  std::string rfbque;
  
  info->usrid = atoi( strtok_r( NULL, " ", &info->saveptr ));
  p = ac->findusrdata(info->usrid);
  
  if ( 0 < p->gettask( 0, rfbque) ) {
	strncpy( info->ret_mesg, rfbque.c_str(), info->size );
  } else {
	snprintf( info->ret_mesg, info->size, "(done)" );
  }
  
  return 1;
}

static int trmp_mouse(char *tp,
					  AjaxClient *ac,
					  struct lex_info *info) {
  UserData *p;
  int xpos, ypos, button_mask;
  std::string rfbque;
  
  info->usrid   = atoi( strtok_r( NULL, " ", &info->saveptr ) );
  p				= ac->findusrdata( info->usrid );
  xpos			= atoi( strtok_r( NULL, " ", &info->saveptr ) );
  ypos			= atoi( strtok_r( NULL, " ", &info->saveptr ) );
  button_mask	= atoi( strtok_r( NULL, " ", &info->saveptr ) );
  
  std::cout << "(" << xpos << ", "
 			<< ypos << "): " << button_mask
 			<< "\n";
  
  p->sendmouse( 0, xpos, ypos, button_mask );
  snprintf( info->ret_mesg, info->size, "(done)" );
  
//   if ( 0 < p->gettask( 0, rfbque ) ) {
// 	strncpy( info->ret_mesg, rfbque.c_str(), info->size );
//   } else {
// 	snprintf( info->ret_mesg, info->size, "(done)" );
//   }
  
  return 1;
}

static int trmp_key(char *tp,
					AjaxClient *ac,
					struct lex_info *info) {
  
  UserData *p;
  int keycode, flag;
  std::string rfbque;
  
  info->usrid	= atoi(strtok_r( NULL, " ", &info->saveptr ));
  p				= ac->findusrdata( info->usrid );
  keycode		= atoi(strtok_r( NULL, " ", &info->saveptr ));
  flag			= atoi(strtok_r( NULL, " ", &info->saveptr ));

  
  std::cout << " keypush: " << keycode
			<< " (" << flag << ")" << "\n";
  
  //  if (keycode == 13 || keycode == 10) {
  //	std::cout << "key send enter" << "\n";
  //	keycode = 0xff0d;
  // } else if (keycode == 8) {
  //	keycode = 0xff08;
  //} else if (keycode == 16) {
  //	keycode = 0xffe1;
  //} else {
  //	std::cout << "keycode: " << keycode << "\n"
  //			  << "key: " << (char)keycode << "\n"
  //			  << "flag: " << flag << "\n";
  //}
  
  p->sendkey( 0, flag, keycode );
  //p->gettask( 0, rfbque );
  //strncpy( info->ret_mesg, rfbque.c_str(), info->size );
  snprintf( info->ret_mesg, info->size, "(done)" );
  
  return 1;
}

static int trmp_release(char *tp,
						AjaxClient *ac,
						struct lex_info *info) {

  UserData *p;

  info->usrid = atoi(strtok_r(NULL, " ", &info->saveptr));
  p = ac->findusrdata( info->usrid );
  
  std::cout << "delete emulator\n";
  delete p;

  snprintf(info->ret_mesg, info->size, "(done)");
  
  return 1;
}

static int trmp_syscmd(char *tp,
					   AjaxClient *ac,
					   struct lex_info *info) {
  
  UserData *p;
  
  info->usrid  = atoi(strtok_r( NULL, " ", &info->saveptr ));
  p = ac->findusrdata( info->usrid );

  std::cout << "syscmd\n";
  
  p->command( 0, info->ret_mesg, &info->saveptr );

  return 1;
}


AjaxClient::AjaxClient(unsigned short port)
  : UdpServerSock(port) {
}

AjaxClient::~AjaxClient(void) {
  
  std::map<int, UserData*>::iterator itr = this->usrlst.begin();
  std::map<int, UserData*>::iterator itrEnd = this->usrlst.end();

  for (; itr != itrEnd ; itr++ ) {
	delete itr->second;
  }
}

UserData* AjaxClient::findusrdata(int id) {
  std::map<int, UserData*>::iterator itr = this->usrlst.find(id);
  if (this->usrlst.end() == itr) {
	throw CVncErr("illegal user id");
  }
  return ((UserData*)itr->second);
}

inline void AjaxClient::add_user(int id, UserData *usr) {
  this->usrlst.insert(std::pair<int, UserData*>(id, usr));
}

int AjaxClient::run() {
  
  int	ret;
  char	mesg[10000 + 1];
  char	ret_mesg[10000 + 1];
  char	*tp;
  
  while( !this->terminated ) {
	
	try {
	  ret = recvfrom(this->sockfd, mesg, 1000, 0,
					 (struct sockaddr*)&this->cliaddr, &len);
	  mesg[ret] = '\0';
	  
	  int		 cnt = sizeof(func_lst) / sizeof(index_val);
	  lex_info	info;
	  int		need = 1;

	  info.usrid		= 0;
	  info.ret_mesg		= ret_mesg;
	  info.size			= sizeof(ret_mesg) - 1;
	  
	  tp = strtok_r(mesg, " ", &info.saveptr);
	  
	  for ( int i = 0 ; i < cnt ; ++i ) {
		if ((strcmp(tp, func_lst[i].cmd)) == 0) {
		  need = func_lst[i].func(tp, this, &info);
		  break;
		}
	  }
	  
	  if (need) {
		if (sendto(this->sockfd, ret_mesg, strlen(ret_mesg), 0,
				   (struct sockaddr*)&this->cliaddr, sizeof(this->cliaddr)) < 0) {
		  std::cout << "sendto error: " << "\n";
		}
	  }

	} catch (CVncErr &err) {

	  std::string err_msg = err.what();
	  
	  std::cout << "server error: " << err_msg << "\n";
	  snprintf(ret_mesg, sizeof(ret_mesg) - 1, "(error \"%s\")", err_msg.c_str());
	  sendto(this->sockfd, ret_mesg, strlen(ret_mesg), 0,
			 (struct sockaddr*)&this->cliaddr, sizeof(this->cliaddr));
	}
  }

  return 1;
}

