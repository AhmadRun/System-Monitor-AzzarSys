#include "system_info.h"
#include <fstream>
#include <unistd.h>
#include <sys/utsname.h>
#include <sstream>

SystemInfo SystemInfoMonitor::getSystemInfo() {
    SystemInfo info;

    // Hostname
    char hostname[256] = {0};
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        info.hostname = hostname;
    }

    // Uptime
    std::ifstream uptimeFile("/proc/uptime");
    if (uptimeFile.is_open()) {
        double up = 0.0;
        uptimeFile >> up;
        info.uptime = static_cast<unsigned long>(up);
        info.bootTime = time(nullptr) - info.uptime;
        uptimeFile.close();
    }

    // Kernel and OS
    struct utsname uts;
    if (uname(&uts) == 0) {
        info.kernelVersion = uts.release;
        info.osVersion = uts.sysname;
    }

    // CPU cores
    info.cpuCoreCount = sysconf(_SC_NPROCESSORS_ONLN);

    return info;
}
