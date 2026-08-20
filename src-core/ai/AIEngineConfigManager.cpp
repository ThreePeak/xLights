/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/ai/AIEngineConfigManager.h"
#include <fstream>
#include <spdlog/spdlog.h>

namespace xLights::AI {

nlohmann::json AIProviderProfile::ToJson() const {
    return {
        {"profileName", profileName},
        {"providerType", static_cast<int>(providerType)},
        {"modelName", modelName},
        {"apiKey", apiKey},
        {"customEndpointUrl", customEndpointUrl},
        {"temperature", temperature},
        {"topP", topP},
        {"maxTokens", maxTokens},
        {"timeoutSeconds", timeoutSeconds},
        {"enableStreaming", enableStreaming},
        {"isLocalOffline", isLocalOffline}
    };
}

AIProviderProfile AIProviderProfile::FromJson(const nlohmann::json& j) {
    AIProviderProfile p;
    if (j.contains("profileName")) p.profileName = j["profileName"].get<std::string>();
    if (j.contains("providerType")) p.providerType = static_cast<AIProviderType>(j["providerType"].get<int>());
    if (j.contains("modelName")) p.modelName = j["modelName"].get<std::string>();
    if (j.contains("apiKey")) p.apiKey = j["apiKey"].get<std::string>();
    if (j.contains("customEndpointUrl")) p.customEndpointUrl = j["customEndpointUrl"].get<std::string>();
    if (j.contains("temperature")) p.temperature = j["temperature"].get<float>();
    if (j.contains("topP")) p.topP = j["topP"].get<float>();
    if (j.contains("maxTokens")) p.maxTokens = j["maxTokens"].get<int>();
    if (j.contains("timeoutSeconds")) p.timeoutSeconds = j["timeoutSeconds"].get<int>();
    if (j.contains("enableStreaming")) p.enableStreaming = j["enableStreaming"].get<bool>();
    if (j.contains("isLocalOffline")) p.isLocalOffline = j["isLocalOffline"].get<bool>();
    return p;
}

AIEngineConfigManager& AIEngineConfigManager::Instance() {
    static AIEngineConfigManager s_instance;
    return s_instance;
}

AIEngineConfigManager::AIEngineConfigManager() {
    // Populate standard out-of-the-box profiles
    AIProviderProfile openaiDefault;
    openaiDefault.profileName = "Cloud High Quality (OpenAI GPT-4o)";
    openaiDefault.providerType = AIProviderType::OPENAI;
    openaiDefault.modelName = "gpt-4o";
    openaiDefault.customEndpointUrl = "https://api.openai.com/v1";
    openaiDefault.temperature = 0.7f;
    openaiDefault.maxTokens = 4096;

    AIProviderProfile claudeProfile;
    claudeProfile.profileName = "Anthropic Claude 3.5 Sonnet";
    claudeProfile.providerType = AIProviderType::ANTHROPIC;
    claudeProfile.modelName = "claude-3-5-sonnet-20241022";
    claudeProfile.customEndpointUrl = "https://api.anthropic.com/v1";
    claudeProfile.temperature = 0.6f;

    AIProviderProfile geminiProfile;
    geminiProfile.profileName = "Google Gemini 1.5 Pro";
    geminiProfile.providerType = AIProviderType::GOOGLE_GEMINI;
    geminiProfile.modelName = "gemini-1.5-pro-latest";
    geminiProfile.customEndpointUrl = "https://generativelanguage.googleapis.com/v1beta";

    AIProviderProfile localOllama;
    localOllama.profileName = "Local Offline LLM (Ollama - Llama 3.1 8B)";
    localOllama.providerType = AIProviderType::LOCAL_OLLAMA;
    localOllama.modelName = "llama3.1:8b";
    localOllama.customEndpointUrl = "http://localhost:11434/v1";
    localOllama.isLocalOffline = true;

    m_profiles[openaiDefault.profileName] = openaiDefault;
    m_profiles[claudeProfile.profileName] = claudeProfile;
    m_profiles[geminiProfile.profileName] = geminiProfile;
    m_profiles[localOllama.profileName] = localOllama;
    m_activeProfileName = openaiDefault.profileName;
}

void AIEngineConfigManager::SetActiveProfile(const std::string& profileName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_profiles.find(profileName) != m_profiles.end()) {
        m_activeProfileName = profileName;
        spdlog::info("AIEngineConfigManager: Set active profile to '{}'", profileName);
    }
}

AIProviderProfile AIEngineConfigManager::GetActiveProfile() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_profiles.find(m_activeProfileName);
    if (it != m_profiles.end()) {
        return it->second;
    }
    return m_profiles.begin()->second;
}

void AIEngineConfigManager::SaveProfile(const AIProviderProfile& profile) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_profiles[profile.profileName] = profile;
    spdlog::info("AIEngineConfigManager: Saved profile '{}' (Model: {})", profile.profileName, profile.modelName);
}

void AIEngineConfigManager::DeleteProfile(const std::string& profileName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_profiles.erase(profileName);
    if (m_activeProfileName == profileName && !m_profiles.empty()) {
        m_activeProfileName = m_profiles.begin()->first;
    }
}

std::vector<std::string> AIEngineConfigManager::GetAvailableProfileNames() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> names;
    names.reserve(m_profiles.size());
    for (const auto& [name, _] : m_profiles) {
        names.push_back(name);
    }
    return names;
}

void AIEngineConfigManager::SetOfflineMode(bool offlineOnly) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_offlineOnlyMode = offlineOnly;
}

bool AIEngineConfigManager::IsOfflineMode() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_offlineOnlyMode;
}

void AIEngineConfigManager::SetMonthlyTokenBudget(uint64_t maxTokens) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_monthlyTokenBudget = maxTokens;
}

uint64_t AIEngineConfigManager::GetMonthlyTokenBudget() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_monthlyTokenBudget;
}

uint64_t AIEngineConfigManager::GetCurrentTokenUsage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentTokenUsage;
}

void AIEngineConfigManager::RecordTokenUsage(uint64_t tokensUsed) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentTokenUsage += tokensUsed;
}

nlohmann::json AIEngineConfigManager::ToJson() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    nlohmann::json profilesJson = nlohmann::json::object();
    for (const auto& [name, p] : m_profiles) {
        profilesJson[name] = p.ToJson();
    }

    return {
        {"activeProfileName", m_activeProfileName},
        {"offlineOnlyMode", m_offlineOnlyMode},
        {"monthlyTokenBudget", m_monthlyTokenBudget},
        {"currentTokenUsage", m_currentTokenUsage},
        {"profiles", profilesJson}
    };
}

void AIEngineConfigManager::FromJson(const nlohmann::json& j) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (j.contains("activeProfileName")) m_activeProfileName = j["activeProfileName"].get<std::string>();
    if (j.contains("offlineOnlyMode")) m_offlineOnlyMode = j["offlineOnlyMode"].get<bool>();
    if (j.contains("monthlyTokenBudget")) m_monthlyTokenBudget = j["monthlyTokenBudget"].get<uint64_t>();
    if (j.contains("currentTokenUsage")) m_currentTokenUsage = j["currentTokenUsage"].get<uint64_t>();

    if (j.contains("profiles") && j["profiles"].is_object()) {
        m_profiles.clear();
        for (auto it = j["profiles"].begin(); it != j["profiles"].end(); ++it) {
            m_profiles[it.key()] = AIProviderProfile::FromJson(it.value());
        }
    }
}

bool AIEngineConfigManager::LoadFromFile(const std::string& configFilePath) {
    std::ifstream in(configFilePath);
    if (!in.is_open()) return false;
    try {
        nlohmann::json j;
        in >> j;
        FromJson(j);
        return true;
    } catch (...) {
        return false;
    }
}

bool AIEngineConfigManager::SaveToFile(const std::string& configFilePath) const {
    std::ofstream out(configFilePath);
    if (!out.is_open()) return false;
    out << ToJson().dump(2);
    return true;
}

} // namespace xLights::AI
