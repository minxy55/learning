
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <poll.h>

#include <signal.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "http.h"
#include "sockets.h"
#include "threads.h"
#include "httpserver.h"

Http_t http =
{
    .port   = 5000,
    .handle = -1,
    .user   = {"\x0"},
    .pass   = {"\x0"},
};

Program_t prg = 
{
	.restart	= 0,
};

int main(int argc, char *argv[])
{
	debug("TCS going\n");
	pthread_mutex_init(&http.lockhttp, NULL);

	debug("Create Server sock tcp\n");
	http.handle = CreateServerSockTcp(http.port);

	debug("start thread http\n");
	start_thread_http();
	while (1) {
		sleep(1);
	}
    close(http.handle);

    return 0;
}
