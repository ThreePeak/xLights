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
#include <cstdint>

namespace xLights::AI {

enum class ESP32BoardType {
    ESP32_DEVKIT_V1,
    ESP32_S3_DEVKIT,
    ESP32_C3_MINI,
    ESP32_C6_RISCV,
    QUINLED_DIG_UNO,
    QUINLED_DIG_QUAD,
    QUINLED_DIG_OCTA,
    WEMOS_D1_R32,
    CUSTOM_DIY_BOARD
};

enum class PixelProtocolType {
    WS2811_800KHZ,
    WS2812B_800KHZ,
    SK6812_RGBW,
    GS8208_12V,
    UCS1903,
    APA102_SPI
};

enum class PeripheralDriverType {
    RMT_CHANNEL,
    I2S_PARALLEL_DMA,
    SPI_DMA,
    BITBANG_GPIO
};

struct GpioPinInfo {
    int gpio{-1};
    std::string label;
    bool isStrapping{false};
    bool isInputOnly{false};
    bool isFlashSpi{false};
    bool isBootGlitch{false};
    bool supportsRmt{true};
    bool supportsI2sDma{true};
    bool supportsSpi{true};
    std::string defaultRole;
    std::string notes;
};

struct PortPinAssignment {
    int portIndex{0};
    int assignedGpio{-1};
    std::string portLabel;
    PeripheralDriverType driverType{PeripheralDriverType::RMT_CHANNEL};
    int driverChannelIndex{0};
    int pixelCount{0};
    double inlineResistorOhms{33.0};
    bool levelShifterRecommended{true};
    double estimatedVoltageDropVolts{0.0};
    double voltageAtPixelStrip{0.0};
    bool isSafeToOperate{true};
    std::string warningMessage;
    std::string recommendation;
};

struct ESP32HardwareOptimizationRequest {
    ESP32BoardType boardType{ESP32BoardType::QUINLED_DIG_QUAD};
    int numPixelPorts{4};
    int pixelsPerPort{600};
    PixelProtocolType protocol{PixelProtocolType::WS2811_800KHZ};
    double supplyVoltageVolts{12.0};
    double wireLengthMeters{5.0};
    int wireGaugeAwg{18};
    bool avoidStrappingPinsStrict{true};
    bool optimizeForHighFrameRate{true};
    std::vector<int> userReservedGpios;
};

struct ESP32HardwareOptimizationResult {
    bool success{false};
    std::string boardName;
    std::string chipArchitecture;
    int totalPixelsSupported{0};
    double estimatedMaxFps{40.0};
    double maxTotalCurrentAmps{0.0};
    std::vector<PortPinAssignment> portAssignments;
    std::vector<std::string> criticalWarnings;
    std::vector<std::string> optimizationNotes;

    std::string GenerateESPixelStickJsonConfig() const;
    std::string GenerateWledJsonConfig() const;
    std::string GeneratePlatformIoIni() const;
    std::string GenerateFormattedReport() const;
};

class AIESP32HardwareOptimizer {
public:
    static ESP32HardwareOptimizationResult OptimizeHardwareLayout(const ESP32HardwareOptimizationRequest& request);
    static std::vector<GpioPinInfo> GetBoardPinout(ESP32BoardType board);
    static std::string GetBoardName(ESP32BoardType board);
    static std::string GetProtocolName(PixelProtocolType protocol);
};

} // namespace xLights::AI
