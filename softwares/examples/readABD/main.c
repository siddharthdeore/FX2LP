#include <stdio.h>
#include <usb.h>
#include <signal.h>
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"

#define BYTE_TO_BINARY(byte)       \
    (byte & 0x80 ? '1' : '0'),     \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')
static volatile int keepRunning = 1;

#define USB_TIMEOUT 100

const unsigned char CMD_READ[] = {0x2};
void intHandler(int dummy)
{
    keepRunning = 0;
    printf("\nCTRL + C Exit!");
}
int main()
{

    signal(SIGINT, intHandler);

    struct usb_bus *p;
    struct usb_device *q;
    struct usb_device *current_device;
    usb_dev_handle *current_handle;

    unsigned char reset[2] = {1, 0};
    unsigned char buffer[512];
    unsigned char lbuff;
    int er[10];
    int endpoint = 1;
    int i, j, tlen;
    FILE *f;
    unsigned char s[1024];
    int length;
    int addr;
    int type;
    unsigned char data[256];
    unsigned char checksum, a;
    unsigned int b;

    usb_init();
    er[2] = usb_find_busses();
    er[3] = usb_find_devices();

    p = usb_busses;
    current_device = NULL;
    while (p != NULL) // find the CY7C68013 on the bus
    {
        q = p->devices;
        while (q != NULL)
        {
            if ((q->descriptor.idVendor == 0x4b4) && (q->descriptor.idProduct == 0x8613))
                current_device = q;
            q = q->next;
        }
        p = p->next;
    }
    if (current_device == NULL)
    {
        printf("\n\nCould not find a CY7C68013\n\n");
        exit(0);
    }
    fflush(stdout);

    current_handle = usb_open(current_device);

    er[4] = usb_control_msg(current_handle, 0x40, 0xa0, 0xE600, 0, reset, 1, USB_TIMEOUT); // RESET
    sleep(0.1);

    f = fopen("portsABD.ihx", "r"); // load firmware from intel hex file
    if (f == NULL)
    {
        printf("\n\nCould not find file: portsABD.ihx\n\n");
        exit(0);
    }
    while (!feof(f))
    {
        fgets(s, 1024, f); /* we should not use more than 263 bytes normally */
        sscanf(s + 1, "%02x", &length);
        sscanf(s + 3, "%04x", &addr);
        sscanf(s + 7, "%02x", &type);
        if (type == 0)
        {
            printf("Programming %3d byte%s starting at 0x%04x", length, length == 1 ? " " : "s", addr);
            a = length + (addr & 0xff) + (addr >> 8) + type;
            for (i = 0; i < length; i++)
            {
                sscanf(s + 9 + i * 2, "%02x", &b);
                data[i] = b;
                a = a + data[i];
            }
            sscanf(s + 9 + length * 2, "%02x", &b);
            checksum = b;
            if (((a + checksum) & 0xff) != 0x00)
                printf("  ** Checksum bad");
            for (i = addr; i < addr + length; i += 16)
            {
                tlen = addr + length - i;
                if (tlen > 16)
                    tlen = 16;
                er[5] = usb_control_msg(current_handle, 0x40, 0xa0, i, 0, data + (i - addr), tlen, 1000);
            }
        }
        else if (type == 0x01)
        {
            printf("End of file\n");
        }
        else if (type == 0x02)
        {
            printf("Extended address?\n");
            continue;
        }
    }
    fclose(f);

    er[6] = usb_control_msg(current_handle, 0x40, 0xa0, 0xE600, 0, reset + 1, 1, USB_TIMEOUT); // UNRESET
    sleep(0.1);

    er[7] = usb_claim_interface(current_handle, 0);
    er[8] = usb_set_altinterface(current_handle, 1);

    // first try if we can set port directions

    buffer[0] = 0x1;
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0;
    buffer[4] = 0;
    buffer[5] = 0x55;
    er[9] = usb_bulk_write(current_handle, endpoint, buffer, 6, 1000);
    sleep(0.1);
    er[10] = usb_bulk_read(current_handle, endpoint, buffer, 5, 1000);
    sleep(0.1);
    printf("\n Port directions: ");
    for (i = 0; i < 5; i++)
        printf(" %02x", buffer[i]);
    printf("    Status: %d  %d \n", er[9], er[10]);
    sleep(0.5);

    // then do some port reading - the values returned should reflect the levels on the pins

    while (keepRunning)
    {
        buffer[0] = 0x2; //"read all ports" command

        unsigned char command[] = {0x3};
        unsigned char reply[512];
        int status = usb_bulk_write(current_handle, endpoint, command, 1, 1000); // send command
        usleep(100);
        int received_count = usb_bulk_read(current_handle, endpoint, reply, 512, 100000); // receive reply
        usleep(100);
        for (size_t i = 0; i < 512; i++)
        {
            printf(BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(reply[i]));
        }
        printf("\n %d \n", received_count);

        /*
            //if (reply[1] != lbuff)
            {
                printf("Ports: ");
                for (i = 0; i < 5; i++)
                {
                    // printf(" %02x", reply[i]);
                    printf(" " BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(reply[i]));
                }
                printf("    Status: %d  %d \n", status, received_count);
            }
        */
        // lbuff = reply[1];
    }

    usb_release_interface(current_handle, 0);
    usb_close(current_handle);

    return 0;
}