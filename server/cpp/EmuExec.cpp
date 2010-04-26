
#include		<iostream>
#include		<sstream>
#include		<fstream>

#include		<stdlib.h>

#include		"common.h"
#include		"EmuExec.h"

EmuExec::EmuExec(int display_num){
  //this->display_num = display_num;
  this->display_num = 2;
}

EmuExec::~EmuExec(void){
  
}

int EmuExec::qemu_server(void){

  char *cdrom_path = QEMU_CDROM_PATH;
  int install = 0;

  if (strlen(cdrom_path) != 0) {
	install = 1;
	commonh::Dbug_sprintf(" Execting qemu (boot mode):\n"
						  "  QEMU_NAME_STR: %s\n"
						  "  QEMU_IMG_PATH: %s\n"
						  "  QEMU_CDROM_PATH: %s\n"
						  "  QEMU_HOST_NAME: %s\n"
						  "  DISPLAY_NUM: %d\n",
						  QEMU_NAME_STR,
						  QEMU_IMG_PATH,
						  QEMU_CDROM_PATH,
						  QEMU_HOST_NAME,
						  this->display_num);
	
  } else {
	install = 0;	
	commonh::Dbug_sprintf(" Execting qemu:\n"
						  "  QEMU_NAME_STR: %s\n"
						  "  QEMU_IMG_PATH: %s\n"
						  "  QEMU_HOST_NAME: %s\n"
						  "  DISPLAY_NUM: %d\n",
						  QEMU_NAME_STR,
						  QEMU_IMG_PATH,
						  QEMU_HOST_NAME,
						  this->display_num);
  }

  install = 0;
  
  return start_vm(install,
				  QEMU_NAME_STR, QEMU_IMG_PATH, QEMU_CDROM_PATH,
				  QEMU_HOST_NAME, this->display_num);
}

int EmuExec::start_vm(int install,
					  char *emu_name, char *img_path, char *cdrom_path,
					  char *host_name, int display_num){
  
  char	arg_str[512];
  int	ret;

  std::cout << "starting qemu" << "\n";

  //   snprintf(arg_str, sizeof(arg_str),
  // 		   "%s -no-kqemu -cdrom /dev/cdrom -boot c -hda %s "
  // 		   "-m 256m -localtime -usb -vnc %s:%d",
  // 		   emu_name, img_path, host_name, display_num);

  if (!install) {
	snprintf(arg_str, sizeof(arg_str),
			 "%s -no-kqemu -boot c -hda %s "
			 "-m 512m -localtime -usb -vnc %s:%d -net nic,vlan=0 "
			 "-net tap,vlan=0,ifname=tap0,script=/etc/qemu-ifup",
			 emu_name, img_path, host_name, display_num);
  } else {
	snprintf(arg_str, sizeof(arg_str),
			 "%s -no-kqemu -boot d -hda %s -cdrom %s "
			 "-m 512m -localtime -usb -vnc %s:%d -net nic,vlan=0 "
			 "-net tap,vlan=0,ifname=tap0,script=/etc/qemu-ifup",
			 emu_name, img_path, cdrom_path ,host_name, display_num);
  }
  
  ret = system(arg_str);
  std::cout << "str " << arg_str << "\n";
  
  return ret;
}

int EmuExec::run(){
  return (this->qemu_server());
}

int EmuExec::getdisplaynum(void){
  return this->display_num;
}
