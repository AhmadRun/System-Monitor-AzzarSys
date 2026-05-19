#pragma once

#include <string>
#include <vector>

struct CPUInfo {
    double totalUsage;
    std::vector<double> coreUsage;
    double loadAvg1;
    double loadAvg5;
    double loadAvg15;
};

class CPUMonitor {
public:
    CPUInfo getCPUInfo();

private:
    struct CPUTimes {
        long total = 0;
        long idle = 0;
    };
    CPUTimes prevTotal;
    std::vector<CPUTimes> prevCores;
    bool hasPrev = false;
};
