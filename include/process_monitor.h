#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct ProcessInfo {
    int pid;
    std::string name;
    std::string user;
    double cpuPercent;
    double memoryPercent;
    unsigned long memoryMB;
    std::string state;
};

struct ProcessTicks {
    unsigned long long utime = 0;
    unsigned long long stime = 0;
};

class ProcessMonitor {
public:
    std::vector<ProcessInfo> getTopProcesses(int count = 10);

private:
    std::unordered_map<int, ProcessTicks> prevTicks;
    unsigned long long prevTotalTicks = 0;
    bool hasPrev = false;

    unsigned long long getTotalSystemTicks();
    ProcessInfo parseProcessInfo(int pid, std::unordered_map<int, ProcessTicks>& ticks);
};
