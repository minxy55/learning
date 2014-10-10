
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

unsigned short inet_cksum(unsigned short *addr, int nleft)
{
	/*
	 * Our algorithm is simple, using a 32 bit accumulator,
	 * we add sequential 16 bit words to it, and at the end, fold
	 * back all the carry bits from the top 16 bits into the lower
	 * 16 bits.
	 */
	unsigned sum = 0;
	while (nleft > 1) {
		sum += *addr++;
		nleft -= 2;
	}

	/* Mop up an odd byte, if necessary */
	if (nleft == 1) {
//		if (BB_LITTLE_ENDIAN)
			sum += *(unsigned char*)addr;
//		else
//			sum += *(unsigned char*)addr << 8;
	}

	/* Add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
	sum += (sum >> 16);                     /* add carry */

	return (unsigned short)~sum;
}

