#define ALLOCATE_EXTERN
#include "fx2regs.h"

#define NOP     \
    _asm \ nop; \
    \ nop;      \
    \ nop;      \
    \ nop;      \
    \ _endasm;
#undef NOP
#define NOP

void main(void)
{
    unsigned char a, b;
    REVCTL = 0x03;
    NOP;

    while (1)
    {
        NOP;
        EP1OUTBC = 1;
        NOP; // arm EP1 for the host to write
        while (EP1OUTCS & bmBIT1)
        {
        } // wait until data is available from host
        NOP;
        b = EP1OUTBC;
        NOP; // number of bytes received
        a = EP1OUTBUF[0];
        switch (a)
        {
        case 1: // set port directions
        {
            OEA = EP1OUTBUF[1];
            OEB = EP1OUTBUF[2];
            OEC = EP1OUTBUF[3];

            OED = EP1OUTBUF[4];
            OEE = EP1OUTBUF[5];

            EP1INBUF[0] = OEA;
            EP1INBUF[1] = OEB;
            EP1INBUF[2] = OEC;
            EP1INBUF[3] = OED;
            EP1INBUF[4] = OEE;
            NOP;
            EP1INBC = 5;
            NOP; // arm EP1 for host to read
            while (EP1INCS & bmBIT1)
            {
            } // wait until host has read the data
            break;
        }
        case 2: // read all ports
        {
            EP1INBUF[0] = IOA;
            EP1INBUF[1] = IOB;
            EP1INBUF[2] = IOC;
            EP1INBUF[3] = IOD;
            EP1INBUF[4] = IOE;
            NOP;
            EP1INBC = 5;
            NOP; // arm EP1 for host to read
            while (EP1INCS & bmBIT1)
            {
            } // wait until host has read the data
            break;
        }
        case 3: // write all ports
        {
            IOA = EP1OUTBUF[1];
            IOB = EP1OUTBUF[2];
            IOC = EP1OUTBUF[3];
            IOD = EP1OUTBUF[4];
            IOE = EP1OUTBUF[5];
            break;
        }
        default:
        {
        }
        }
    }
}