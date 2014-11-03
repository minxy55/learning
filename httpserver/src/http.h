#ifndef http_h__
#define http_h__

#include <pthread.h>

#define FALSE 0
#define TRUE 1

typedef unsigned char uchar;
typedef unsigned short int usint;
//typedef unsigned int uint;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;


typedef int SOCKET;
typedef int HANDLE;
#define INVALID_HANDLE_VALUE -1
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

typedef struct {
	int port;
	int handle;
	char user[64];
	char pass[64];

	pthread_t http_tid;
	pthread_mutex_t lockhttp;
} Http_t;

typedef struct {
	int restart;
} Program_t;

extern Http_t http;
extern Program_t prg;


#endif 

