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
        iss >> device >> mountpoint >> fstype;

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
        d.usedPercent = total > 0 ? 100.0 * used / total : 0.0;

        if (total == 0) continue;

        disks.push_back(d);
    }

    return disks;
}
