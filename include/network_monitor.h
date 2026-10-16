#pragma once

#include <string>
#include <vector>

struct NetworkInterface {
    std::string name;
    unsigned long bytesSent = 0;
    unsigned long bytesReceived = 0;
    unsigned long packetsSent = 0;
    unsigned long packetsReceived = 0;
    unsigned long errors = 0;
    unsigned long dropped = 0;
};

class NetworkMonitor {
public:
    std::vector<NetworkInterface> getNetworkInfo();
    
private:
    std::vector<NetworkInterface> previousReading;
};
