#include <iostream>
#include <bitset>

#include <FX2/FX2.h>
#include <FX2/DumpWriter.h>

#include <atomic>
#include <signal.h>
#include <chrono>

std::atomic<bool> keepRunning;

int main(int argc, char const *argv[])
{
    // Register signal and signal handler
    signal(SIGINT, [](int signum)
           { keepRunning = false; });

    FX2 dev;
    DumpWriter<uint8_t> dump;

    dev.print_devices();

    dev.load_firmware("port_b_fifo_ep6_fw.ihx");

    unsigned char in_buffer[512];
    unsigned char tbuf;
    keepRunning = true;

    auto start_time = std::chrono::high_resolution_clock::now();
    while (keepRunning)
    {
        int received_count = dev.ep6_bulkin(in_buffer, 512);
        dump.append(in_buffer, received_count);
        /*
        for (int i = 0; i < received_count; i++)
        {
            if (tbuf != in_buffer[i])
            {
                std::cout << std::bitset<8>(in_buffer[i]) << "\n";
            }
            tbuf = in_buffer[i];
        }
       */ 
    }
    auto stop_time = std::chrono::high_resolution_clock::now();
    double time_in_sec = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time - start_time).count() / 1e9;

    std::cout << "\n"
              << std::endl;
    std::cout << "Duratin        : " << time_in_sec << " secs" << std::endl;
    std::cout << "Bytes recorded : " << dump.getSize() << " Bytes" << std::endl;
    std::cout << "Rate           : " << 8 * dump.getSize() / (time_in_sec * 1024 * 1024) << " Mbps\n\n"
              << std::endl;

    return 0;
}