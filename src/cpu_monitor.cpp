#include "cpu_monitor.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

// /proc/stat columns: user nice system idle iowait irq softirq steal guest guest_nice.
// guest and guest_nice are already counted in user and nice, so they are left out
// of the total, and iowait counts as idle time (the same as top and htop).
static void sumTimes(const std::vector<long>& t, long& total, long& idle) {
    total = 0;
    for (size_t i = 0; i < t.size() && i < 8; ++i) total += t[i];
    idle = t[3] + (t.size() > 4 ? t[4] : 0);
}

static double usagePercent(long dTotal, long dIdle) {
    if (dTotal <= 0) return 0.0;
    double pct = 100.0 * (dTotal - dIdle) / dTotal;
    if (pct < 0.0) return 0.0;
    if (pct > 100.0) return 100.0;
    return pct;
}

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
    long totalTicks, idleTicks;
    sumTimes(total, totalTicks, idleTicks);

    if (hasPrev) {
        info.totalUsage = usagePercent(totalTicks - prevTotal.total,
                                       idleTicks - prevTotal.idle);
    }

    // Per-core
    info.coreUsage.resize(cpuTimes.size() - 1);
    for (size_t i = 1; i < cpuTimes.size(); ++i) {
        auto& core = cpuTimes[i];
        long coreTotal, coreIdle;
        sumTimes(core, coreTotal, coreIdle);

        if (hasPrev && i - 1 < prevCores.size()) {
            info.coreUsage[i - 1] = usagePercent(coreTotal - prevCores[i - 1].total,
                                                 coreIdle - prevCores[i - 1].idle);
        }
    }

    // Store for next delta
    prevTotal = {totalTicks, idleTicks};
    prevCores.clear();
    for (size_t i = 1; i < cpuTimes.size(); ++i) {
        long ct, ci;
        sumTimes(cpuTimes[i], ct, ci);
        prevCores.push_back({ct, ci});
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
