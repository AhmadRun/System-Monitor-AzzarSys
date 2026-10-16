#include "network_monitor.h"
#include <fstream>
#include <sstream>
#include <string>

std::vector<NetworkInterface> NetworkMonitor::getNetworkInfo() {
    std::vector<NetworkInterface> ifaces;

    std::ifstream netdev("/proc/net/dev");
    if (!netdev.is_open()) return ifaces;

    std::string line;
    // skip headers
    std::getline(netdev, line);
    std::getline(netdev, line);

    while (std::getline(netdev, line)) {
        // The name ends at ':' and the first counter can follow it with no space
        // (e.g. "eth0:123456 ..."), so split on the colon instead of whitespace.
        auto colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string name = line.substr(0, colon);
        name.erase(0, name.find_first_not_of(" \t"));
        if (name.empty()) continue;
        std::istringstream iss(line.substr(colon + 1));

        NetworkInterface ni;
        ni.name = name;
        // read stats: receive bytes, packets, errs, drop, fifo, frame, compressed, multicast
        unsigned long r_bytes, r_packets, r_errs, r_drop, r_fifo, r_frame, r_compressed, r_multicast;
        unsigned long t_bytes, t_packets, t_errs, t_drop, t_fifo, t_colls, t_carrier, t_compressed;

        if (iss >> r_bytes >> r_packets >> r_errs >> r_drop >> r_fifo >> r_frame >> r_compressed >> r_multicast
                >> t_bytes >> t_packets >> t_errs >> t_drop >> t_fifo >> t_colls >> t_carrier >> t_compressed) {
            ni.bytesReceived = r_bytes;
            ni.packetsReceived = r_packets;
            ni.errors = r_errs;
            ni.dropped = r_drop;

            ni.bytesSent = t_bytes;
            ni.packetsSent = t_packets;
            ifaces.push_back(ni);
        }
    }

    netdev.close();
    return ifaces;
}
