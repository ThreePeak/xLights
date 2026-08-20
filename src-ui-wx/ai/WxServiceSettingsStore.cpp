#include "WxServiceSettingsStore.h"

#include "settings/XLightsConfigAdapter.h"
#include "AI/AIConfigurationManager.h"

#include <wx/secretstore.h>

#include <log.h>

#include <string>

WxServiceSettingsStore::WxServiceSettingsStore() = default;

std::string WxServiceSettingsStore::getString(std::string_view key, const std::string& defaultValue) const {
    auto* config = GetXLightsConfig();
    return config->Read(std::string(key), defaultValue);
}

int WxServiceSettingsStore::getInt(std::string_view key, int defaultValue) const {
    auto* config = GetXLightsConfig();
    return static_cast<int>(config->Read(std::string(key), static_cast<long>(defaultValue)));
}

bool WxServiceSettingsStore::getBool(std::string_view key, bool defaultValue) const {
    auto* config = GetXLightsConfig();
    return config->ReadBool(std::string(key), defaultValue);
}

void WxServiceSettingsStore::setString(std::string_view key, const std::string& value) {
    auto* config = GetXLightsConfig();
    config->Write(std::string(key), value);
    config->Flush();
    SyncToAIConfigurationManager();
}

void WxServiceSettingsStore::setInt(std::string_view key, int value) {
    auto* config = GetXLightsConfig();
    config->Write(std::string(key), value);
    config->Flush();
    SyncToAIConfigurationManager();
}

void WxServiceSettingsStore::setBool(std::string_view key, bool value) {
    auto* config = GetXLightsConfig();
    config->Write(std::string(key), value);
    config->Flush();
    SyncToAIConfigurationManager();
}

#if wxUSE_SECRETSTORE

namespace {
// Lazily-constructed singleton so static-init order doesn't bite.
wxSecretStore& pwdStore() {
    static wxSecretStore store = wxSecretStore::GetDefault();
    return store;
}
}

std::string WxServiceSettingsStore::getSecret(std::string_view serviceName) const {
    auto& store = pwdStore();
    if (store.IsOk()) {
        wxSecretValue password;
        wxString usr;
        if (store.Load(wxString(std::string(serviceName)), usr, password)) {
            return password.GetAsString().ToStdString();
        }
    }
    return {};
}

void WxServiceSettingsStore::setSecret(std::string_view serviceName, const std::string& token) {
    auto& store = pwdStore();
    if (!store.IsOk()) {
        return;
    }
    wxSecretValue tt(token);
    if (!store.Save(wxString(std::string(serviceName)), "token", tt)) {
        spdlog::warn("Failed to save secret for {}", std::string(serviceName));
    }
}

#else

std::string WxServiceSettingsStore::getSecret(std::string_view serviceName) const {
    return getString(std::string(serviceName) + "_token", std::string());
}

void WxServiceSettingsStore::setSecret(std::string_view serviceName, const std::string& token) {
    setString(std::string(serviceName) + "_token", token);
    SyncToAIConfigurationManager();
}

#endif

void WxServiceSettingsStore::SyncToAIConfigurationManager() {
    using namespace xLights::AI;
    auto* config = GetXLightsConfig();
    if (!config) return;

    AIConfigSettings cfg;
    cfg.executionProvider = static_cast<int>(config->Read("AI_ExecutionProvider", 0L));
    cfg.quantizationPrecision = static_cast<int>(config->Read("AI_QuantizationPrecision", 0L));
    cfg.cpuThreads = static_cast<int>(config->Read("AI_CpuThreads", 4L));
    cfg.vramCapMb = static_cast<int>(config->Read("AI_VRAMCapMB", 4096L));
    cfg.onnxModelDir = config->Read("AI_OnnxModelDir", "");
    cfg.primaryModel = static_cast<int>(config->Read("AI_PrimaryModel", 0L));
    cfg.openAIKey = config->Read("AI_OpenAIKey", "");
    cfg.anthropicKey = config->Read("AI_AnthropicKey", "");
    cfg.geminiKey = config->Read("AI_GeminiKey", "");
    cfg.deepSeekKey = config->Read("AI_DeepSeekKey", "");
    cfg.customEndpoint = config->Read("AI_CustomEndpoint", "http://localhost:8000/v1");
    cfg.ollamaEndpoint = config->Read("AI_OllamaEndpoint", "http://localhost:11434");
    cfg.temperature = static_cast<float>(config->Read("AI_Temperature", 70L)) / 100.0f;
    cfg.topP = static_cast<float>(config->Read("AI_TopP", 95L)) / 100.0f;
    cfg.maxTokens = static_cast<int>(config->Read("AI_MaxTokens", 4096L));
    cfg.systemPrompt = config->Read("AI_SystemPrompt",
        "You are an expert xLights lighting sequence copilot.");

    AIConfigurationManager::Instance().UpdateSettings(cfg);
}
