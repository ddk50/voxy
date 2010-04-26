
#ifndef	_EMUEXEC_H
#define	_EMUEXEC_H

#include		<sys/socket.h>
#include		<netinet/in.h>
#include		<arpa/inet.h>

#include		"thread.h"

#define	UT_MACHINE

#ifdef	UT_MACHINE
 #define	QEMU_NAME_STR	"/usr/bin/qemu"
 #define	QEMU_IMG_PATH	"/home/ddk/qemuimg/fedora_core.qcow"
 #define    QEMU_CDROM_PATH "/dev/cdrom"
#else
 #define	QEMU_NAME_STR	"/home/ddk/programs/qemu-orig/bin/qemu"
 #define    QEMU_CDROM_PATH "/dev/cdrom"
// #define	QEMU_IMG_PATH	"/home/ddk/programs/qemu-orig/bin/cent_os.qcow"
 #define    QEMU_IMG_PATH   "/home/ddk/programs/qemu-orig/bin/fedora_core.qcow"
#endif

#define	QEMU_HOST_NAME	"127.0.0.1"
//#define QEMU_HOST_NAME "192.168.1.2"

using namespace mythread;
using namespace std;

class EmuExec : public thread {
  
private:
  int	display_num;

private:
  int qemu_server(void);
  int start_vm(int install,
			   char *emu_name, char *img_path, char *cdrom_path,
			   char *host_name, int display_num);
  
protected:
  virtual int run();

public:
  EmuExec(int display_num);
  ~EmuExec(void);

  int getdisplaynum(void);
  
};

#endif
