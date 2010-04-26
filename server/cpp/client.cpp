
#include		<sys/socket.h>
#include		<netinet/in.h>
#include		<arpa/inet.h>
#include		<stdio.h>
#include		<string.h>

#include		<iostream>
#include		<sstream>
#include		<fstream>
#include		<iterator>
#include		<iomanip>
#include		<list>
#include		<map>
#include		<vector>

#include		"AjaxClient.h"
#include		"EmuExec.h"
#include		"thread.h"
#include		"userdata.h"
#include		"CVncErr.h"
#include		"RfbProtClient.h"
#include		"common.h"
#include		"SExpr.h"
#include		"ImgTile.h"
#include        "vncclient.hpp"
#include        "thread.h"
#include        "Yavnc.h"

#define			UDP_SERVER_PORT			32000
#define			SERV_HOST_ADDR			"127.0.0.1"

using namespace mythread;

class test_thread : public thread {
  
private:
  char **buf;

public:
  test_thread()
  {
	this->buf = new char*[100];
	for (int i = 0 ; i < 100 ; ++i) {
	  this->buf[i] = new char[100];
	}
  }
  
  ~test_thread()
  {
	std::cout << "test_thread delete start...";
	for (int i = 0 ; i < 100 ; ++i) {
	  delete[] this->buf[i];
	}
	delete[] this->buf;
	std::cout << "done\n";
  }

private:
  
  int run()
  {
	int i = 0;
	
	while (1) {
	  std::cout << "child thread: " << i++ << "\n";
	  sleep(3);
	}

	std::cout << " child end" << "\n";
	
	return 1;
  }
  
};

static int test_wget(const char *url);
static int test_list();
static int test_mosaic();
static int test_vnc_cli();
static int start_ajax_client();
static int test_thread_cli();
static int test_yavnc();


static int test_wget(const char *url){
  
  std::ostringstream strm;

  strm << "wget " << url;
  std::cout << "exec cmd " << strm;
  
  return system(string(strm.str()).c_str());
}

static int test_list(){

  SExpr	*expr = new SExpr();
  std::string val;

  expr->add_val("test");
  expr->add_val("uhaaa");
  expr->add_val("wewww");
  expr->add_val("uhoo");
  expr->add_val("aaa");

  cout << "test" << "\n";
  expr->out(val);
  cout << val << "\n";
  
  delete expr;

  return 1;
  
}

static int test_mosaic() {

  MosaicMan *mosaic = NULL;
  unsigned char **img = NULL;
  
  int height = 480;
  int width = 640;
  
  mosaic = new MosaicMan(width, height, 24);
  
  img = new unsigned char*[height];
  for (int y = 0 ; y < height ; ++y) {
 	img[y] = new unsigned char[width * (24 >> 3)];
  }
  
  for (int y = 0, i = 0 ; y < height ; ++y) {
 	for (int x = 0, j = 0 ; x < width ; ++x, j+=3) {
	  /*
	  img[y][j + 0] = ((unsigned)i >> 24) & 0xff;
 	  img[y][j + 1] = ((unsigned)i >> 16) & 0xff;
 	  img[y][j + 2] = (unsigned)i & 0xff;
	  */
	  img[y][j + 0] = (unsigned char)i;
 	  img[y][j + 1] = (unsigned char)i;
 	  img[y][j + 2] = (unsigned char)i;
	  ++i;
 	}
  }
  //mosaic->update_mosaic(0, 0, width, height, img, 0, 0);

  //VNCRGB
  VNCRGB *vnc_src = new VNCRGB[height * width];
  
  for (int y = 0, i = 0; y < height ; ++y) {
 	for (int x = 0 ; x < width ; ++x) {
	  /*
	  img[y][j + 0] = ((unsigned)i >> 24) & 0xff;
 	  img[y][j + 1] = ((unsigned)i >> 16) & 0xff;
 	  img[y][j + 2] = (unsigned)i & 0xff;
	  */
	  vnc_src[(y * width) + x].Red = (unsigned char)i;
 	  vnc_src[(y * width) + x].Green = (unsigned char)i;
 	  vnc_src[(y * width) + x].Blue = (unsigned char)i;
	  ++i;
 	}
  }
  mosaic->update_mosaic(0, 0, width, height, vnc_src, 0, 0);
  
  sleep(10);

  for (int y = 0 ; y < height ; ++y) {
	delete[] img[y];
  }
  delete[] img;
  
  delete[] vnc_src;
  delete mosaic;

  return 1;
}

static int test_vnc_cli() {
  
  VNCClient *vnc_cli = NULL;
  
  try {
	std::cout << "start connecting...";
	vnc_cli = new VNCClient("127.0.0.1", 5900, "passwd");
	
	if ( vnc_cli->VNCInit() ) {	  	
	  vnc_cli->sendFramebufferUpdateRequest(0, 0,
											vnc_cli->fbWidth,
											vnc_cli->fbHeight, 0);
	  
	  while (1) {
		if ( !vnc_cli->handleRFBServerMessage() ) {
		  std::cout << " break\n";
		  break;
		}
	  }
	}
	
  } catch (...) {
	std::cout << " error\n";
  }

  delete vnc_cli;

  return 1;
}

static int test_thread_cli() {

  test_thread *test;
  int i = 0;
  
  test = new test_thread();
  test->start();
  
  while (1) {
	++i;
	std::cout << "main thread: " << i << "\n";
	sleep(3);
	
	if (i == 3) {
	  delete test;
	} else if (i == 5) {
	  break;
	}
  }

  return 1;
}

static int start_ajax_client() {

  AjaxClient *client = NULL;
  
  try {
 	client = new AjaxClient(UDP_SERVER_PORT);
 	client->init_server();
	
 	if ( !client->start() ) {
 	  client->join();
 	}
	
  } catch (std::bad_alloc& e){
 	cout << "alloc error, due to " << e.what() << endl;
  } catch (CVncErr& e) {
 	cout << "VNC err: " << e.what() << endl;
  }

  delete client;

  return 1;
  
}

static int test_yavnc() {

  Yavnc *vnc = new Yavnc("127.0.0.1", 10);
  int i = 0;
  
  std::cout << "vnc connect\n";
  vnc->start();

  while(1) {
	++i;

	std::cout << "main thread: " << i << "\n";
	sleep(1);

	if (i == 15) {
	  std::cout << "main thread end, and delete vnc\n";
	  break;
	}
  }

  printf(" delete\n");
  delete vnc;
  return 1;
}

//#define __TEST

int main(int argc, char *argv[]){

#ifdef __TEST
  //test_wget();
  //test_list();
  //test_mosaic();
  //test_thread_cli();
  
  //test_vnc_cli();
  //test_yavnc();
#else
  start_ajax_client();
#endif
  
  return 1;
}
