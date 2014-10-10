/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"

#define MAX_CLIENTS	10000

static int clientfd[MAX_CLIENTS];

int main(int argc, char **argv)
{
    int port, i, n, c;
    char *host, buf[MAXLINE];

    if (argc != 3) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(0);
    }
    host = argv[1];
    port = atoi(argv[2]);

    memset(clientfd, 0, sizeof(clientfd));

	for (i = 0; i < MAX_CLIENTS; i++) {
	    clientfd[i] = Open_clientfd(host, port);

	    if (clientfd[i] < 0)
	    	break;
	}
	
	printf("number of connection: %d\n", i);

	c = 1000;
	while (c-- > 0) {
		for (i = 0; i < MAX_CLIENTS; i++) {	
			if (clientfd[i] > 0) {
				sprintf(buf, "ABCDEFGHIJKLMNOPQRSTUVWXYZ %d\n", i);
				Rio_write(clientfd[i], buf, strlen(buf));

				memset(buf, 0, sizeof(buf));
				Rio_read(clientfd[i], buf, MAXLINE);
				Fputs(buf, stdout);	
			}
		}
	}

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (clientfd[i] > 0) {
			Close(clientfd[i]);
		}
	}

	exit(0);
}
/* $end echoclientmain */
