#include <iostream>
#include <bitset>
#include <time.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <signal.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#endif
#include <vector>
#include <fstream>
#include <FX2/DumpWriter.h>
#include <chrono>
static volatile int keepRunning = 1;

void error_verbose_handle(std::string _str, int err)
{
    std::cout << _str << " : " << libusb_error_name(err) << "\n";
}
void intHandler(int dummy)
{
    keepRunning = 0;
    std::cout << "\nCTRL + C Exit!" << std::endl;
}

int main(int argc, char const *argv[])
{

    signal(SIGINT, intHandler);
    // find usb
    libusb_device_handle *handle;
    libusb_context *context = NULL;

    unsigned char reset[2] = {1, 0};
    int endpoint = 0x86;

    int i, j, tlen;
    FILE *file_handle;
    char s[256];
    int length;
    int addr;
    int type;
    unsigned char data[263];
    unsigned char checksum, a;
    unsigned int b;

    int ret;
    libusb_init(&context);
    handle = libusb_open_device_with_vid_pid(context, 0x04b4, 0x8613);
    if (handle == NULL)
    {
        std::cout << "Failed : libusb_open_device_with_vid_pid\n";
        return EXIT_FAILURE;
    }

    ret = libusb_claim_interface(handle, 0);
    if (ret < 0)
    {
        error_verbose_handle("Claim interface failed", ret);
    }
    ret = libusb_control_transfer(handle, 0x40, 0xa0, 0xE600, 0, reset, 1, 1000);

    file_handle = fopen("fifo_ep6_fw.ihx", "r");
    if (file_handle == NULL)
    {
        printf("\n\nCould not find file: fifo_ep6_fw.ihx\n\n");
        exit(0);
    }

    while (!feof(file_handle))
    {
        fgets(s, 263, file_handle);
        sscanf(s + 1, "%02x", &length);
        sscanf(s + 3, "%04x", &addr);
        sscanf(s + 7, "%02x", &type);
        if (type == 0)
        {
            printf("\n Programming %3d byte%s starting at 0x%04x", length, length == 1 ? " " : "s", addr);
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
                std::cout << "  ** Checksum bad";
            for (i = addr; i < addr + length; i += 16)
            {
                tlen = addr + length - i;
                if (tlen > 16)
                    tlen = 16;
                ret = libusb_control_transfer(handle, 0x40, 0xa0, i, 0, data + (i - addr), tlen, 1000);
            }
        }
        else if (type == 0x01)
        {
            std::cout << "\nEnd of file\n";
            break;
        }
        else if (type == 0x02)
        {
            std::cout << "Extended address?\n";
            continue;
        }
    }
    fclose(file_handle);

    // reset
    ret = libusb_control_transfer(handle, 0x40, 0xa0, 0xE600, 0, reset + 1, 1, 1000); // UNRESET
    if (ret < 0)
    {
        error_verbose_handle("libusb_control_transfer failed", ret);
    }
    sleep(0.1);

    // claim interface
    ret = libusb_claim_interface(handle, 0);
    if (ret < 0)
    {
        error_verbose_handle("libusb_claim_interface failed", ret);
    }
    ret = libusb_set_interface_alt_setting(handle, 0, 1);
    if (ret < 0)
    {
        error_verbose_handle("libusb_set_interface_alt_setting failed", ret);
    }
    // keep reading

    unsigned char reply[512];
    int received_count;

    DumpWriter<uint8_t> dump;

    auto start = std::chrono::high_resolution_clock::now();
    while (keepRunning)
    {
        ret = libusb_bulk_transfer(handle, 0x86, reply, 512, &received_count, 100); // receive reply
        if (ret < 0)
        {
            break;
        }
        usleep(10);
        //dump.append(reply, received_count);
       // /*

        for (i = 0; i < received_count; i++)
        {
             //std::cout << std::bitset<8>(reply[i]) << "\n";
            //dump.append(reply[i]);
            // reply[i] = 0;
        }
        //*/
    }
    auto finish = std::chrono::high_resolution_clock::now();
    auto sz = dump.getSize();
    double time_in_sec = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() / 1e9;

    std::cout << "\n Bytes " << sz << " in " << time_in_sec << "sec";

    std::cout << "\n Bandwidth " << sz / (time_in_sec * 1024) << "KB/sec \n";

    return 0;
}
