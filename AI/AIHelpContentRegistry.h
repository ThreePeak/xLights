#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace xLights::AI {

struct AIHelpOption {
    std::string name;
    std::string controlType;
    std::string description;
    std::string effect;
    std::string recommended;
    std::string warning;
};

struct AIHelpTopic {
    std::string topicId;
    std::string title;
    std::string category;
    std::string summary;
    std::vector<std::string> workflowSteps;
    std::vector<AIHelpOption> options;
    std::string diagram;
    std::vector<std::string> examples;
    std::vector<std::string> troubleshooting;

    std::string GenerateHtml() const;
};

class AIHelpContentRegistry {
public:
    static void Initialize();
    static const AIHelpTopic* GetTopic(const std::string& topicId);
    static std::vector<AIHelpTopic> GetAllTopics();
    static std::vector<std::string> GetCategories();
    static std::vector<AIHelpTopic> GetTopicsByCategory(const std::string& category);
    static std::vector<AIHelpTopic> Search(const std::string& query);

private:
    static void RegisterTopic(AIHelpTopic topic);
    static std::map<std::string, AIHelpTopic>& GetRegistry();
    static bool s_initialized;
};

} // namespace xLights::AI
