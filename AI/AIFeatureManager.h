#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../src-core/ai/AISubsystemBase.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>

class ServiceManager;

namespace xLights::AI {

/**
 * @brief Central Registry & Manager for all 15 Pre-Production AI Features in xLights.
 * Encapsulated within namespace xLights::AI adhering to C++20 standards.
 */
class AIFeatureManager {
public:
    static AIFeatureManager& GetInstance();

    virtual ~AIFeatureManager() = default;

    AIFeatureManager(const AIFeatureManager&) = delete;
    AIFeatureManager& operator=(const AIFeatureManager&) = delete;
    AIFeatureManager(AIFeatureManager&&) = delete;
    AIFeatureManager& operator=(AIFeatureManager&&) = delete;

    /**
     * @brief Virtual lifecycle initialization method.
     */
    virtual bool Initialize();

    /**
     * @brief Virtual lifecycle shutdown method.
     */
    virtual void Shutdown();

    /**
     * @brief Checks if a specific feature ID (1..15) is enabled.
     */
    [[nodiscard]] virtual bool IsFeatureEnabled(int featureId) const;

    /**
     * @brief Enables or disables a feature by ID.
     */
    void SetFeatureEnabled(int featureId, bool enabled);

    /**
     * @brief Initializes all registered AI features with the given ServiceManager.
     */
    void InitializeAllFeatures(::ServiceManager* serviceManager);

    /**
     * @brief Registers an AI subsystem instance.
     */
    void RegisterSubsystem(const std::string& name, std::unique_ptr<AISubsystemBase> subsystem);

    /**
     * @brief Retrieves a pointer to a registered AI subsystem by name.
     */
    [[nodiscard]] AISubsystemBase* GetSubsystem(const std::string& name) const;

    /**
     * @brief Checks if a specific AI feature is registered and available.
     */
    [[nodiscard]] bool IsFeatureSupported(const std::string& featureName) const;

    /**
     * @brief Returns names of all registered AI features.
     */
    [[nodiscard]] std::vector<std::string> GetRegisteredFeatureNames() const;

    /**
     * @brief Requests cooperative cancellation across all active AI subsystems.
     */
    void CancelAll();

    /**
     * @brief Gracefully shuts down all AI subsystems and releases GPU/memory handles.
     */
    void ShutdownAll();

protected:
    AIFeatureManager() = default;

    mutable std::mutex m_managerMutex;
    std::unordered_map<std::string, std::unique_ptr<AISubsystemBase>> m_subsystems;
    std::unordered_set<int> m_enabledFeatures;
    ::ServiceManager* m_serviceManager{nullptr};
};

} // namespace xLights::AI
