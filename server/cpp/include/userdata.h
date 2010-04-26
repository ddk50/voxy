
#ifndef	_USERDATA_H
#define	_USERDATA_H

#include		<iostream>
#include		<vector>

#include		"EmuExec.h"
#include        "RFBClient.h"
#include        "SExpr.h"

#define EMUCNT_PER_USR	4

class EmuSet {
public:
  EmuExec		*emu;
  RFBClient     *rfbcli;
  SExpr			*sque;
  
public:
  EmuSet(char *host, int timeout_value, int display_num);
  ~EmuSet();
  
  int get_dispnum(void);
  
private:
  int	display_num;
};

class UserData {

public:
  int					display_num;
  int					user_id;
  std::vector<EmuSet*>	emu_lst;
  
private:
  EmuSet *find_emu(int emu_num);
  
public:
  UserData(void);
  ~UserData(void);

  int new_emu(void);
  void del_emu(int emu_enum);
  
  int getuserid(void);
  
  int execemu(int emu_num);
  int gettask(int emu_num, std::string &out);
  void sendmouse(int emu_num, int x, int y, int button_mask);
  void sendkey(int emu_num, int flag, int key);
  void command(int emu_num, char *out, char **param);
};

#endif
