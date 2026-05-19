#include "cpu_monitor.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

CPUInfo CPUMonitor::getCPUInfo() {
    CPUInfo info;

    // Read /proc/stat
    std::ifstream statFile("/proc/stat");
    std::string line;
    std::vector<std::vector<long>> cpuTimes;

    while (std::getline(statFile, line)) {
        if (line.substr(0, 3) != "cpu") break;
        std::istringstream iss(line);
        std::string cpu;
        iss >> cpu;
        std::vector<long> times;
        long val;
        while (iss >> val) times.push_back(val);
        if (times.size() >= 4) cpuTimes.push_back(times);
    }
    statFile.close();

    if (cpuTimes.empty()) return info;

    // Total line
    auto& total = cpuTimes[0];
    long totalTicks = 0, idleTicks = total[3];
    for (auto t : total) totalTicks += t;

    if (hasPrev) {
        long deltaTotal = totalTicks - prevTotal.total;
        long deltaIdle = idleTicks - prevTotal.idle;
        if (deltaTotal > 0)
            info.totalUsage = 100.0 * (deltaTotal - deltaIdle) / deltaTotal;
    }

    // Per-core
    info.coreUsage.resize(cpuTimes.size() - 1);
    for (size_t i = 1; i < cpuTimes.size(); ++i) {
        auto& core = cpuTimes[i];
        long coreTotal = 0, coreIdle = core[3];
        for (auto t : core) coreTotal += t;

        if (hasPrev && i - 1 < prevCores.size()) {
            long dTotal = coreTotal - prevCores[i - 1].total;
            long dIdle = coreIdle - prevCores[i - 1].idle;
            if (dTotal > 0)
                info.coreUsage[i - 1] = 100.0 * (dTotal - dIdle) / dTotal;
        }
    }

    // Store for next delta
    prevTotal = {totalTicks, idleTicks};
    prevCores.clear();
    for (size_t i = 1; i < cpuTimes.size(); ++i) {
        long ct = 0;
        for (auto t : cpuTimes[i]) ct += t;
        prevCores.push_back({ct, cpuTimes[i][3]});
    }
    hasPrev = true;

    // Load average
    std::ifstream loadFile("/proc/loadavg");
    if (loadFile.is_open()) {
        loadFile >> info.loadAvg1 >> info.loadAvg5 >> info.loadAvg15;
        loadFile.close();
    }

    return info;
}
