#pragma once

#include <string>
#include <vector>

struct DiskInfo {
    std::string mountPoint;
    std::string filesystem;
    unsigned long totalSize;
    unsigned long usedSize;
    unsigned long availableSize;
    double usedPercent;
};

class DiskMonitor {
public:
    std::vector<DiskInfo> getDiskInfo();
};
