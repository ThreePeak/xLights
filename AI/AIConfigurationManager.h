/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <string>
#include <mutex>
#include <memory>

namespace xLights::AI {

struct AIConfigSettings {
    int executionProvider = 0;             // 0=DirectML, 1=CUDA, 2=OpenVINO, 3=CoreML, 4=CPU
    int quantizationPrecision = 0;         // 0=INT8, 1=FP16, 2=FP32
    int cpuThreads = 4;
    int vramCapMb = 4096;
    std::string onnxModelDir;

    int primaryModel = 0;                  // 0=OpenAI, 1=Claude, 2=Gemini, 3=DeepSeek, 4=Ollama, 5=Custom
    std::string openAIKey;
    std::string anthropicKey;
    std::string geminiKey;
    std::string deepSeekKey;
    std::string customEndpoint = "http://localhost:8000/v1";
    std::string ollamaEndpoint = "http://localhost:11434";

    float temperature = 0.70f;
    float topP = 0.95f;
    int maxTokens = 4096;
    float frequencyPenalty = 0.0f;
    float presencePenalty = 0.0f;

    std::string systemPrompt = "You are an expert xLights lighting sequence copilot. Prioritize rhythmic musical timing, harmonious color palettes, and efficient prop rendering.";
};

class AIConfigurationManager {
public:
    static AIConfigurationManager& Instance() {
        static AIConfigurationManager instance;
        return instance;
    }

    AIConfigSettings GetSettings() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_settings;
    }

    void UpdateSettings(const AIConfigSettings& settings) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_settings = settings;
    }

    std::string GetActiveApiKey() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_settings.primaryModel == 1) return m_settings.anthropicKey;
        if (m_settings.primaryModel == 2) return m_settings.geminiKey;
        if (m_settings.primaryModel == 3) return m_settings.deepSeekKey;
        return m_settings.openAIKey;
    }

private:
    AIConfigurationManager() = default;
    ~AIConfigurationManager() = default;
    AIConfigurationManager(const AIConfigurationManager&) = delete;
    AIConfigurationManager& operator=(const AIConfigurationManager&) = delete;

    mutable std::mutex m_mutex;
    AIConfigSettings m_settings;
};

} // namespace xLights::AI
