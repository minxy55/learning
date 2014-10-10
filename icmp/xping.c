#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include "inet_chksum.c"
#include "raw_sock.c"

int main(int argc, char *argv[])
{
	unsigned char txbuf[256];
	unsigned char rxbuf[256];
	struct in_addr local_addr;
	struct in_addr remot_addr;
	struct sockaddr_in local_saddr;
	struct sockaddr_in remot_saddr;
	socklen_t socklen;
	struct iphdr *iph;
	struct icmphdr *ich;
	struct timeval start, end;
	int sock, ttl = 64;
	int i, n, one, bytes;

	if (argc < 3) {
		printf("Usage: %s local remote\n", argv[0]);
		return -1;
	}

	local_addr.s_addr = inet_addr(argv[1]);
	remot_addr.s_addr = inet_addr(argv[2]);
	n = argc == 4 ? atoi(argv[3]) : 4;

	sock = raw_socket_create();

	while (sock > 0 && n-- > 0) {
		memset(txbuf, 0, sizeof(txbuf));
		iph = (struct iphdr*)txbuf;
		ich = (struct icmphdr*)(iph + 1);

		iph->version	= 4;
		iph->ihl		= 5;
		iph->ttl		= ttl;
		iph->tos		= 0;
		iph->tot_len	= 64;
		iph->frag_off	= 0;
		iph->check		= 0;
		iph->id			= (unsigned short)rand();
		iph->protocol	= IPPROTO_ICMP;
		iph->saddr		= local_addr.s_addr;
		iph->daddr		= remot_addr.s_addr;

		ich->type		= ICMP_ECHO;
		ich->code		= 0x00;
		ich->un.echo.id = getpid();
		ich->un.echo.sequence = 0;
		ich->checksum	= 0;
		ich->checksum	= inet_cksum((unsigned short*)ich, 44);

		gettimeofday(&start, NULL);
		bytes = raw_socket_xmit(sock, txbuf, 64, remot_addr);
		if (bytes < 0) {
			break;
		}

		bytes = raw_socket_recvfrom(sock, rxbuf, sizeof(rxbuf), &remot_saddr);
		if (bytes < 0) {
			break;
		}
		gettimeofday(&end, NULL);

		iph = (struct iphdr*)rxbuf;
		ich = (struct icmphdr*)(rxbuf + (iph->ihl << 2));
		printf("replay from %s  bytes %d, type %x, code %x, ttl %d, diff %ldms\n", 
			inet_ntoa(remot_saddr.sin_addr), bytes, ich->type, ich->code, iph->ttl,
			(end.tv_sec * 1000 + (end.tv_usec / 1000)) - (start.tv_sec * 1000 + (start.tv_usec / 1000)));

		sleep(1);
	}

	close(sock);

	return 0;
}

