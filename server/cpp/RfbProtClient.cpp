
#include		<iostream>
#include		<sys/socket.h>
#include		<netinet/in.h>
#include		<arpa/inet.h>

#include		"CVncErr.h"
#include		"RfbProtClient.h"
#include		"common.h"
#include		"png.h"
#include		"genrand.h"

const char protocol_ver[] = {
  0x52, 0x46, 0x42, 0x20, 0x30, 0x30,
  0x33, 0x2e, 0x30, 0x30, 0x33, 0x0a
};

const char protocol_ver37[] = {
  0x52, 0x46, 0x42, 0x20, 0x30, 0x30,
  0x33, 0x2e, 0x30, 0x30, 0x37, 0x0a
};

ServerData::ServerData() {
  this->imgbuf			= NULL;
  this->srv_name		= NULL;
  this->mosaic			= NULL;
  
  this->sque	= new SExpr();
  
  this->width = this->height = 0;
}

ServerData::~ServerData() {
  this->free_tmpbuf();
  delete this->mosaic;
  delete this->sque;
  delete[] this->srv_name;
}

void ServerData::free_tmpbuf() {
  
  for(int y = 0 ; y < this->height ; ++y) {
	delete[] this->imgbuf[y];
  }
  delete[] this->imgbuf;
}

void ServerData::set_wh(int width, int height, __u8 bits_per_pixel) {

  if( this->mosaic != NULL ) {
	delete this->mosaic;
  }
  
  this->mosaic = new MosaicMan(width, height, bits_per_pixel);
  this->mosaic->assign_que(this->sque);
  
  if(this->imgbuf != NULL){
	this->free_tmpbuf();
  }
  
  this->imgbuf = new unsigned char*[height];
  for(int y = 0 ; y < height ; ++y){
	this->imgbuf[y] = new unsigned char[width * (bits_per_pixel >> 3)];
  }

  this->width			= width;
  this->height			= height;
  this->bits_per_pixel	= bits_per_pixel;
  
}

void ServerData::set_srvname_len(int len)
{
  
  if(len <= 0){
	return;
  }
  
  delete[] this->srv_name;
  
  this->srv_name = new char[len + 1];
  this->srv_name[len] = '\0';
  
}

RfbProtClient::RfbProtClient(const char *ip, int timeout_value = 10)
  : RFBClient()
{
  
  this->qemu_connect_time		= timeout_value;
  this->rfb_offset_port			= 5900;
  this->display_num				= 0;
  
  this->ip = ip;
  srvdata = new ServerData();
  frametmpbuf	= NULL;

  this->seed = (unsigned)time(NULL);
  this->rfb_connected = 0;
}

RfbProtClient::~RfbProtClient(void) {
  close(this->sockfd);
  delete srvdata;
  delete[] frametmpbuf;
}

int RfbProtClient::run(){

  RfbMsg		msg;

  if ( !this->rfb_connected ) return 0;
  
  try {
  
	this->handshake_protocol_version();
	this->handshake_security_type();


	this->client_send_init( 0 );
	this->recv_server_init();

	/* 0: full update */
	this->cli_framebuf_update_req(0);

	while( !this->terminated ){

	  if(commonh::readn(this->sockfd, (char*)&msg,
						sizeof(RfbMsg)) != sizeof(RfbMsg)){
		throw CVncErr("Can't recognize rfb packet");
	  }
	
	  /* rfb client msg loop */
	  switch(msg.message_type){
	  case RFB_FRAMEBUFFER_UPDATE:
		/* server to client FramebufferUpdate */
		//commonh::Dbug_sprintf(" catch FramebufUpdate\n");
	  
		this->recv_framebuf_update();
		break;
	  
	  case RFB_SETCOLOR_MAP_ENTRIES:
		commonh::Dbug_sprintf(" catch SetColourMapEntries\n");
		break;
	  
	  case RFB_BELL:
		/* catch bell request */
		commonh::Dbug_sprintf(" bell");
		break;

	  case RFB_SERVER_CUT_TEXT:
		commonh::Dbug_sprintf(" ServerCutText\n");
		break;
	  
	  default:
		commonh::Dbug_sprintf(" client: %u\n", msg.message_type);
		break;
	  }

	  /* 1: rect update */
	  //this->cli_framebuf_update_req(1);
	
	}

  } catch (...) {

  }
  
  return 1;
  
}

int RfbProtClient::connect_seq() {

  int	cnt = this->qemu_connect_time;

  for(int retry = 0 ; cnt ; --cnt){
	
	int	ret;
	
	retry ?
	  std::cout << " retry\n" :
	  std::cout << " [server]: trying to connect...";

	ret = this->rfb_connect(retry);
	if(ret < 0){
	  std::cout << " failed\n";
	  return 0;
	}else if(ret > 0){
	  std::cout << " done\n";
	  this->rfb_connected = 1;
	  this->start();
	  break;
	}

	sleep(1);
	retry = 1;
  }

  return 1;

}

int RfbProtClient::rfb_connect(int retry) {

  unsigned short	port = (unsigned short)(this->rfb_offset_port + this->display_num);

  if(!retry){
	std::cout << " [server] connecting qemu with RFB..." << this->ip
		 << ":" << port << "\n";
	
	memset(&this->addr, 0, sizeof(this->addr));
	this->addr.sin_family		= AF_INET;
	this->addr.sin_addr.s_addr	= inet_addr(this->ip.c_str());
	this->addr.sin_port			= htons(port);

	if((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
	  std::cout << " [server]: can't create socket for qemu rbf\n";
	  return -1;
	}
  }
  
  if(connect(this->sockfd, (struct sockaddr*)&this->addr,
			 sizeof(this->addr)) < 0){
	return 0;
  }

  return 1;
  
}

void RfbProtClient::handshake_protocol_version() {
  
  char	proto_data[12];
  
  if(commonh::readn(this->sockfd,
					proto_data, 12) != 12){
	goto err;
  }

  if(commonh::writen(this->sockfd,
					 proto_data, 12) != 12){
	goto err;
  }

  proto_data[11] = '\0';
  commonh::Dbug_sprintf("Connected to RFB server, "
						"%s\n", proto_data);
  
  /*  server->proto_ver = atoi(proto_data + 8); */
  return;

 err:
  throw CVncErr("RFB handshake wasn't done successfully by ver");
  
}

void RfbProtClient::handshake_security_type() {
  
  __u32	security_type = 1;
  __u32	security_result = 0;
  
  if(commonh::readn(this->sockfd,
					(char*)&security_type, sizeof(security_type)
					!= sizeof(security_type))){
	goto err;
  }
  /*   security_type = Swap32IfLE(security_type); */
  commonh::Dbug_sprintf("security type is %d\n", security_type);
  
  if(security_type == 0){
	commonh::Dbug_sprintf(" invalid\n");
	goto err;
  }else if(security_type == 1){
	commonh::Dbug_sprintf(" vnc none\n");
  }else if(security_type == 2){
	commonh::Dbug_sprintf(" vnc authentication\n");
  }else{
	commonh::Dbug_sprintf(" security type is invalid\n");
	goto err;
  }
  
  if(commonh::readn(this->sockfd,
					(char*)&security_result, sizeof(__u32)
					!= sizeof(__u32))){
	goto err;
  }

  security_result = Swap32IfLE(security_result);
  commonh::Dbug_sprintf(" security result: %d\n", security_result);
  if(security_result == 1){
	throw CVncErr("Can not support security_result");
  }
  
  this->srvdata->security_type = security_type;

  return;

 err:
  throw CVncErr("RFB handshake wasn't done successfully by sectype");
  
}

void RfbProtClient::client_send_init(__u8 shared_flag) {
  
  __u32			check;

  if(commonh::writen(this->sockfd, (char*)&shared_flag, 1) != 1){
	goto err;
  }

  this->srvdata->shared_flag = shared_flag;
  commonh::Dbug_sprintf(" client init sended\n");

  if(commonh::readn(this->sockfd, (char*)&check, 4) != 4){
	goto err;
  }

  commonh::Dbug_sprintf(" ??? = %d\n", Swap32IfLE(check));

  return;

 err:
  throw CVncErr("RFB clientinit wasn't done successfully");
  
}

void RfbProtClient::recv_server_init() {
  
  ServerInit	srv_init;
  
  memset(&srv_init, 0, sizeof(ServerInit));
  commonh::Dbug_sprintf("ServerInit size: %d [bytes]\n", sizeof(ServerInit));
  
  if(commonh::readn(this->sockfd, (char*)&srv_init, 24) != 24){
	goto err;
  }

  if(srv_init.server_pixel_format.big_endian_flag){
	commonh::Dbug_sprintf(" big_endian_flag\n");
  }

  /* set width and height */
  this->srvdata->set_wh(Swap16IfLE(srv_init.framebuffer_width),
						Swap16IfLE(srv_init.framebuffer_height),
						srv_init.server_pixel_format.bits_per_pixel);
  
  this->srvdata->red_max		= Swap16IfLE(srv_init.server_pixel_format.red_max);
  this->srvdata->green_max		= Swap16IfLE(srv_init.server_pixel_format.green_max);
  this->srvdata->blue_max		= Swap16IfLE(srv_init.server_pixel_format.blue_max);

  this->srvdata->red_shift		= srv_init.server_pixel_format.red_shift;
  this->srvdata->green_shift	= srv_init.server_pixel_format.green_shift;
  this->srvdata->blue_shift		= srv_init.server_pixel_format.blue_shift;

  /* set name */
  srv_init.name_length	= Swap32IfLE(srv_init.name_length);
  this->srvdata->set_srvname_len(srv_init.name_length);
  
  if(srv_init.name_length > 0){
	
	if(commonh::readn(this->sockfd, this->srvdata->srv_name, srv_init.name_length)
	   != (int)srv_init.name_length){
	  goto err;
	}
  }

  commonh::Com_sprintf(1,
					   " framebuffer-width: %u\n"
					   " framebuffer-height: %u\n"
					   /* 		 " server-pixel-format: %d\n" */
					   " red_max: %u\n"
					   " green_max: %u\n"
					   " blue_max: %u\n"
					   " bits_per_pixel: %u\n"
					   " depth: %u\n"
					   " name: %s\n",
					   this->srvdata->width,
					   this->srvdata->height,
					   this->srvdata->red_max,
					   this->srvdata->green_max,
					   this->srvdata->blue_max,
					   this->srvdata->bits_per_pixel,
					   srv_init.server_pixel_format.depth,
					   this->srvdata->srv_name);

  return;
  
 err:
  throw CVncErr("Rfb error, recv_server_init\n");
  
}

void RfbProtClient::cli_framebuf_update_req(int mode) {
  
  Rfb_framebuf_update_req	packet;
  int len = sizeof(Rfb_framebuf_update_req);
  
  memset(&packet, 0, len);
  
  packet.message_type	= 3;
  packet.incremental	= mode;
  packet.x_pos			= Swap16IfLE(0);
  packet.y_pos			= Swap16IfLE(0);
  packet.width			= Swap16IfLE(this->srvdata->width);
  packet.height			= Swap16IfLE(this->srvdata->height);
  
  if(commonh::writen(this->sockfd, (char*)&packet, len) != len){
	commonh::Dbug_sprintf("send error\n");
	goto err;
  }
  
  commonh::Dbug_sprintf(" send framebuf update reg \n");

  return;

 err:
  CVncErr("Err, client framebuf update req\n");
  
}

inline uint8_t RfbProtClient::rescalePixValue(uint32_t pixel,
											  uint8_t shift, uint16_t max) {
  return (uint8_t)(((pixel >> shift) & max));
}

inline void RfbProtClient::raw_encode(char *in,
									  unsigned char **out, Rfb_rect_data *data) {
  int i = 0;
  uint32_t *src = (uint32_t*)in;
  
  for (int y = 0; y < data->height; ++y) {
	for (int x = 0, j = 0; x < data->width; ++x, j+=3, ++i) {
	  out[y][j + 0] = rescalePixValue(src[i], this->srvdata->red_shift, this->srvdata->red_max);
	  out[y][j + 1] = rescalePixValue(src[i], this->srvdata->green_shift, this->srvdata->green_max);
	  out[y][j + 2] = rescalePixValue(src[i], this->srvdata->blue_shift, this->srvdata->blue_max);
	}
  }
  
  return;
}

void RfbProtClient::recv_vgaframebuf(int len, char *tmp_buf, Rfb_rect_data *data) {
  
  if(!tmp_buf){
	goto err;
  }
  
  if(commonh::readn(this->sockfd, (char*)tmp_buf, len) != len){
	goto err;
  }

  switch(data->encode_type){
  case 0:
	this->raw_encode(tmp_buf, this->srvdata->imgbuf, data);
	break;
  case 1:
  case 2:
  case 5:
  case 16:
  default:
	commonh::Com_sprintf(1, "Unsuppoted encode type: %d\n",
						 data->encode_type);
	throw CVncErr("Can not suppoted encode type\n");
  }

//   filename = this->gen_filename();
//   snprintf(this->sexpr, SEXPR_SIZE,
// 		   "(rectdata \"%s\" %d %d %d %d )",
// 		   filename, data->x_pos, data->y_pos,
// 		   data->width, data->height);


  //*********************************************************************
  //
  // この実装ではthis->srvdata->imgbufの(0, 0)をオフセットとした場所
  // データがコピーされる
  //*********************************************************************
  this->srvdata->mosaic->update_mosaic(data->x_pos, data->y_pos,
									   data->width, data->height,
									   this->srvdata->imgbuf,
									   0,
									   0);
  
//   this->write_png(filename, this->srvdata->imgbuf,
// 				  data->width, data->height);
  
  //this->que->add_val(this->sexpr);
  
  return;
  
 err:
  throw CVncErr("recv_vgaframebuf");
  
}

void RfbProtClient::recv_framebuf_update(){
  
  /*   Rfb_framebuf_update header; */
  Rfb_rect_data			rect_data;
  int					len;
  __u16					num_of_rect;
  
  if(commonh::readn(this->sockfd, (char*)&num_of_rect, sizeof(__u16))
	 != sizeof(__u16)){
	commonh::Dbug_sprintf(" can not catch\n");
	goto err;
  }

  num_of_rect = Swap16IfLE(num_of_rect);
  
  this->frametmpbuf = new char[(this->srvdata->bits_per_pixel >> 3) *
							   this->srvdata->height * this->srvdata->width];
  
  for (int i = 0 ; i < num_of_rect ; i++) {
	
	if(commonh::readn(this->sockfd, (char*)&rect_data, sizeof(Rfb_rect_data))
	   != sizeof(Rfb_rect_data)){
	  goto err;
	}
	
	rect_data.x_pos				= Swap16IfLE(rect_data.x_pos);
	rect_data.y_pos				= Swap16IfLE(rect_data.y_pos);
	rect_data.width				= Swap16IfLE(rect_data.width);
	rect_data.height			= Swap16IfLE(rect_data.height);
	rect_data.encode_type		= Swap32IfLE(rect_data.encode_type);

	len = rect_data.height * rect_data.width * (this->srvdata->bits_per_pixel >> 3);
	if (len == 0) {
	  continue;
	}
	
	if ((rect_data.x_pos + rect_data.width > this->srvdata->width) ||
		 (rect_data.y_pos + rect_data.height > this->srvdata->height)){
	  commonh::Dbug_sprintf(" Rect too large: %dx%d (%d, %d)\n",
							rect_data.x_pos, rect_data.y_pos,
							rect_data.width, rect_data.height);
	  throw CVncErr("recv_toobig_rectdata");
	}
	
	this->recv_vgaframebuf(len, this->frametmpbuf, &rect_data);
  }

  delete[] (this->frametmpbuf);
  this->frametmpbuf = NULL;
  
  return;

 err:
  CVncErr("recv_framebuf_update\n");
  
}

void RfbProtClient::write_png(char *file_name,
							  unsigned char **image,
							  int width, int height)
{
  
  FILE            *fp;
  png_structp     png_ptr;
  png_infop       info_ptr;
  
  fp = fopen(file_name, "wb");
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
									NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, fp);
  
//   png_set_IHDR(png_ptr, info_ptr, width, height,
// 			   8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
// 			   PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_set_IHDR(png_ptr, info_ptr, width, height,
 			   8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
 			   PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  
  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, image);
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  
  fclose(fp);
  
  return;
}

char *RfbProtClient::gen_filename() {
  
  static char	filename[MAX_NAME + 1];
  GenRand		*rand = GenRand::instance();
  int			val = rand->gennum();
  
  snprintf(filename, sizeof(filename) - 1, "pngtest/%d.png", val);
  return filename;
}

void RfbProtClient::mouse_event(int x, int y, int button_mask)
{

  if ( this->rfb_connected ) {
  
	Rfb_pnt_evt	point_data;
  
	point_data.data.x_pos			= Swap16IfLE((unsigned short)x);
	point_data.data.y_pos			= Swap16IfLE((unsigned short)y);
	point_data.data.button_mask		= button_mask;
  
	point_data.message_type		= 5;
  
	if(commonh::writen(this->sockfd, (char*)&point_data,
					   sizeof(Rfb_pnt_evt)) != sizeof(Rfb_pnt_evt)){
	  CVncErr("Can not send mouse_event\n");
	}

  }

  return;
}

void RfbProtClient::keyboard_event(int flag, int key)
{
  
  if ( this->rfb_connected ) {
  
	Rfb_key_data	key_data;

	key_data.message_type = 4;
	key_data.down_flag	= flag;
	key_data.padding		= 0;
	key_data.key			= Swap32IfLE((unsigned)key);
  
	if(commonh::writen(this->sockfd, (char*)&key_data,
					   sizeof(Rfb_key_data)) != sizeof(Rfb_key_data)){
	  CVncErr("Can not send keyevent");
	}
	
  }

  return;
}

void RfbProtClient::get_rfb_que(int *cnt, std::string &out)
{
  this->srvdata->sque->first("rfbque");
  *cnt = this->srvdata->sque->out(out, 50);
  //if (cnt > 0) {
  //sendto(sockfd, val.c_str(), val.size(), 0, (struct sockaddr*)cliaddr, addrlen);
  //}
}

void RfbProtClient::set_port(int port) {
  
}

int RfbProtClient::vnc_connect(void) {
  //将来的にはここでループをまわしてconnect_seqを実行すること．
  //RfbProtClient全体は例外を投げるようになってるが，このままの実装だとpthreadのスレッド内部から，外部へと
  //例外を投げていることになっているよ！（これはおかしいだろ）
  
  if( !this->connect_seq() ){
	return 0;
  }

  return 1;
}

int RfbProtClient::getsrvinfo(char *ret) {
  return 0;
}

int RfbProtClient::chgblocklen(int len) {
  return 0;
}
