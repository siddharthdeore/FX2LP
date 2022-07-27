//simple firmware for testing read and write over EP1 

#define ALLOCATE_EXTERN
#include "fx2regs.h"

#define NOP   _asm \ nop; \ nop; \ nop; \ nop; \ _endasm;
#undef NOP
#define NOP

void main(void)
{
unsigned char b,i;

REVCTL=0x03;NOP;

while (1)
	{
	NOP;EP1OUTBC=1;NOP;		//arm EP1 for the host to write
	while (EP1OUTCS & bmBIT1){}	//wait until data is available from host
	NOP;b=EP1OUTBC;NOP;		//number of bytes received

	for (i=0;i<b;i++)
		EP1INBUF[i]=EP1OUTBUF[i]+3;	//just return the bytes, each incremented by 3

	NOP;EP1INBC=b;NOP;		//arm EP1 for host to read
	while (EP1INCS & bmBIT1){}	//wait until host has read the data
	}

}