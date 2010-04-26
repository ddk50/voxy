
#include		<stdio.h>
#include		<stdarg.h>
#include		<stdlib.h>
#include		<string.h>
#include		<malloc.h>

#include		<linux/types.h>
#include		<unistd.h>

#include		<pthread.h>
#include		<semaphore.h>

#include		"common.h"

static pthread_mutex_t	send_mut = PTHREAD_MUTEX_INITIALIZER;

int commonh::writen(int sockfd,
					char *ptr,
					int nbytes){

  int	nleft = nbytes;
  int	n;

  while(nleft > 0){
	n = write(sockfd, ptr, nleft);

	if(n <= 0){
	  /* send error */
	  return n;
	}
	
	ptr += n;
	nleft -= n;
  }

  return (nbytes - nleft);
  
}

int commonh::readn(int sockfd,
				   char *ptr,
				   int nbytes){

  int nread;
  int nleft = nbytes;

  while(nleft > 0){
	nread = read(sockfd, ptr, nleft);

	if(nread < 0){
	  return nread;
	}else if(nread == 0){
	  break;
	}

	nleft -= nread;
	ptr += nread;
  }
  
  return (nbytes - nleft);
  
}

int commonh::writen_r(int sockfd,
					  pthread_mutex_t *mutex,
					  char *ptr,
					  int nbytes){

  int	nleft = nbytes;
  int	n;

  pthread_mutex_lock(mutex);

  while(nleft > 0){
	n = write(sockfd, ptr, nleft);

	if(n <= 0){
	  /* send error */
	  return n;
	}
	
	ptr += n;
	nleft -= n;
  }

  pthread_mutex_unlock(mutex);

  return (nbytes - nleft);
  
}

int commonh::readn_r(int sockfd,
					 pthread_mutex_t *mutex,
					 char *ptr,
					 int nbytes){

  int nread;
  int nleft = nbytes;

  pthread_mutex_lock(mutex);

  while(nleft > 0){
	nread = read(sockfd, ptr, nleft);

	if(nread < 0){
	  return nread;
	}else if(nread == 0){
	  break;
	}

	nleft -= nread;
	ptr += nread;
  }

  pthread_mutex_unlock(mutex);
  
  return (nbytes - nleft);
}

int commonh::send_cmd(int fd, int opcode,
					  unsigned char *buf, int len){

  pkt_header	header;
  unsigned char *ptr;
  int			send_size = len + sizeof(pkt_header);

  header.op_code		= opcode;
  header.payload_len	= len;

  //pthread_mutex_lock(&send_mut); {
  ptr = (unsigned char*)malloc(send_size);
  //} pthread_mutex_unlock(&send_mut);
  
  if(!ptr){
	Dbug_sprintf(" %s malloc error\n", __FUNCTION__);
	return 0;
  }
  
  memset(ptr, 0x0, send_size);
  memcpy(ptr, &header, sizeof(pkt_header));
  memcpy(ptr + sizeof(pkt_header), buf, len);

  if(writen(fd, (char*)ptr, send_size) != send_size){
	return 0;
  }

  free(ptr);
  
  return 1;
  
}

int commonh::Dbug_sprintf(const char *fmt, ...){

#ifdef		__DEBUG
  
  va_list		argptr;
  int			ret;
  
  va_start(argptr, fmt);
  ret = vprintf(fmt, argptr);
  va_end(argptr);

  return ret;
  
#else
  return 1;
#endif
  
}

int commonh::Com_sprintf(int force, const char *fmt, ...){
  
  va_list		argptr;
  int			ret;
  
  if(!force){
	return -1;
  }
  
  va_start(argptr, fmt);
  ret = vprintf(fmt, argptr);
  va_end(argptr);
  
  return ret;
}
