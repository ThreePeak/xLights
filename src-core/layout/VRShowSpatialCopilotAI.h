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
#include <memory>
#include <nlohmann/json.hpp>

namespace xLights::AI {

enum class HeadsetConnectionState {
    DISCONNECTED = 0,
    CONNECTED_OPENXR = 1,
    CONNECTED_META_QUEST = 2,
    CONNECTED_APPLE_VISION_PRO = 3,
    CONNECTED_MOBILE_AR = 4
};

struct SpatialObstacle {
    std::string name{"Oak Tree Overhang"};
    float worldX{12.0f};  ///< Feet relative to yard center
    float worldY{8.0f};   ///< Feet height
    float worldZ{15.0f};  ///< Feet depth
    float radiusFeet{4.0f};
    bool isHardBoundary{true};
};

struct SpatialClearanceReport {
    bool isClear{true};
    std::string propName{"MegaTree"};
    float minimumClearanceFeet{3.2f};
    std::string nearestObstacleName;
    std::string spatialAdvice;
};

struct VRSpatialSessionState {
    HeadsetConnectionState connectionState{HeadsetConnectionState::CONNECTED_OPENXR};
    float userHeadPosX{0.0f};
    float userHeadPosY{5.5f};
    float userHeadPosZ{30.0f};
    std::vector<SpatialObstacle> detectedObstacles;
    std::vector<SpatialClearanceReport> clearanceReports;

    [[nodiscard]] nlohmann::json ToJson() const;
    [[nodiscard]] std::string GenerateFormattedReport() const;
};

class VRShowSpatialCopilotAI {
public:
    static VRSpatialSessionState RunSpatialClearanceAudit(
        const std::vector<SpatialObstacle>& obstacles = {}
    );

    static std::string ProcessConversationalSpatialQuery(
        const std::string& naturalLanguageQuery,
        const VRSpatialSessionState& session
    );
};

} // namespace xLights::AI
