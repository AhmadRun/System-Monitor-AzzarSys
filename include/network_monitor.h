#pragma once

#include <string>
#include <vector>

struct NetworkInterface {
    std::string name;
    unsigned long bytesSent;
    unsigned long bytesReceived;
    unsigned long packetsSent;
    unsigned long packetsReceived;
    unsigned long errors;
    unsigned long dropped;
};

class NetworkMonitor {
public:
    std::vector<NetworkInterface> getNetworkInfo();
    
private:
    std::vector<NetworkInterface> previousReading;
};
