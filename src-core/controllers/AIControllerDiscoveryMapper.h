/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "src-core/ai/AICommandHistory.h"

namespace xLights::AI {

enum class ControllerVendorType {
    ESPIXELSTICK_V4,
    WLED,
    FALCON_FPP,
    KULP_LIGHTS,
    GENERIC_DDP_E131
};

struct DiscoveredControllerDevice {
    std::string ipAddress;
    std::string hostname;
    std::string macAddress;
    ControllerVendorType vendor{ControllerVendorType::ESPIXELSTICK_V4};
    std::string firmwareVersion{"v4.0-ci"};
    int totalPorts{4};
    int maxPixelsPerPort{800};
    std::string supportedProtocols{"WS2811, WS2812B, SK6812"};
    int wifiRssiDbm{-58};
    bool isOnline{true};
};

struct ModelControllerBindingProposal {
    std::string modelName;
    std::string targetControllerIp;
    std::string targetControllerHost;
    int targetPortIndex{1};
    uint32_t startUniverse{1};
    uint32_t startChannel{1};
    uint32_t endChannel{1500};
    uint32_t pixelCount{500};
    uint32_t channelCount{1500};
    std::string colorOrder{"RGB"};
    bool isCheckedForApplication{true};
    std::string rationale;
};

struct ControllerDiscoveryReport {
    std::string subnetScanned{"192.168.1.0/24"};
    int controllersDiscovered{0};
    int modelsMatched{0};
    int totalChannelsMapped{0};
    std::vector<DiscoveredControllerDevice> devices;
    std::vector<ModelControllerBindingProposal> proposals;

    std::string GenerateFormattedReport() const;
};

class AIControllerDiscoveryMapper {
public:
    AIControllerDiscoveryMapper() = default;
    ~AIControllerDiscoveryMapper() = default;

    /// Strictly On-Demand Subnet Scan (No automatic background polling)
    static std::vector<DiscoveredControllerDevice> ScanSubnetOnDemand(const std::string& subnetRange = "192.168.1.0/24");

    /// Generates intelligent layout-to-port mapping proposals
    static ControllerDiscoveryReport GenerateMappingProposals(
        const std::vector<DiscoveredControllerDevice>& devices,
        const std::vector<std::string>& layoutModels);

    static std::string GetVendorName(ControllerVendorType vendor);
};

} // namespace xLights::AI
