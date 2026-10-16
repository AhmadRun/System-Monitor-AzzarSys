#pragma once

#include <string>
#include <vector>

struct CPUInfo {
    double totalUsage = 0.0;
    std::vector<double> coreUsage;
    double loadAvg1 = 0.0;
    double loadAvg5 = 0.0;
    double loadAvg15 = 0.0;
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
