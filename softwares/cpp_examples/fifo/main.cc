#include <iostream>
#include <bitset>

#include <FX2/FX2.h>
#include <FX2/DumpWriter.h>

#include <atomic>
#include <signal.h>

std::atomic<bool> keepRunning;

int main(int argc, char const *argv[])
{
    // Register signal and signal handler
    signal(SIGINT, [](int signum)
           { keepRunning = false; });

    FX2 dev;
    DumpWriter<uint8_t> dump;

    dev.print_devices();

    dev.load_firmware("fifo_ep6_fw.ihx");

    unsigned char in_buffer[512];
    keepRunning = true;

    while (keepRunning)
    {
        int received_count = dev.ep6_bulkin(in_buffer, 512);
        // dump.append(in_buffer, received_count);

        for (int i = 0; i < received_count; i++)
            dump.append(in_buffer[i]);
        //     std::cout << std::bitset<8>(in_buffer[i]) << "\n";
    }

    return 0;
}