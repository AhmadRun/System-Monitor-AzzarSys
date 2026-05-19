#pragma once

#include <string>
#include <ctime>

struct SystemInfo {
    std::string hostname;
    std::string osVersion;
    std::string kernelVersion;
    int cpuCoreCount;
    unsigned long uptime;
    std::time_t bootTime;
};

class SystemInfoMonitor {
public:
    SystemInfo getSystemInfo();
};
