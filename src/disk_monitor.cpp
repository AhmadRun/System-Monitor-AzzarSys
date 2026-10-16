#include "disk_monitor.h"
#include <sys/statvfs.h>
#include <fstream>
#include <sstream>
#include <set>
#include <unistd.h>
#include <algorithm>

static bool isPhysicalFilesystem(const std::string& fstype) {
    static const std::set<std::string> pseudo = {
        "tmpfs", "sysfs", "proc", "cgroup", "cgroup2", "pstore",
        "devtmpfs", "devpts", "securityfs", "bpf", "configfs",
        "selinuxfs", "binfmt_misc", "tracefs", "hugetlbfs",
        "fuse.gvfsd-fuse", "fuse.portal", "fuse.lxcfs",
        "debugfs", "mqueue", "efivarfs", "autofs", "overlay",
        "fuse", "fuseblk", "fusectl", "squashfs", "rpc_pipefs"
    };
    return pseudo.find(fstype) == pseudo.end();
}

// /proc/mounts escapes spaces, tabs, newlines and backslashes as octal (\040 etc.).
static std::string unescapeMountField(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 3 < s.size() &&
            s[i + 1] >= '0' && s[i + 1] <= '7' &&
            s[i + 2] >= '0' && s[i + 2] <= '7' &&
            s[i + 3] >= '0' && s[i + 3] <= '7') {
            out += static_cast<char>((s[i + 1] - '0') * 64 + (s[i + 2] - '0') * 8 + (s[i + 3] - '0'));
            i += 3;
        } else {
            out += s[i];
        }
    }
    return out;
}

std::vector<DiskInfo> DiskMonitor::getDiskInfo() {
    std::vector<DiskInfo> disks;
    std::set<std::string> seen;

    std::ifstream mounts("/proc/mounts");
    if (!mounts.is_open()) {
        mounts.open("/etc/mtab");
    }

    std::string line;
    while (std::getline(mounts, line)) {
        std::istringstream iss(line);
        std::string device, mountpoint, fstype;
        if (!(iss >> device >> mountpoint >> fstype)) continue;
        mountpoint = unescapeMountField(mountpoint);

        if (!isPhysicalFilesystem(fstype)) continue;
        if (seen.count(mountpoint)) continue;
        seen.insert(mountpoint);

        // Skip virtual/non-device mounts
        if (device.find("/dev/") != 0 && device != "none") continue;

        struct statvfs stat;
        if (statvfs(mountpoint.c_str(), &stat) != 0) continue;

        unsigned long long total = static_cast<unsigned long long>(stat.f_blocks) * stat.f_frsize;
        unsigned long long available = static_cast<unsigned long long>(stat.f_bavail) * stat.f_frsize;
        unsigned long long freeBlocks = static_cast<unsigned long long>(stat.f_bfree) * stat.f_frsize;
        unsigned long long used = total - freeBlocks;

        DiskInfo d;
        d.mountPoint = mountpoint;
        d.filesystem = device == "none" ? fstype : device;
        d.totalSize = total;
        d.usedSize = used;
        d.availableSize = available;
        // Same formula as df: blocks reserved for root count as neither used nor available.
        unsigned long long usable = used + available;
        d.usedPercent = usable > 0 ? 100.0 * used / usable : 0.0;

        if (total == 0) continue;

        disks.push_back(d);
    }

    return disks;
}
