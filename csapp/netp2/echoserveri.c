/*
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"
#include <sys/epoll.h>

#define MAX_EVENTS 100000

void echo(int connfd);

int main(int argc, char **argv) 
{
    int listenfd, connfd, port, clientlen, epfd, nfds, i, nr;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;
    struct epoll_event events[MAX_EVENTS];
    struct epoll_event ev, *evp;

    if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
    }
    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);

	epfd = epoll_create(MAX_EVENTS);
	if (epfd == -1) {
		perror("epoll create");
		exit(0);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listenfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
		perror("epoll_ctl: listenfd");
		exit(0);
	}

	nr = 0;

    while (1) {
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(0);
		}

		for (i = 0; i < nfds; i++) {
			evp = &events[i];

			if (evp->events & EPOLLIN)
			{
				if (evp->data.fd == listenfd) {
					clientlen = sizeof(clientaddr);
					connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

					/* determine the domain name and IP address of the client */
					hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
					haddrp = inet_ntoa(clientaddr.sin_addr);
					printf("server connected to %s (%s)\n", hp->h_name, haddrp);

					Set_nonblocking(connfd);

					ev.events = EPOLLIN;
					ev.data.fd = connfd;
					if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
						perror("epoll_ctl");
						Close(connfd);
					}
					
					nr++;
					printf("number connection: %d\n", nr);
				}
				else
				{
					//echo(evp->data.fd);
					ssize_t n, bs;
					int c = 0;
					char buf[MAXLINE];

					while ((n = read(evp->data.fd, buf, MAXLINE)) > 0) {
						bs = write(evp->data.fd, buf, n);
					}

					if (!n) {
						ev.data.fd = evp->data.fd;
						epoll_ctl(epfd, EPOLL_CTL_DEL, evp->data.fd, &ev);

						printf("disconnected\n");
						Close(evp->data.fd);

						nr--;
						printf("number connection: %d\n", nr);
					}
				}
			}
		}
    }

    exit(0);
}
/* $end echoserverimain */
