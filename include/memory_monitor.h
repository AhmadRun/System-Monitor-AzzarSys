#pragma once

#include <string>

struct MemoryInfo {
    unsigned long totalMemory;
    unsigned long usedMemory;
    unsigned long freeMemory;
    unsigned long buffers;
    unsigned long cached;
    unsigned long totalSwap;
    unsigned long usedSwap;
    double memoryPercent;
    double swapPercent;
};

class MemoryMonitor {
public:
    MemoryInfo getMemoryInfo();
};
