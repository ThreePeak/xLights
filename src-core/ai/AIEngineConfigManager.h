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
#include <mutex>
#include <nlohmann/json.hpp>

namespace xLights::AI {

enum class AIProviderType {
    OPENAI = 0,         ///< OpenAI (GPT-4o, GPT-4o-mini)
    ANTHROPIC = 1,      ///< Anthropic (Claude 3.5 Sonnet, Claude 3 Opus)
    GOOGLE_GEMINI = 2,  ///< Google DeepMind (Gemini 1.5 Pro, Gemini 1.5 Flash)
    LOCAL_OLLAMA = 3,   ///< Local Ollama Server (e.g. http://localhost:11434)
    LOCAL_VLLM = 4,     ///< Local vLLM / OpenAI-compatible endpoint
    LOCAL_ONNX = 5      ///< Embedded In-Process ONNX Runtime (Zero Network)
};

/**
 * @brief Configuration settings for a specific AI / LLM provider.
 */
struct AIProviderProfile {
    std::string profileName{"Default"};
    AIProviderType providerType{AIProviderType::OPENAI};
    std::string modelName{"gpt-4o"};
    std::string apiKey;
    std::string customEndpointUrl{"https://api.openai.com/v1"};
    float temperature{0.7f};
    float topP{0.95f};
    int maxTokens{4096};
    int timeoutSeconds{30};
    bool enableStreaming{true};
    bool isLocalOffline{false};

    [[nodiscard]] nlohmann::json ToJson() const;
    static AIProviderProfile FromJson(const nlohmann::json& j);
};

/**
 * @brief Centralized AI LLM & Vision Subsystem Configuration Manager.
 */
class AIEngineConfigManager {
public:
    static AIEngineConfigManager& Instance();

    void SetActiveProfile(const std::string& profileName);
    [[nodiscard]] AIProviderProfile GetActiveProfile() const;

    void SaveProfile(const AIProviderProfile& profile);
    void DeleteProfile(const std::string& profileName);
    [[nodiscard]] std::vector<std::string> GetAvailableProfileNames() const;

    /// Global Safety & Fallback Controls
    void SetOfflineMode(bool offlineOnly);
    [[nodiscard]] bool IsOfflineMode() const;

    void SetMonthlyTokenBudget(uint64_t maxTokens);
    [[nodiscard]] uint64_t GetMonthlyTokenBudget() const;
    [[nodiscard]] uint64_t GetCurrentTokenUsage() const;
    void RecordTokenUsage(uint64_t tokensUsed);

    /// Persistence
    bool LoadFromFile(const std::string& configFilePath);
    bool SaveToFile(const std::string& configFilePath) const;

    [[nodiscard]] nlohmann::json ToJson() const;
    void FromJson(const nlohmann::json& j);

private:
    AIEngineConfigManager();
    ~AIEngineConfigManager() = default;

    mutable std::mutex m_mutex;
    std::map<std::string, AIProviderProfile> m_profiles;
    std::string m_activeProfileName{"Default"};
    bool m_offlineOnlyMode{false};
    uint64_t m_monthlyTokenBudget{1000000};
    uint64_t m_currentTokenUsage{12450};
};

} // namespace xLights::AI
