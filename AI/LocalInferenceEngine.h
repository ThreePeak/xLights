#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Feature #15: Hardware-Accelerated Local Inference Engine (OpenVINO / CoreML / DirectML)

#include "AISubsystemBase.h"
#include <string>
#include <vector>
#include <map>
#include <future>

namespace xLights::AI {

enum class ExecutionProvider {
    CPU,
    DirectML,      // Windows DirectX GPU / NPU Acceleration
    OpenVINO,      // Intel CPU / NPU Acceleration
    CoreML,        // Apple Silicon Neural Engine (macOS / iOS)
    CUDA           // NVIDIA GPU Acceleration
};

struct HardwareDeviceInfo {
    std::string deviceName;            // e.g. "Intel NPU / DirectML GPU"
    ExecutionProvider provider = ExecutionProvider::CPU;
    bool isAvailable = false;
    uint64_t dedicatedVRAMBytes = 0;
    std::string driverVersion;
};

struct InferenceSessionConfig {
    std::string modelFilePath;         // Path to ONNX / OpenVINO model
    ExecutionProvider preferredProvider = ExecutionProvider::DirectML;
    bool enableQuantizationINT8 = true;
    int intraOpThreads = 4;
    bool zeroCopyMemory = true;
};

struct InferenceSessionStatus {
    bool isLoaded = false;
    std::string activeProviderName;
    std::string modelPath;
    float avgInferenceLatencyMs = 0.0f;
    uint64_t memoryAllocatedBytes = 0;
    std::string deviceName;
};

class LocalInferenceEngine : public AISubsystemBase {
public:
    LocalInferenceEngine(ServiceManager* sm = nullptr) : AISubsystemBase(sm) {}
    virtual ~LocalInferenceEngine() override = default;

    virtual std::future<bool> InitializeAsync(StatusCallback callback = nullptr) override {
        return std::async(std::launch::async, [this, callback]() {
            if (callback) callback("Initializing LocalInferenceEngine...", 0.0f);
            m_isInitialized.store(true);
            if (callback) callback("LocalInferenceEngine initialized.", 100.0f);
            return true;
        });
    }

    virtual void Shutdown() override {
        m_isInitialized.store(false);
    }

    [[nodiscard]] virtual std::string GetSubsystemName() const override { return "LocalInferenceEngine"; }
    [[nodiscard]] virtual std::vector<std::string> GetCapabilities() const override {
        return {"local_hardware_acceleration", "directml_openvino_coreml", "int8_quantization", "zero_copy_memory"};
    }

    // Enumerates available local hardware execution providers (DirectML, OpenVINO, CoreML, CUDA, CPU)
    [[nodiscard]] static std::vector<HardwareDeviceInfo> EnumerateHardwareDevices();

    // Initializes a local hardware-accelerated model inference session
    [[nodiscard]] static InferenceSessionStatus CreateInferenceSession(const InferenceSessionConfig& config);

    // Converts ExecutionProvider enum to human readable string name
    [[nodiscard]] static std::string ProviderToString(ExecutionProvider provider);
};

} // namespace xLights::AI
