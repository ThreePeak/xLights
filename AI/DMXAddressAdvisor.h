#pragma once
#include <string>
#include <vector>
#include <mutex>

namespace xLights::AI {

struct DMXAddressConflict {
    int channel;
    std::string fixtureA;
    std::string fixtureB;
    std::string resolution;
};

class DMXAddressAdvisor {
public:
    std::vector<DMXAddressConflict> DetectConflicts(const std::string& universeXml);
    std::string SuggestRemapping(const std::vector<DMXAddressConflict>& conflicts);
private:
    mutable std::mutex m_mutex;
};

} // namespace xLights::AI
