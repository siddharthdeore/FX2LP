#ifndef FX2_H
#define FX2_H

#pragma once
#include <iostream>
#include <iomanip>
#include <fstream>
#include <libusb-1.0/libusb.h>
#include <memory.h>
class FX2
{
private:
    libusb_context *_context = NULL;
    libusb_device **_list;
    libusb_device_handle *_handle;
    int ret;
    uint16_t _vid, _pid;

public:
    std::unique_ptr<FX2> ptr;

    /**
     * @brief Construct a new FX2 object
     *
     * @param vendor_id Vender ID
     * @param product_id Product ID
     */
    FX2(const uint16_t vendor_id = 0x04b4, const uint16_t product_id = 0x8613)
    {
        _vid = vendor_id;
        _pid = product_id;

        // initialize usb and get contect
        libusb_init(&_context);
        // open device
        _handle = libusb_open_device_with_vid_pid(_context, _vid, _pid);
        if (_handle == NULL)
        {
            std::cout << "Failed : libusb_open_device_with_vid_pid\n";
            return;
        }

        // claim interface
        if (libusb_claim_interface(_handle, 0) < 0)
        {
            std::cout << "Failed : libusb_claim_interface\n";
        }

        // set alternate interface
        if (libusb_set_interface_alt_setting(_handle, 0, 1) < 0)
        {
            std::cout << "Failed : libusb_claim_interface\n";
        }
    }

    ~FX2()
    {
    }
    void reset_device()
    {
        unsigned char reset[] = {1};
        if (libusb_control_transfer(_handle, 0x40, 0xA0, 0xE600, 0, reset, 1, 1000) < 0)
        {
            std::cout << "Failed : libusb_control_transfer\n";
        }
        else
        {
            std::cout << "Device reset \n";
        }
    }
    void unreset_device()
    {
        unsigned char reset[] = {0};
        if (libusb_control_transfer(_handle, 0x40, 0xA0, 0xE600, 0, reset, 1, 1000) < 0)
        {
            std::cout << "Failed : libusb_control_transfer\n";
        }
        else
        {
            std::cout << "Device Running \n";
        }
    }
    void load_firmware(std::string firmware_path)
    {

        reset_device();
        int number_of_lines = 0;
        int total_lines = 0;

        std::ifstream temp_file(firmware_path, std::ios::binary);

        for (std::string line; std::getline(temp_file, line);)
            ++total_lines;
        temp_file.close();
        std::ifstream infile(firmware_path, std::ios::binary);

        unsigned char data[1024];
        int length, addr, type, tlen;
        unsigned char checksum, a;
        unsigned int b;

        std::cout << "Writing firmware to " << std::hex << std::setw(4) << _vid << ":"
                  << std::hex << std::setw(4) << _pid << std::endl;
        for (std::string line_str; std::getline(infile, line_str);)
        {
            char *line = &line_str[0];

            std::sscanf(line + 1, "%02x", &length); // byte 1 to 2
            std::sscanf(line + 3, "%04x", &addr);   // byte 3 to 6
            std::sscanf(line + 7, "%02x", &type);   // byte 7 to 8
            switch (type)
            {
            case 0:
                std::cout << "Writing " << std::setw(3) << length << " bytes "
                          << "starting at 0x" << std::setw(4) << addr << " " << std::dec << (++number_of_lines + 1) * 100 / total_lines << "%" << std::endl;
                a = length + (addr & 0xff) + (addr >> 8) + type;
                for (int i = 0; i < length; i++)
                {
                    std::sscanf(line + 9 + i * 2, "%02x", &b);
                    data[i] = b;
                    a = a + data[i];
                }
                sscanf(line + 9 + length * 2, "%02x", &b);
                checksum = b;

                if (((a + checksum) & 0xff) != 0x00)
                    std::cout << "  ** Checksum bad";

                for (int i = addr; i < addr + length; i += 16)
                {
                    tlen = addr + length - i;
                    if (tlen > 16)
                        tlen = 16;
                    ret = libusb_control_transfer(_handle, 0x40, 0xA0, i, 0, data + (i - addr), tlen, 1000);
                }

                continue;

            case 0x01:
                std::cout << "Firmware upladed to ram" << std::endl;
                continue;

            case 0x02:
                std::cout << "Extended address?\n";
                continue;

            default:
                break;
            }
        }

        infile.close();
        unreset_device();

        // claim interface
        ret = (libusb_claim_interface(_handle, 0));

        // set alternate interface
        ret = libusb_set_interface_alt_setting(_handle, 0, 1);
    }
    void print_devices()
    {
        ssize_t cnt = libusb_get_device_list(_context, &_list);
        std::cout << "Devices found" << std::endl;
        for (int i = 0; i < cnt; i++)
        {
            struct libusb_device_descriptor desc;
            libusb_device *device = _list[i];

            if (libusb_get_device_descriptor(device, &desc) > -1)
            {
                std::cout << std::hex << std::setfill('0') << std::setw(4) << desc.idVendor << ":"
                          << std::hex << std::setfill('0') << std::setw(4) << desc.idProduct
                          << std::dec << ", (bus " << (int)libusb_get_bus_number(device) << ", device " << (int)libusb_get_device_address(device) << ")\n";
            }
        }
        libusb_free_device_list(_list, 1);
    }
    /**
     * @brief End-Point 6 Bulk in transfer from host to device
     *
     * @param buffer
     * @param length
     * @return int returns number of bytes received
     */
    int ep6_bulkin(unsigned char *buffer, const int length = 512)
    {
        int received_count;
        if (libusb_bulk_transfer(_handle, 0x86, buffer, length, &received_count, 100) < 0) // receive reply
        {
            return -1;
        }
        return received_count;
    }
};

#endif