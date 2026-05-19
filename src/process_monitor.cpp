#include "process_monitor.h"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <pwd.h>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <climits>

unsigned long long ProcessMonitor::getTotalSystemTicks() {
    std::ifstream stat("/proc/stat");
    std::string line;
    if (!std::getline(stat, line)) return 0;
    std::istringstream iss(line);
    std::string cpu;
    iss >> cpu;
    unsigned long long total = 0, val;
    while (iss >> val) total += val;
    return total;
}

std::vector<ProcessInfo> ProcessMonitor::getTopProcesses(int count) {
    std::vector<ProcessInfo> processes;

    unsigned long long totalTicks = getTotalSystemTicks();
    if (totalTicks == 0) return processes;

    DIR* dir = opendir("/proc");
    if (!dir) return processes;

    std::unordered_map<int, ProcessTicks> currentTicks;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_DIR) continue;
        bool isNumber = true;
        for (char* p = entry->d_name; *p; p++) {
            if (!std::isdigit(*p)) {
                isNumber = false;
                break;
            }
        }
        if (!isNumber) continue;

        int pid = std::atoi(entry->d_name);
        ProcessInfo process = parseProcessInfo(pid, currentTicks);
        if (process.pid == 0) continue;

        // Compute delta CPU
        if (hasPrev) {
            auto it = prevTicks.find(pid);
            if (it != prevTicks.end()) {
                unsigned long long prevProc = it->second.utime + it->second.stime;
                unsigned long long curProc = currentTicks[pid].utime + currentTicks[pid].stime;
                unsigned long long dProc = curProc - prevProc;
                unsigned long long dSys = totalTicks - prevTotalTicks;
                process.cpuPercent = dSys > 0 ? 100.0 * dProc / dSys : 0.0;
            } else {
                process.cpuPercent = 0;
            }
        } else {
            process.cpuPercent = 0;
        }

        processes.push_back(process);
    }
    closedir(dir);

    // Sort by CPU descending
    std::sort(processes.begin(), processes.end(),
              [](const ProcessInfo& a, const ProcessInfo& b) {
                  return a.cpuPercent > b.cpuPercent;
              });

    // Update prev state for next call
    prevTicks = std::move(currentTicks);
    prevTotalTicks = totalTicks;
    hasPrev = true;

    if (processes.size() > static_cast<size_t>(count)) {
        processes.resize(count);
    }

    return processes;
}

ProcessInfo ProcessMonitor::parseProcessInfo(int pid, std::unordered_map<int, ProcessTicks>& ticks) {
    ProcessInfo process{};
    process.pid = 0;

    std::string statPath = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream stat(statPath);
    if (!stat.is_open()) return process;

    std::string line;
    std::getline(stat, line);
    stat.close();

    auto rparen = line.rfind(')');
    if (rparen == std::string::npos) return process;

    std::istringstream iss(line.substr(rparen + 2));
    char state;
    int ppid, pgrp, session, ttyNr, tpgid;
    unsigned int flags;
    unsigned long long minflt, cminflt, majflt, cmajflt, utime, stime;

    if (!(iss >> state >> ppid >> pgrp >> session >> ttyNr >> tpgid >> flags
               >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime)) {
        return process;
    }

    process.state = std::string(1, state);
    process.pid = pid;
    ticks[pid] = {utime, stime};

    // Get process name
    std::string commPath = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream comm(commPath);
    if (comm.is_open()) {
        std::getline(comm, process.name);
        comm.close();
    }

    // Get user and RSS
    std::string statusPath = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream status(statusPath);
    if (status.is_open()) {
        std::string sl;
        while (std::getline(status, sl)) {
            if (sl.find("Uid:") == 0) {
                std::istringstream uidStream(sl);
                std::string label;
                unsigned int uid;
                uidStream >> label >> uid;
                struct passwd* pwd = getpwuid(uid);
                if (pwd) process.user = pwd->pw_name;
            } else if (sl.find("VmRSS:") == 0) {
                std::istringstream rssStream(sl);
                std::string label;
                unsigned long rssKb;
                rssStream >> label >> rssKb;
                process.memoryMB = rssKb / 1024;
            }
        }
        status.close();
    }

    return process;
}
