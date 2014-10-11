#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <netdb.h>
#include <arpa/inet.h>

static void x2y()
{
	struct in_addr addr = {0};
	char buf[64] = {0};

	addr.s_addr = 0x010000c0;
	printf("inet_ntoa-> %08x %s\n", addr.s_addr, inet_ntoa(addr));

	addr.s_addr = 0x0;
	strcpy(buf, (char *)"192.168.1.1");
	inet_aton(buf, &addr);
	printf("inet_aton-> %08x %s\n", addr.s_addr, buf);

	addr.s_addr = 0x010189c0;
	inet_ntop(AF_INET, &addr, buf, sizeof(buf));
	printf("inet_ntop-> %08x %s\n", addr.s_addr, buf);

	addr.s_addr = 0x0;
	strcpy(buf, "192.168.1.111");
	inet_pton(AF_INET, buf, &addr);
	printf("inet_pton-> %08x %s\n", addr.s_addr, buf);
}

int main(int argc, char *argv[])
{
	x2y();

	return 0;
}

