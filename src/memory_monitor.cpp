#include "memory_monitor.h"
#include <fstream>
#include <sstream>

MemoryInfo MemoryMonitor::getMemoryInfo() {
    MemoryInfo info = {};
    
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    
    while (std::getline(meminfo, line)) {
        std::istringstream iss(line);
        std::string key;
        unsigned long value;
        
        iss >> key >> value;
        
        if (key == "MemTotal:") {
            info.totalMemory = value * 1024;  // Convert KB to bytes
        } else if (key == "MemFree:") {
            info.freeMemory = value * 1024;
        } else if (key == "Buffers:") {
            info.buffers = value * 1024;
        } else if (key == "Cached:") {
            info.cached = value * 1024;
        } else if (key == "SwapTotal:") {
            info.totalSwap = value * 1024;
        } else if (key == "SwapFree:") {
            unsigned long swapFree = value * 1024;
            info.usedSwap = info.totalSwap - swapFree;
        }
    }
    meminfo.close();
    
    // Calculate used memory (total - free - buffers - cached)
    info.usedMemory = info.totalMemory - info.freeMemory - info.buffers - info.cached;
    
    // Calculate percentages
    if (info.totalMemory > 0) {
        info.memoryPercent = (100.0 * info.usedMemory) / info.totalMemory;
    }
    
    if (info.totalSwap > 0) {
        info.swapPercent = (100.0 * info.usedSwap) / info.totalSwap;
    }
    
    return info;
}
