#pragma once

#include <string>
#include <ctime>

struct SystemInfo {
    std::string hostname;
    std::string osVersion;
    std::string kernelVersion;
    int cpuCoreCount = 0;
    unsigned long uptime = 0;
    std::time_t bootTime = 0;
};

class SystemInfoMonitor {
public:
    SystemInfo getSystemInfo();
};
