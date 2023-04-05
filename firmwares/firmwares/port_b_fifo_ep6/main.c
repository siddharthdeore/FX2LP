#define ALLOCATE_EXTERN
#include "fx2regs.h"
#include "fx2macros.h"
#include "delay.h"

//! Initialize cpu clock, fifo and bulk-in end point 6
static void initialize(void)
{
    // Set DYN_OUT and ENH_PKT bits, as recommended by the TRM.
    REVCTL = bmNOAUTOARM | bmSKIPCOMMIT; // REVCTL = 0x03;
    
    // CPUCS = 0x10; // 48 MHz, CLKOUT output disabled.
    // SYNCDELAY4; // Internal IFCLK, 48MHz; A,B as normal ports.
    //  Slave FIFO, sync mode, 48MHz clock
    IFCONFIG = bmIFCFGMASK | bmIFCLKSRC | bmIFCLKOE | bmIFCLKPOL | bm3048MHZ;
    SYNCDELAY4;

	SETCPUFREQ(CLK_48M);
    SYNCDELAY4;

    /*
        EP2CFG = 0xa2; // EP2 BULK-OUT
        EP4CFG = 0xa0; // EP4 BULK-OUT
        EP6CFG = 0xe2; // EP6 BULK-IN
        EP8CFG = 0xe0; // EP8 BULK-IN
    */
    EP6CFG = 0xe2;
    SYNCDELAY4; // 1110 0010 (bulk IN, 512 bytes, double-buffered)
    FIFORESET = 0x80;
    SYNCDELAY4; // NAK all requests from host.
    FIFORESET = 0x82;
    SYNCDELAY4; // Reset individual EP (2,4,6,8)
    FIFORESET = 0x84;
    SYNCDELAY4;
    FIFORESET = 0x86;
    SYNCDELAY4;
    FIFORESET = 0x88;
    SYNCDELAY4;
    FIFORESET = 0x00;
    SYNCDELAY4; // Resume normal operation.
}

void main(void)
{
    initialize();

    // set PORT-B to input
    OEB = 0x0;
    SYNCDELAY4;
    for (;;)
    {
        // Wait for the EP6 buffer to become non-full.
        if (!(EP2468STAT & bmEP6FULL))
        {

            for (int i = 0; i < 512; i++)
            {
                EP6FIFOBUF[i] = IOB;
            }

            // Arm the endpoint. Be sure to set BCH before BCL because BCL access
            // actually arms the endpoint.
            // SYNCDELAY4;
            EP6BCH = 0x02; // commit 512 bytes
            SYNCDELAY4;
            EP6BCL = 0x00;
            SYNCDELAY4;
        }
    }
}