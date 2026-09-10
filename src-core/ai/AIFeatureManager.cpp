/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AIFeatureManager.h"
#include "ServiceManager.h"
#include "spdlog/spdlog.h"

namespace xLights::AI {

AIFeatureManager& AIFeatureManager::GetInstance() {
    static AIFeatureManager instance;
    return instance;
}

bool AIFeatureManager::Initialize() {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    spdlog::info("[AIFeatureManager] Virtual Initialize called — enabling default AI feature suite (Features 1..15).");
    for (int i = 1; i <= 15; ++i) {
        m_enabledFeatures.insert(i);
    }
    return true;
}

void AIFeatureManager::Shutdown() {
    ShutdownAll();
}

bool AIFeatureManager::IsFeatureEnabled(int featureId) const {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    return m_enabledFeatures.find(featureId) != m_enabledFeatures.end();
}

void AIFeatureManager::SetFeatureEnabled(int featureId, bool enabled) {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    if (enabled) {
        m_enabledFeatures.insert(featureId);
    } else {
        m_enabledFeatures.erase(featureId);
    }
}

void AIFeatureManager::InitializeAllFeatures(::ServiceManager* serviceManager) {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    m_serviceManager = serviceManager;

    spdlog::info("[AIFeatureManager] Initializing xLights AI Subsystem suite...");
    std::vector<std::future<bool>> initFutures;
    initFutures.reserve(m_subsystems.size());
    for (auto& [name, subsystem] : m_subsystems) {
        if (subsystem) {
            spdlog::info("[AIFeatureManager] Initializing feature: {}", name);
            initFutures.push_back(subsystem->InitializeAsync());
        }
    }
    for (auto& fut : initFutures) {
        if (fut.valid()) {
            fut.wait();
        }
    }
    spdlog::info("[AIFeatureManager] All AI Subsystems initialized.");
}

void AIFeatureManager::RegisterSubsystem(const std::string& name, std::unique_ptr<AISubsystemBase> subsystem) {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    if (subsystem) {
        spdlog::info("[AIFeatureManager] Registered AI subsystem: {}", name);
        m_subsystems[name] = std::move(subsystem);
    }
}

AISubsystemBase* AIFeatureManager::GetSubsystem(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    auto it = m_subsystems.find(name);
    if (it != m_subsystems.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool AIFeatureManager::IsFeatureSupported(const std::string& featureName) const {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    auto it = m_subsystems.find(featureName);
    return (it != m_subsystems.end() && it->second != nullptr);
}

std::vector<std::string> AIFeatureManager::GetRegisteredFeatureNames() const {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    std::vector<std::string> names;
    names.reserve(m_subsystems.size());
    for (const auto& [name, subsystem] : m_subsystems) {
        names.push_back(name);
    }
    return names;
}

void AIFeatureManager::CancelAll() {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    spdlog::info("[AIFeatureManager] Requesting cooperative cancellation across all AI subsystems...");
    for (auto& [name, subsystem] : m_subsystems) {
        if (subsystem) {
            subsystem->RequestCancel();
        }
    }
}

void AIFeatureManager::ShutdownAll() {
    std::lock_guard<std::mutex> lock(m_managerMutex);
    spdlog::info("[AIFeatureManager] Shutting down all AI subsystems...");
    for (auto& [name, subsystem] : m_subsystems) {
        if (subsystem) {
            subsystem->Shutdown();
        }
    }
    m_subsystems.clear();
    m_enabledFeatures.clear();
}

} // namespace xLights::AI
