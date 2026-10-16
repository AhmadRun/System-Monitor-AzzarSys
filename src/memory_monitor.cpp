#include "memory_monitor.h"
#include <fstream>
#include <sstream>

MemoryInfo MemoryMonitor::getMemoryInfo() {
    MemoryInfo info = {};
    unsigned long available = 0;
    unsigned long swapFree = 0;
    bool hasAvailable = false;

    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    
    while (std::getline(meminfo, line)) {
        std::istringstream iss(line);
        std::string key;
        unsigned long value = 0;

        if (!(iss >> key >> value)) continue;

        if (key == "MemAvailable:") {
            available = value * 1024;
            hasAvailable = true;
        } else if (key == "MemTotal:") {
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
            swapFree = value * 1024;
        }
    }
    meminfo.close();
    
    // Used memory = total - available (same as `free`). Older kernels without
    // MemAvailable fall back to total - free - buffers - cached.
    unsigned long reclaimable = hasAvailable
        ? available
        : info.freeMemory + info.buffers + info.cached;
    info.usedMemory = info.totalMemory > reclaimable ? info.totalMemory - reclaimable : 0;
    info.usedSwap = info.totalSwap > swapFree ? info.totalSwap - swapFree : 0;
    
    // Calculate percentages
    if (info.totalMemory > 0) {
        info.memoryPercent = (100.0 * info.usedMemory) / info.totalMemory;
    }
    
    if (info.totalSwap > 0) {
        info.swapPercent = (100.0 * info.usedSwap) / info.totalSwap;
    }
    
    return info;
}
