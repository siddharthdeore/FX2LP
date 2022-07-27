#ifndef DUMPWRITER_H
#define DUMPWRITER_H

#pragma once
#include <iostream>
#include <bitset>
#include <vector>
template <typename T>
class DumpWriter
{
private:
    /* data */
    std::vector<T> _data;
    uint32_t _cnt;

    std::ofstream outdata; // outdata is like cin
public:
    DumpWriter(/* args */)
    {
        _data.reserve(512 * 512 * 512);
        _cnt = 0;
    }
    ~DumpWriter()
    {
        std::cout << "Writing data to dump!" << std::endl;
        outdata.open("dump.txt");
        for (size_t i = 0; i < _cnt; i++)
        {
            outdata << std::bitset<8>(_data[i]) << std::endl;
        }
        std::cout << _cnt << " lines dumped" << std::endl;
    }
    int getSize()
    {
        return _data.size();
    }
    void append(T var)
    {
        _data.push_back(var);
        _cnt++;
    }
    void append(T arr[], int cnt)
    {
        _data.insert(_data.end(), arr, arr + cnt);
        _cnt += cnt;
    }
};

#endif