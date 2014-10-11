#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <ares.h>
#include <netdb.h>
#include <arpa/inet.h>

ares_channel ac;

static void sock_state_callback(void *data, int sockfd, int read, int write)
{
}

static int sock_create_callback(int sockfd, int type, void *data)
{
	return 0;
}

static void host_callback(void *data, int status, int timeouts, struct hostent *hostent)
{
	char **alias, **haddr;

	printf("h_name %s\n", hostent->h_name);
	for (alias = hostent->h_aliases; *alias != NULL; ++alias) {
		printf("alias: %s\n", *alias);
	}	
	
	for (haddr = hostent->h_addr_list; *haddr != NULL; ++haddr) {
		char buf[32];

		printf("%s\n", *haddr);
		inet_ntop(AF_INET, *haddr, buf, sizeof(buf));
		printf(" %s\n", buf);
	}
}

static void x2y()
{
	struct in_addr addr = {0};
	char buf[64] = {0};

	addr.s_addr = 0x010000c0;
	printf("%x %s\n", addr.s_addr, inet_ntoa(addr));

	addr.s_addr = 0x0;
	strcpy(buf, (char *)"192.168.1.1");
	inet_aton(buf, &addr);
	printf("%x %s\n", addr.s_addr, buf);

	addr.s_addr = 0x010189c0;
	inet_ntop(AF_INET, &addr, buf, sizeof(buf));
	printf("%x %s\n", addr.s_addr, buf);

	addr.s_addr = 0x0;
	strcpy(buf, "192.168.1.111");
	inet_pton(AF_INET, buf, &addr);
	printf("%x %s\n", addr.s_addr, buf);
}

int main(int argc, char *argv[])
{
	int nfds;
	fd_set read_fds, write_fds;
	struct timeval tv, *tvp;

	x2y();

	ares_library_init(ARES_LIB_INIT_ALL);
	ares_init(&ac);

	ares_set_socket_callback(ac, sock_create_callback, NULL);

	ares_gethostbyname(ac, "www.baidu.com", AF_INET, host_callback, NULL);

	for (;;)
	{
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		nfds = ares_fds(ac, &read_fds, &write_fds);
		if (nfds == 0)
			break;
		tvp = ares_timeout(ac, NULL, &tv);
		select(nfds, &read_fds, &write_fds, NULL, tvp);
		ares_process(ac, &read_fds, &write_fds);
	}

	ares_destroy(ac);
	ares_library_cleanup();

	return 0;
}

