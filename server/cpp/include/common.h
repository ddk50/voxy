
#ifndef	__COMMON_H
#define	__COMMON_H

#include		<pthread.h>
#include		<semaphore.h>

#define	PKT_OP_HELLO			0x10	/* hello */
#define	PKT_OP_BYE				0xff	/* bye */
#define	PKT_OP_RESSIZE			0x20	/* get resolution size */
#define	PKT_OP_DATA				0x30	/* tmp binary */
#define	PKT_OP_CMD				0x20	/* cmd */

#define	SERV_FIFO				"fifo.serv"
#define	FILE_MODE				666
#define	MAX_LINE				256

#define	PKT_HEADER_SIZE			(sizeof(pkt_header))

typedef struct _pkt_header {
  unsigned char	op_code;
  unsigned long payload_len;
/*   char			payload[0]; */
} __attribute__ ((packed)) pkt_header;

typedef struct _pkt_ressize {
  unsigned long	weight;
  unsigned long height;
} pkt_ressize;

namespace commonh {

  int readn(int fd, char *ptr,
			int nbytes);

  int writen(int sockfd, char *ptr,
			 int nbytes);

  int writen_r(int sockfd,
			   pthread_mutex_t *mutex,
			   char *ptr,
			   int nbytes);

  int readn_r(int sockfd,
			  pthread_mutex_t *mutex,
			  char *ptr,
			  int nbytes);

  int send_cmd(int fd, int header,
			   unsigned char *buf, int len);

  int Com_sprintf(int force, const char *fmt, ...);
  int Dbug_sprintf(const char *fmt, ...);

};

#endif
