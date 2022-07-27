#define ALLOCATE_EXTERN
#include "fx2regs.h"
#include "fx2types.h"
#include "delay.h"
#include "fx2macros.h"

typedef enum
{
    INPUT = 0,
    OUTPUT = 1
} PortMode;

typedef enum
{
    LOW = 0,
    HIGH = 1
} OutputLevel;

void pinMode(int pin, PortMode mode)
{
    if(pin < 8)
    {
        OEA = (OEA | (mode << pin)); // modify pin bit with mode
    }
}

void digitalWrite(unsigned char pin, OutputLevel level)
{
    if(pin < 8)
    {
        IOA = (IOA ^ (level << pin));
    }

}
void main(void)
{
    delay(1);
    
    pinMode(0,OUTPUT);
    for (;;)
    {
		digitalWrite(0,LOW);
        delay(1000); // warning: delay function is not working properly need to configure cpu freq.
		digitalWrite(0,HIGH);
        delay(1000);
    }
}
