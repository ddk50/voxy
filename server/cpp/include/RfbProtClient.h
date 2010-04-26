
#ifndef	_RFBPROTCLIENT_H
#define	_RFBPROTCLIENT_H

#include		<arpa/inet.h>
#include		<netinet/in.h>
#include		<sys/socket.h>
#include		<sys/types.h>
#include		<sys/param.h>
#include		<sys/stat.h>
#include		<unistd.h>

#include		<linux/types.h>

#include        <iostream>

#include        "RFBClient.h"
#include		"SExpr.h"
#include		"ImgTile.h"

#define			SEXPR_SIZE		1000

#define Swap16IfLE(s)											\
    ((__u16) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)))

#define Swap32IfLE(l)											\
    ((__u32) ((((l) & 0xff000000) >> 24) |						\
     (((l) & 0x00ff0000) >> 8)  |								\
	 (((l) & 0x0000ff00) << 8)  |								\
	 (((l) & 0x000000ff) << 24)))

#define RFB_FRAMEBUFFER_UPDATE   0
#define RFB_SETCOLOR_MAP_ENTRIES 1
#define RFB_BELL                 2
#define RFB_SERVER_CUT_TEXT      3

typedef struct tagMouseData {
  __u8			button_mask;
  __u16			x_pos;
  __u16			y_pos;
} __attribute__ ((packed)) MouseData;

typedef struct tagPixel_format {
  __u8			bits_per_pixel;
  __u8			depth;
  __u8			big_endian_flag;
  __u8			true_color_flag;
  __u16			red_max;
  __u16			green_max;
  __u16			blue_max;
  __u8			red_shift;
  __u8			green_shift;
  __u8			blue_shift;
  __u8			padding[3];
} __attribute__ ((packed)) Pixel_format;

typedef struct tagServerInit {
  __u16			framebuffer_width;
  __u16			framebuffer_height;
  Pixel_format	server_pixel_format;
  __u32			name_length;
/*   __u8			*name_string; */
} __attribute__ ((packed)) ServerInit;

typedef struct tagRfbMsg {
  __u8			message_type;
  __u8			padding;
} __attribute__ ((packed)) RfbMsg;

/* client to server */
typedef struct tagRfb_framebuf_update_req {
  __u8			message_type;
  __u8			incremental;
  __u16			x_pos;
  __u16			y_pos;
  __u16			width;
  __u16			height;
} __attribute__ ((packed)) Rfb_framebuf_update_req;

/* server to client */
typedef struct tagRfb_framebuf_update {
  __u8			message_type;
  __u8			padding;
  __u16			num_of_rect;
} __attribute__ ((packed)) Rfb_framebuf_update;

typedef struct tagRfb_rect_data {
  __u16			x_pos;
  __u16			y_pos;
  __u16			width;
  __u16			height;
  __u32			encode_type;
} __attribute__ ((packed)) Rfb_rect_data;

typedef struct tagRfb_key_data {
  __u8			message_type;
  __u8			down_flag;
  __u16			padding;
  __u32			key;
} __attribute__ ((packed)) Rfb_key_data;

typedef struct tagRfb_pnt_evt {
  __u8			message_type;
  MouseData		data;
} __attribute__ ((packed)) Rfb_pnt_evt;

class ServerData {
  
public:
  int			proto_ver;
  int			security_type;
  int			shared_flag;
  __u16			red_max;
  __u16			green_max;
  __u16			blue_max;
  __u8			red_shift;
  __u8			green_shift;
  __u8			blue_shift;
  __u8			bits_per_pixel;
  int			width;
  int			height;
  
  unsigned char	**imgbuf;
  char			*srv_name;

  SExpr	*sque;
  MosaicMan *mosaic;
  
private:
  void free_tmpbuf();

public:
  ServerData();
  ~ServerData();

  void set_wh(int width, int height, __u8 bits_per_pixel);
  void set_srvname_len(int len);
  
};

class RfbProtClient : public RFBClient {
  
private:
  int	qemu_connect_time;
  int	rfb_offset_port;
  int					display_num;
  int					sockfd;
  int					rfb_connected;
  struct sockaddr_in	addr;
  
  ServerData			*srvdata;
  char					*frametmpbuf;
  
  unsigned int	seed;

private:
  int rfb_connect(int retry);
  int connect_seq();

  void handshake_protocol_version();
  void handshake_security_type();

  void client_send_init(__u8 shared_flag);
  void recv_server_init();
  
  void cli_framebuf_update_req(int mode);
  inline uint8_t rescalePixValue(uint32_t pixel, uint8_t shift, uint16_t max);
  inline void raw_encode(char *in, unsigned char **out, Rfb_rect_data *data);
  void recv_vgaframebuf(int len, char *tmp_buf, Rfb_rect_data *data);
  void recv_framebuf_update();
  void write_png(char *file_name, unsigned char **image, int width, int height);
  
  char *gen_filename();
  
protected:
  int run();
  
public:
  RfbProtClient(const char *ip, int timeout_value);
  ~RfbProtClient(void);
  
  void mouse_event(int x, int y, int button_mask);
  void keyboard_event(int flag, int key);
  void get_rfb_que(int *cnt, std::string &out);
  void set_port(int port);
  int vnc_connect(void);
  int getsrvinfo(char *ret);
  int chgblocklen(int len);
  
public:
  std::string ip;
};

#endif
