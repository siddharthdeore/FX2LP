#include <stdio.h>
#include <usb.h>

int main()
{
    struct usb_bus *p;
    struct usb_device *q;
    struct usb_device *current_device;
    usb_dev_handle *current_handle;

    unsigned char reset[2] = {1, 0};
    unsigned char buffer[10000];
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
    while (p != NULL) // find the CY7C68013
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

    er[4] = usb_control_msg(current_handle, 0x40, 0xa0, 0xE600, 0, reset, 1, 1000); // RESET
    sleep(0.1);

    f = fopen("endpoint.ihx", "r"); // load firmware from intel hex file
    if (f == NULL)
    {
        printf("\n\nCould not find file: endpoint.ihx\n\n");
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

    er[6] = usb_control_msg(current_handle, 0x40, 0xa0, 0xE600, 0, reset + 1, 1, 1000); // UNRESET
    sleep(0.1);

    printf("\n SETCONFIG=%d", usb_set_configuration(current_handle, 1));

    er[7] = usb_claim_interface(current_handle, 0);
    er[8] = usb_set_altinterface(current_handle, 1);

    buffer[0] = 0x41;
    buffer[1] = 0x42;
    buffer[2] = 0x43;
    buffer[3] = 0x44;
    buffer[4] = 0x45; // just some numbers

    printf("\n Before: ");
    for (i = 0; i < 5; i++)
        printf(" %02x", buffer[i]);
    printf("\n");

    for (j = 0; j < 10; j++)
    {
        er[9] = usb_bulk_write(current_handle, endpoint, buffer, 5, 1000); // send 5 bytes
        sleep(0.1);
        er[10] = usb_bulk_read(current_handle, endpoint, buffer, 5, 1000); // receive 5 bytes
        printf("\n After: ");
        for (i = 0; i < 5; i++)
            printf(" %02x", buffer[i]);
        printf("     status = %d  %d  \n", er[9], er[10]);
        sleep(0.1);
        for (i = 0; i < 3; i++)
            buffer[i]--; // decrement, to see that data goes both ways each time
    }

    usb_release_interface(current_handle, 0);
    usb_close(current_handle);

    return 0;
}