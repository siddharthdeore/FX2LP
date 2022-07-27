#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

#define TIMEOUT 2000 // in milliseconds

// Hopefully the version string length won't overflow this
#define BUFSIZE 200

#define VERSION_REQUEST 0x94

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <bus_number> <device_address>\n", argv[0]);
        return 1;
    }
    libusb_init(0);
    libusb_device **list;
    libusb_device *fpga = 0;
    int n = libusb_get_device_list(0, &list);
    printf("%d usb devices available\n", n);
    int i;
    for (i = 0; i < n; i++) {
        libusb_device *device = list[i];
        if (libusb_get_bus_number(device) == atoi(argv[1]) && libusb_get_device_address(device) == atoi(argv[2])) {
            fpga = device;
            break;
        }
    }
    if (i == n) {
        fprintf(stderr, "Error: no usb device with specified bus number and device address accessible\n");
        return 1;
    }
    libusb_device_handle *dh = 0;
    int err = libusb_open(fpga, &dh);
    if (err != 0) {
        fprintf(stderr, "Error opening device: %d\n", err);
        return 1;
    }
    char version[BUFSIZE];
    for (i = 0; i < BUFSIZE; i++) {
        version[i] = '\0';
    }
    int ret = libusb_control_transfer(dh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, VERSION_REQUEST, 0, 0, version, BUFSIZE, TIMEOUT);
    if (ret < 0) {
        fprintf(stderr, "Error asking for version info: %d\n", ret);
        return 1;
    }

    printf("Firmware version reported: ");
    for (i = 0; i != 200 && version[i] != '\0'; i++) {
        printf("%c", version[i]);
    }
    printf("\n");
    libusb_free_device_list(list, 0);
    return 0;
}
