#pragma once

#include "cpu_monitor.h"
#include "memory_monitor.h"
#include "disk_monitor.h"
#include "process_monitor.h"
#include "network_monitor.h"
#include "system_info.h"
#include <memory>

class SystemMonitorUI {
public:
    SystemMonitorUI();
    ~SystemMonitorUI();

    void run();
    void cleanup();

private:
    std::unique_ptr<CPUMonitor> cpuMonitor;
    std::unique_ptr<MemoryMonitor> memoryMonitor;
    std::unique_ptr<DiskMonitor> diskMonitor;
    std::unique_ptr<ProcessMonitor> processMonitor;
    std::unique_ptr<NetworkMonitor> networkMonitor;
    std::unique_ptr<SystemInfoMonitor> systemInfoMonitor;

    void drawAll();
    void drawHeader();
    void drawSystemInfo();
    void drawCPUInfo();
    void drawMemoryInfo();
    void drawDiskInfo();
    void drawNetworkInfo();
    void drawProcessesInfo();
    void drawFooter();

    void enableRawMode();
    void disableRawMode();
    int getchNonBlock();

    int savedTermFlags = 0;
    bool rawMode = false;
    bool running = true;
};
