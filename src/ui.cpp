#include "ui.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <csignal>
#include <cstdio>

#define CLEAR_LINE "\033[2K"
#define HOME       "\033[H"
#define CLR_EOS    "\033[J"
#define CURS_OFF   "\033[?25l"
#define CURS_ON    "\033[?25h"

static void printLine(const std::string& s) {
    std::cout << CLEAR_LINE << s << "\n";
}

template <typename... Args>
static void printLineF(const char* fmt, Args... args) {
    char buf[512];
    int n = snprintf(buf, sizeof(buf), fmt, args...);
    if (n < 0) return;
    std::cout << CLEAR_LINE << buf << "\n";
}

// Builds "[####......]". The percentage is clamped so a bad reading can never
// draw past the bar.
static std::string makeBar(double percent, int width) {
    if (!(percent > 0.0)) percent = 0.0;
    if (percent > 100.0) percent = 100.0;
    int used = static_cast<int>(percent / 100.0 * width);
    return "[" + std::string(used, '#') + std::string(width - used, '.') + "]";
}

static volatile std::sig_atomic_t stopRequested = 0;

static void handleStopSignal(int) {
    stopRequested = 1;
}

SystemMonitorUI::SystemMonitorUI() {
    cpuMonitor = std::make_unique<CPUMonitor>();
    memoryMonitor = std::make_unique<MemoryMonitor>();
    diskMonitor = std::make_unique<DiskMonitor>();
    processMonitor = std::make_unique<ProcessMonitor>();
    networkMonitor = std::make_unique<NetworkMonitor>();
    systemInfoMonitor = std::make_unique<SystemInfoMonitor>();

    // Ctrl+C / kill would otherwise exit without restoring the terminal,
    // leaving echo off and the cursor hidden.
    struct sigaction sa {};
    sa.sa_handler = handleStopSignal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGHUP, &sa, nullptr);

    enableRawMode();
}

SystemMonitorUI::~SystemMonitorUI() {
    cleanup();
}

void SystemMonitorUI::enableRawMode() {
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) == 0) {
        savedTermFlags = term.c_lflag;
        term.c_lflag &= ~(ECHO | ICANON);
        term.c_cc[VMIN] = 0;
        term.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
        rawMode = true;
    }
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void SystemMonitorUI::disableRawMode() {
    if (rawMode) {
        struct termios term;
        if (tcgetattr(STDIN_FILENO, &term) == 0) {
            term.c_lflag = savedTermFlags;
            tcsetattr(STDIN_FILENO, TCSANOW, &term);
        }
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
        rawMode = false;
    }
}

void SystemMonitorUI::cleanup() {
    if (cleanedUp) return;
    cleanedUp = true;
    disableRawMode();
    std::cout << CURS_ON << "\033[0m\n" << std::flush;
}

int SystemMonitorUI::getchNonBlock() {
    char c;
    if (read(STDIN_FILENO, &c, 1) > 0) return c;
    return -1;
}

void SystemMonitorUI::run() {
    while (running && !stopRequested) {
        drawAll();
        // Poll for input in short steps so 'q' and Ctrl+C respond quickly.
        for (int i = 0; i < 10 && running && !stopRequested; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int ch;
            while ((ch = getchNonBlock()) != -1) {
                if (ch == 'q' || ch == 'Q') running = false;
            }
        }
    }
    cleanup();
}

void SystemMonitorUI::drawAll() {
    std::cout << CURS_OFF << HOME;

    drawHeader();
    drawSystemInfo();
    drawCPUInfo();
    drawMemoryInfo();
    drawDiskInfo();
    drawNetworkInfo();
    drawProcessesInfo();
    drawFooter();

    std::cout << CLR_EOS << std::flush;
}

void SystemMonitorUI::drawHeader() {
    printLine("=== System Monitor ===");
}

void SystemMonitorUI::drawSystemInfo() {
    SystemInfo info = systemInfoMonitor->getSystemInfo();
    unsigned long days = info.uptime / 86400;
    unsigned long hours = (info.uptime % 86400) / 3600;
    unsigned long mins = (info.uptime % 3600) / 60;

    printLineF("Host: %s  |  Uptime: %lud %02luh %02lum  |  Cores: %d",
               info.hostname.c_str(), days, hours, mins, info.cpuCoreCount);
}

void SystemMonitorUI::drawCPUInfo() {
    CPUInfo info = cpuMonitor->getCPUInfo();

    printLineF("CPU Total: %6.2f%%  %s  Load: %.2f %.2f %.2f",
               info.totalUsage, makeBar(info.totalUsage, 20).c_str(),
               info.loadAvg1, info.loadAvg5, info.loadAvg15);

    printLine("Per-Core:");
    for (size_t i = 0; i < info.coreUsage.size() && i < 16; ++i) {
        printLineF("  Core %2zu: %6.2f%%  %s", i, info.coreUsage[i],
                   makeBar(info.coreUsage[i], 10).c_str());
    }
}

void SystemMonitorUI::drawMemoryInfo() {
    MemoryInfo m = memoryMonitor->getMemoryInfo();
    printLineF("Memory: %5.1f%%  %s  %lu / %lu MB",
               m.memoryPercent, makeBar(m.memoryPercent, 20).c_str(),
               m.usedMemory / (1024 * 1024), m.totalMemory / (1024 * 1024));
}

void SystemMonitorUI::drawDiskInfo() {
    auto disks = diskMonitor->getDiskInfo();
    printLine("Disks:");
    for (const auto& d : disks) {
        printLineF("  %-15s %6.2f%% %s  %lu / %lu MB",
                   d.mountPoint.c_str(), d.usedPercent, makeBar(d.usedPercent, 10).c_str(),
                   d.usedSize / (1024 * 1024), d.totalSize / (1024 * 1024));
    }
}

void SystemMonitorUI::drawNetworkInfo() {
    auto ifs = networkMonitor->getNetworkInfo();
    printLine("Network:");
    for (const auto& n : ifs) {
        double rx = n.bytesReceived, tx = n.bytesSent;
        const char* units[] = {"B", "KB", "MB", "GB"};
        int rxu = 0, txu = 0;
        while (rx >= 1024 && rxu < 3) { rx /= 1024; rxu++; }
        while (tx >= 1024 && txu < 3) { tx /= 1024; txu++; }
        printLineF("  %-8s RX: %7.2f %-2s  TX: %7.2f %-2s",
                   n.name.c_str(), rx, units[rxu], tx, units[txu]);
    }
}

void SystemMonitorUI::drawProcessesInfo() {
    auto procs = processMonitor->getTopProcesses(10);
    printLine("Top Processes (by CPU):");
    printLineF("  %-5s  %-20s %6s  %5s", "PID", "NAME", "CPU%", "MEM");
    for (const auto& p : procs) {
        printLineF("  %-5d  %-20.20s %6.2f  %5lu MB",
                   p.pid, p.name.c_str(), p.cpuPercent, p.memoryMB);
    }
}

void SystemMonitorUI::drawFooter() {
    printLine("");
    std::cout << CLEAR_LINE << "Press 'q' to quit" << std::flush;
}
