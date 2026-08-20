/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/ai/AIEngineConfigManager.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running AIEngineConfigManager verification..." << std::endl;

    auto& mgr = AIEngineConfigManager::Instance();

    // Test 1: Default Profiles
    {
        auto names = mgr.GetAvailableProfileNames();
        assert(names.size() >= 4);
        auto active = mgr.GetActiveProfile();
        assert(!active.modelName.empty());
        std::cout << " -> Test 1 (Default Profiles Initialization): PASSED (Active: " << active.profileName << ")" << std::endl;
    }

    // Test 2: Profile Switching & Custom Profile Creation
    {
        AIProviderProfile custom;
        custom.profileName = "Custom Local vLLM";
        custom.providerType = AIProviderType::LOCAL_VLLM;
        custom.modelName = "mistral-large-2407";
        custom.customEndpointUrl = "http://192.168.1.100:8000/v1";
        custom.temperature = 0.4f;

        mgr.SaveProfile(custom);
        mgr.SetActiveProfile("Custom Local vLLM");

        auto active = mgr.GetActiveProfile();
        assert(active.profileName == "Custom Local vLLM");
        assert(active.modelName == "mistral-large-2407");
        assert(active.customEndpointUrl == "http://192.168.1.100:8000/v1");
        std::cout << " -> Test 2 (Custom Profile Creation & Switching): PASSED" << std::endl;
    }

    // Test 3: Token Usage Tracking & Offline Mode Toggle
    {
        mgr.SetOfflineMode(true);
        assert(mgr.IsOfflineMode());
        mgr.SetOfflineMode(false);
        assert(!mgr.IsOfflineMode());

        uint64_t before = mgr.GetCurrentTokenUsage();
        mgr.RecordTokenUsage(1500);
        assert(mgr.GetCurrentTokenUsage() == before + 1500);
        std::cout << " -> Test 3 (Token Usage Tracking & Offline Toggle): PASSED" << std::endl;
    }

    // Test 4: JSON Serialization & Round-Trip
    {
        auto jsonDump = mgr.ToJson();
        assert(jsonDump.contains("activeProfileName"));
        assert(jsonDump.contains("profiles"));

        mgr.FromJson(jsonDump);
        assert(mgr.GetActiveProfile().profileName == "Custom Local vLLM");
        std::cout << " -> Test 4 (JSON Serialization & Round-Trip): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] AIEngineConfigManager ALL TESTS PASSED!" << std::endl;
    return 0;
}
