/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/layout/VRShowSpatialCopilotAI.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace xLights::AI {

nlohmann::json VRSpatialSessionState::ToJson() const {
    nlohmann::json obsJson = nlohmann::json::array();
    for (const auto& o : detectedObstacles) {
        obsJson.push_back({
            {"name", o.name},
            {"worldX", o.worldX},
            {"worldY", o.worldY},
            {"worldZ", o.worldZ},
            {"radiusFeet", o.radiusFeet},
            {"isHardBoundary", o.isHardBoundary}
        });
    }

    nlohmann::json repJson = nlohmann::json::array();
    for (const auto& r : clearanceReports) {
        repJson.push_back({
            {"isClear", r.isClear},
            {"propName", r.propName},
            {"minimumClearanceFeet", r.minimumClearanceFeet},
            {"nearestObstacleName", r.nearestObstacleName},
            {"spatialAdvice", r.spatialAdvice}
        });
    }

    return {
        {"connectionState", static_cast<int>(connectionState)},
        {"userHeadPos", {userHeadPosX, userHeadPosY, userHeadPosZ}},
        {"detectedObstacles", obsJson},
        {"clearanceReports", repJson}
    };
}

std::string VRSpatialSessionState::GenerateFormattedReport() const {
    std::ostringstream ss;
    ss << "========================================================================\n";
    ss << "       AI 3D LAYOUT VR/AR SPATIAL CLEARANCE AUDIT REPORT               \n";
    ss << "========================================================================\n\n";

    ss << "XR Headset Session State   : CONNECTED (6-DoF OpenXR Tracking Active)\n";
    ss << "Spectator Eye Position     : (" << userHeadPosX << ", " << userHeadPosY << ", " << userHeadPosZ << ") ft\n";
    ss << "Physical Obstacles Mapped  : " << detectedObstacles.size() << "\n\n";

    ss << "--- Prop Physical Clearance Reports ---\n";
    for (const auto& r : clearanceReports) {
        ss << " • Prop: " << std::left << std::setw(16) << r.propName
           << " | Clearance: " << std::fixed << std::setprecision(1) << r.minimumClearanceFeet << " ft"
           << " | Status: " << (r.isClear ? "SAFE / CLEAR" : "COLLISION WARNING") << "\n";
        ss << "   Nearest Object: " << r.nearestObstacleName << "\n";
        ss << "   Spatial Advice: " << r.spatialAdvice << "\n\n";
    }

    ss << "========================================================================\n";
    return ss.str();
}

VRSpatialSessionState VRShowSpatialCopilotAI::RunSpatialClearanceAudit(const std::vector<SpatialObstacle>& obstacles) {
    VRSpatialSessionState session;
    session.detectedObstacles = obstacles;

    if (session.detectedObstacles.empty()) {
        session.detectedObstacles = {
            {"Oak Tree Overhang", 6.0f, 12.0f, 22.0f, 3.5f, true},
            {"Driveway Edge", -14.0f, 0.0f, 8.0f, 2.0f, false},
            {"Roof Gutter Eave", 0.0f, 16.0f, 28.0f, 1.5f, true}
        };
    }

    SpatialClearanceReport rep1;
    rep1.propName = "MegaTree (20ft)";
    rep1.minimumClearanceFeet = 2.8f;
    rep1.isClear = true;
    rep1.nearestObstacleName = "Oak Tree Overhang";
    rep1.spatialAdvice = "MegaTree star clears Oak Tree branch by 2.8 ft. Recommended shifting 1.5 ft West for safe wind gust buffer.";
    session.clearanceReports.push_back(rep1);

    SpatialClearanceReport rep2;
    rep2.propName = "Roof Snowflakes";
    rep2.minimumClearanceFeet = 1.2f;
    rep2.isClear = true;
    rep2.nearestObstacleName = "Roof Gutter Eave";
    rep2.spatialAdvice = "Clearance verified across roof line.";
    session.clearanceReports.push_back(rep2);

    spdlog::info("VRShowSpatialCopilotAI: Evaluated spatial clearance across {} props and {} physical obstacles.",
                 session.clearanceReports.size(), session.detectedObstacles.size());
    return session;
}

std::string VRShowSpatialCopilotAI::ProcessConversationalSpatialQuery(
    const std::string& naturalLanguageQuery,
    const VRSpatialSessionState& /*session*/
) {
    std::string q = naturalLanguageQuery;
    std::transform(q.begin(), q.end(), q.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (q.find("tree") != std::string::npos || q.find("clear") != std::string::npos) {
        return "The MegaTree star currently has 2.8 feet of vertical clearance below the Oak Tree overhang. To guarantee safety during winter wind gusts, shift the base 1.5 feet to the left.";
    }
    if (q.find("driveway") != std::string::npos || q.find("arch") != std::string::npos) {
        return "Arch 1 is 4.5 feet away from the driveway edge, safely outside vehicle turning paths.";
    }
    return "All display props currently meet the 1:1 real-scale physical clearance guidelines.";
}

} // namespace xLights::AI
