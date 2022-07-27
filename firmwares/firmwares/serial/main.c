#define ALLOCATE_EXTERN
#include "fx2regs.h"
#include "fx2types.h"
#include "delay.h"
#include "fx2macros.h"

#include <stdbool.h>
#include <stdio.h>


volatile unsigned char sendcounter;
volatile unsigned int senddata;
volatile _Bool sending;

void send_bit(void) __interrupt(3)
{
	TH1 = (65536 - 833) / 256;
	TL1 = (65536 - 833) % 256;

	if(!sending)
		return;

	IOA = senddata & 1;
	senddata >>= 1;

	if(!--sendcounter)
	{
		sending = false;
		IE &= ~0x08;
	}
}

int putchar(int c)
{
	while(sending);

	senddata = (c << 1) | 0x200;

	sendcounter = 10;

	sending = true;
	IE |= 0x08;

	return (c);
}

void main(void)
{
	unsigned long int i = 0;

	// Configure timer for 1200 baud
	TH1 = (65536 - 833) / 256;
	TL1 = (65536 - 833) % 256;
	TMOD = 0x10;
	IE |= 0x80;
	TCON |= 0x40; // Start timer

	OEA=0x01; // port A as output

	
	for(;;)
	{
		printf("Hello World!\n");
		for(i = 0; i < 147456; i++); // Sleep
	}
}