
#include		"userdata.h"
#include		"genrand.h"
#include		"DispMan.h"
#include		"CVncErr.h"
#include        "RfbProtClient.h"
#include        "Yavnc.h"

EmuSet::EmuSet(char *host, int timeout_value, int display_num) {

  this->display_num = -1;
  
  this->emu		= new EmuExec(display_num);
  this->rfbcli	= dynamic_cast<RFBClient*>(new Yavnc(host, timeout_value));
  
  this->display_num = display_num;

  std::cout << " New emulator. display_num: "
			<< this->display_num
			<< "\n";
}

EmuSet::~EmuSet() {

  if (this->display_num >= 0) {
	DispMan *disp = DispMan::instance();
	disp->free_disp(this->display_num);
  }
  
  delete this->emu;
  delete this->rfbcli;
}

int EmuSet::get_dispnum(void) {
  return this->display_num;
}

UserData::UserData(void) {

  GenRand *rand = GenRand::instance();
  this->user_id = rand->gennum();

  for (int i = 0 ; i < EMUCNT_PER_USR ; ++i) {
	this->emu_lst.push_back( NULL );
  }
  
}

UserData::~UserData(void) {
  for( int i = 0 ; i < (int)this->emu_lst.size() ; i++ ) {
	delete this->emu_lst.at(i);
  }  
}

EmuSet* UserData::find_emu(int emu_num) {

  EmuSet *p;
  
  if ( emu_num >= (int)this->emu_lst.size() ) {
	throw CVncErr("Invalid emulator number. out of range");
  }

  if ( (p = this->emu_lst.at(emu_num)) == NULL ) {
	throw CVncErr("Invalid emulator number.");
  }
  
  return p;
}

int UserData::new_emu(void) {

  int	i;
  int	new_display_num;
  DispMan *disp = DispMan::instance();
  EmuSet *p;
  
  for (i = 0 ; i < EMUCNT_PER_USR ; ++i) {
	if (!this->emu_lst.at( i )) break;
  }

  if (i >= EMUCNT_PER_USR) {
	throw CVncErr("Can not create emulator anymore.");
  }

  new_display_num = disp->get_disp();
  p = new EmuSet(QEMU_HOST_NAME, 3, new_display_num);
  this->emu_lst.at( i ) = p;
  
  return i;
}

void UserData::del_emu(int emu_num) {
  
  EmuSet *p;
  
  if ( (unsigned)emu_num > this->emu_lst.size() ) {
	throw CVncErr("Invalid emulator number.");
  } else {
	p = this->emu_lst.at(emu_num);
	delete p;

	this->emu_lst.at(emu_num) = NULL;
  }
}

int UserData::getuserid(void) {
  return (this->user_id);
}

int UserData::execemu(int emu_num) {
  
  EmuSet *p = find_emu( emu_num );

  // use emulator start;
  //p->emu->start();  
  return ( p->rfbcli->vnc_connect() );
}

int UserData::gettask(int emu_num, std::string &out) {
  
  EmuSet *p = find_emu( emu_num );
  int cnt;
  
  p->rfbcli->get_rfb_que( &cnt, out );
  
  return cnt;
}

void UserData::sendmouse(int emu_num,
						 int x,
						 int y,
						 int button_mask) {
  
  EmuSet *p = find_emu( emu_num );
  p->rfbcli->mouse_event( x, y, button_mask );
}

void UserData::sendkey(int emu_num,
					   int flag,
					   int key) {
  EmuSet *p = find_emu( emu_num );
  p->rfbcli->keyboard_event( flag, key );
}

void UserData::command(int emu_num, char *out, char **param) {

  char *cmd = strtok_r( NULL, " ", param);
  EmuSet *p = find_emu( emu_num );
  
  if ( strcasecmp(cmd, "getsrvinfo") == 0 ) {
	std::cout << "getsrvinfo\n";
	//p->rfbcli->getsrvinfo(out);
  } else if ( strcasecmp(cmd, "chgblocklen") == 0 ) {
	int len = atoi( strtok_r( NULL, " ", param));
	std::cout << "chgblocklen: " << len << "\n";
	p->rfbcli->chgblocklen(len);
  }
  
}
