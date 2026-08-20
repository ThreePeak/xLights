/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/controllers/AIESP32HardwareOptimizer.h"
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace xLights::AI {

static double GetResistancePerMeter(int awg) {
    switch (awg) {
        case 12: return 0.00521;
        case 14: return 0.00828;
        case 16: return 0.01317;
        case 18: return 0.02095;
        case 20: return 0.03330;
        case 22: return 0.05296;
        case 24: return 0.08422;
        default: return 0.02095;
    }
}

std::string AIESP32HardwareOptimizer::GetBoardName(ESP32BoardType board) {
    switch (board) {
        case ESP32BoardType::ESP32_DEVKIT_V1: return "ESP32 DevKit V1 (Dual-Core Xtensa)";
        case ESP32BoardType::ESP32_S3_DEVKIT: return "ESP32-S3 DevKit (Dual-Core + Vector AI)";
        case ESP32BoardType::ESP32_C3_MINI:   return "ESP32-C3 Mini (Single-Core RISC-V)";
        case ESP32BoardType::ESP32_C6_RISCV:  return "ESP32-C6 (RISC-V + WiFi 6 / Thread)";
        case ESP32BoardType::QUINLED_DIG_UNO: return "QuinLED Dig-Uno (2-Port w/ Level Shifter)";
        case ESP32BoardType::QUINLED_DIG_QUAD:return "QuinLED Dig-Quad (4-Port w/ Level Shifter)";
        case ESP32BoardType::QUINLED_DIG_OCTA:return "QuinLED Dig-Octa (8-Port System Brain)";
        case ESP32BoardType::WEMOS_D1_R32:    return "WEMOS D1 R32 (Arduino Form-Factor)";
        case ESP32BoardType::CUSTOM_DIY_BOARD:return "Custom DIY ESP32 Controller";
        default: return "Unknown ESP32 Board";
    }
}

std::string AIESP32HardwareOptimizer::GetProtocolName(PixelProtocolType protocol) {
    switch (protocol) {
        case PixelProtocolType::WS2811_800KHZ: return "WS2811 (800 kHz High Speed)";
        case PixelProtocolType::WS2812B_800KHZ:return "WS2812B (800 kHz RGB)";
        case PixelProtocolType::SK6812_RGBW:   return "SK6812 (800 kHz RGBW 4-Channel)";
        case PixelProtocolType::GS8208_12V:    return "GS8208 (12V Dual Data Backup)";
        case PixelProtocolType::UCS1903:       return "UCS1903 (800 kHz Standard)";
        case PixelProtocolType::APA102_SPI:    return "APA102 / DotStar (SPI Clock + Data)";
        default: return "Standard 800kHz Pixel";
    }
}

std::vector<GpioPinInfo> AIESP32HardwareOptimizer::GetBoardPinout(ESP32BoardType board) {
    std::vector<GpioPinInfo> pins;
    if (board == ESP32BoardType::QUINLED_DIG_OCTA) {
        pins.push_back({16, "LED1 (Port 1)", false, false, false, false, true, true, false, "Pixel Output", "Level-shifted 74HCT245 buffer"});
        pins.push_back({3,  "LED2 (Port 2)", false, false, false, true,  true, true, false, "Pixel Output", "Level-shifted, boot serial RX"});
        pins.push_back({1,  "LED3 (Port 3)", false, false, false, true,  true, true, false, "Pixel Output", "Level-shifted, boot serial TX"});
        pins.push_back({4,  "LED4 (Port 4)", false, false, false, false, true, true, false, "Pixel Output", "Level-shifted 74HCT245 buffer"});
        pins.push_back({15, "LED5 (Port 5)", true,  false, false, true,  true, true, false, "Pixel Output", "Strapping pin (MTDO), quiet at boot"});
        pins.push_back({13, "LED6 (Port 6)", false, false, false, false, true, true, false, "Pixel Output", "Level-shifted 74HCT245 buffer"});
        pins.push_back({14, "LED7 (Port 7)", false, false, false, false, true, true, false, "Pixel Output", "Level-shifted 74HCT245 buffer"});
        pins.push_back({27, "LED8 (Port 8)", false, false, false, false, true, true, false, "Pixel Output", "Level-shifted 74HCT245 buffer"});
        pins.push_back({32, "Relay / Aux",   false, false, false, false, true, false, false, "Relay", "On-board power relay driver"});
        pins.push_back({33, "Button / I2C",  false, false, false, false, false, false, false, "Input", "Q1/Q2 button input or I2C SDA"});
    } else if (board == ESP32BoardType::QUINLED_DIG_QUAD) {
        pins.push_back({16, "LED1 (Port 1)", false, false, false, false, true, true, false, "Pixel Output", "Level-shifted 74HCT245 buffer"});
        pins.push_back({3,  "LED2 (Port 2)", false, false, false, true,  true, true, false, "Pixel Output", "Level-shifted, boot serial RX"});
        pins.push_back({1,  "LED3 (Port 3)", false, false, false, true,  true, true, false, "Pixel Output", "Level-shifted, boot serial TX"});
        pins.push_back({4,  "LED4 (Port 4)", false, false, false, false, true, true, false, "Pixel Output", "Level-shifted 74HCT245 buffer"});
        pins.push_back({15, "Q1R (Relay)",   true,  false, false, true,  false, false, false, "Relay", "Power cut-off relay"});
    } else if (board == ESP32BoardType::QUINLED_DIG_UNO) {
        pins.push_back({16, "LED1 (Port 1)", false, false, false, false, true, true, false, "Pixel Output", "Level-shifted 74HCT245 buffer"});
        pins.push_back({3,  "LED2 (Port 2)", false, false, false, true,  true, true, false, "Pixel Output", "Level-shifted, boot serial RX"});
        pins.push_back({15, "Q1R (Relay)",   true,  false, false, true,  false, false, false, "Relay", "Power cut-off relay"});
    } else {
        // Generic ESP32 / S3 / C3 / DIY
        pins.push_back({0,  "GPIO 0",  true,  false, false, true,  true, true, false, "Strapping", "Boot mode select (PULL-UP required)"});
        pins.push_back({2,  "GPIO 2",  true,  false, false, true,  true, true, false, "Strapping", "Strapping pin / Onboard LED"});
        pins.push_back({4,  "GPIO 4",  false, false, false, false, true, true, false, "General I/O", "Safe for RMT/I2S pixel output"});
        pins.push_back({5,  "GPIO 5",  true,  false, false, true,  true, true, true,  "Strapping", "Outputs PWM clock at boot"});
        pins.push_back({12, "GPIO 12", true,  false, false, true,  true, true, false, "Strapping", "MTDI flash voltage strapping (CRITICAL)"});
        pins.push_back({13, "GPIO 13", false, false, false, false, true, true, true,  "General I/O", "Safe for RMT/I2S pixel output"});
        pins.push_back({14, "GPIO 14", false, false, false, false, true, true, true,  "General I/O", "Safe for RMT/I2S pixel output"});
        pins.push_back({15, "GPIO 15", true,  false, false, true,  true, true, true,  "Strapping", "MTDO debug print output"});
        pins.push_back({16, "GPIO 16", false, false, false, false, true, true, false, "General I/O", "Safe for RMT/I2S (UART2 RX on some)"});
        pins.push_back({17, "GPIO 17", false, false, false, false, true, true, false, "General I/O", "Safe for RMT/I2S (UART2 TX on some)"});
        pins.push_back({18, "GPIO 18", false, false, false, false, true, true, true,  "VSPI SCK", "Safe for RMT/I2S or SPI clock"});
        pins.push_back({19, "GPIO 19", false, false, false, false, true, true, true,  "VSPI MISO", "Safe for RMT/I2S pixel output"});
        pins.push_back({21, "GPIO 21", false, false, false, false, true, true, false, "I2C SDA", "Safe for RMT/I2S pixel output"});
        pins.push_back({22, "GPIO 22", false, false, false, false, true, true, false, "I2C SCL", "Safe for RMT/I2S pixel output"});
        pins.push_back({23, "GPIO 23", false, false, false, false, true, true, true,  "VSPI MOSI", "Safe for RMT/I2S or SPI data"});
        pins.push_back({25, "GPIO 25", false, false, false, false, true, true, false, "DAC1", "Safe for RMT/I2S pixel output"});
        pins.push_back({26, "GPIO 26", false, false, false, false, true, true, false, "DAC2", "Safe for RMT/I2S pixel output"});
        pins.push_back({27, "GPIO 27", false, false, false, false, true, true, false, "General I/O", "Safe for RMT/I2S pixel output"});
        pins.push_back({32, "GPIO 32", false, false, false, false, true, true, false, "ADC1_CH4", "Safe for RMT/I2S pixel output"});
        pins.push_back({33, "GPIO 33", false, false, false, false, true, true, false, "ADC1_CH5", "Safe for RMT/I2S pixel output"});
        pins.push_back({34, "GPIO 34", false, true,  false, false, false,false, false, "Input Only", "Cannot output pixel signals!"});
        pins.push_back({35, "GPIO 35", false, true,  false, false, false,false, false, "Input Only", "Cannot output pixel signals!"});
        pins.push_back({36, "GPIO 36", false, true,  false, false, false,false, false, "Input Only", "Cannot output pixel signals!"});
        pins.push_back({39, "GPIO 39", false, true,  false, false, false,false, false, "Input Only", "Cannot output pixel signals!"});
    }
    return pins;
}

ESP32HardwareOptimizationResult AIESP32HardwareOptimizer::OptimizeHardwareLayout(const ESP32HardwareOptimizationRequest& req) {
    ESP32HardwareOptimizationResult res;
    res.boardName = GetBoardName(req.boardType);
    
    if (req.boardType == ESP32BoardType::ESP32_S3_DEVKIT) {
        res.chipArchitecture = "Xtensa LX7 Dual-Core 240MHz (Vector Instructions / RMT 4-ch + I2S LCD DMA)";
    } else if (req.boardType == ESP32BoardType::ESP32_C3_MINI || req.boardType == ESP32BoardType::ESP32_C6_RISCV) {
        res.chipArchitecture = "RISC-V 32-bit Single-Core 160MHz (RMT 2-ch Tx)";
    } else {
        res.chipArchitecture = "Xtensa LX6 Dual-Core 240MHz (RMT 8-ch + I2S0/I2S1 DMA)";
    }

    auto boardPins = GetBoardPinout(req.boardType);
    std::vector<GpioPinInfo> availablePins;

    for (const auto& pin : boardPins) {
        if (pin.isInputOnly || pin.isFlashSpi) continue;
        if (req.avoidStrappingPinsStrict && pin.isStrapping) continue;
        if (std::find(req.userReservedGpios.begin(), req.userReservedGpios.end(), pin.gpio) != req.userReservedGpios.end()) continue;
        availablePins.push_back(pin);
    }

    int requestedPorts = std::max(1, std::min(req.numPixelPorts, 16));
    int totalPixels = requestedPorts * req.pixelsPerPort;
    res.totalPixelsSupported = totalPixels;

    // Refresh rate calculation: 800kHz WS2811 = 30us per pixel + 300us latch
    double frameTimeMs = (req.pixelsPerPort * 0.030) + 0.300;
    res.estimatedMaxFps = (frameTimeMs > 0.0) ? std::min(100.0, 1000.0 / frameTimeMs) : 40.0;

    // Power calculation: WS2811 5V/12V typical 0.055A full white per node
    double ampsPerPort = req.pixelsPerPort * 0.055;
    res.maxTotalCurrentAmps = requestedPorts * ampsPerPort;

    double wireResistance = GetResistancePerMeter(req.wireGaugeAwg) * req.wireLengthMeters;
    double roundTripResistance = wireResistance * 2.0;

    int rmtChannelsAvailable = (req.boardType == ESP32BoardType::ESP32_C3_MINI) ? 2 : (req.boardType == ESP32BoardType::ESP32_S3_DEVKIT ? 4 : 8);

    for (int i = 0; i < requestedPorts; ++i) {
        PortPinAssignment port;
        port.portIndex = i + 1;
        port.pixelCount = req.pixelsPerPort;

        if (i < (int)availablePins.size()) {
            port.assignedGpio = availablePins[i].gpio;
            port.portLabel = availablePins[i].label;
        } else {
            port.assignedGpio = -1;
            port.portLabel = "Unassigned (Out of safe GPIOs)";
            port.isSafeToOperate = false;
            port.warningMessage = "Not enough safe GPIO pins available on this board profile.";
            res.criticalWarnings.push_back("Port " + std::to_string(port.portIndex) + " could not be allocated a safe GPIO.");
        }

        // Driver selection
        if (req.protocol == PixelProtocolType::APA102_SPI) {
            port.driverType = PeripheralDriverType::SPI_DMA;
            port.driverChannelIndex = 1;
        } else if (i < rmtChannelsAvailable) {
            port.driverType = PeripheralDriverType::RMT_CHANNEL;
            port.driverChannelIndex = i;
        } else {
            port.driverType = PeripheralDriverType::I2S_PARALLEL_DMA;
            port.driverChannelIndex = 0;
        }

        // Hardware line tuning
        if (req.wireLengthMeters > 3.0) {
            port.inlineResistorOhms = 249.0;
            port.levelShifterRecommended = true;
            port.recommendation = "Use 74AHCT125/74HCT245 level shifter with 249-ohm damping resistor for line length > 3m.";
        } else {
            port.inlineResistorOhms = 33.0;
            port.levelShifterRecommended = (req.boardType != ESP32BoardType::QUINLED_DIG_UNO && 
                                            req.boardType != ESP32BoardType::QUINLED_DIG_QUAD && 
                                            req.boardType != ESP32BoardType::QUINLED_DIG_OCTA);
            port.recommendation = "Direct run with 33-ohm ringing damping resistor.";
        }

        // Voltage drop calculation
        double drop = (ampsPerPort * 0.5) * roundTripResistance; // assume 50% power injection distribution
        port.estimatedVoltageDropVolts = drop;
        port.voltageAtPixelStrip = std::max(0.0, req.supplyVoltageVolts - drop);

        if (port.voltageAtPixelStrip < (req.supplyVoltageVolts * 0.85)) {
            port.warningMessage += " Severe voltage drop predicted (" + std::to_string(drop).substr(0,4) + "V). Power injection required!";
            res.optimizationNotes.push_back("Port " + std::to_string(port.portIndex) + ": Inject 12V power at pixel #" + std::to_string(req.pixelsPerPort / 2) + ".");
        }

        res.portAssignments.push_back(port);
    }

    res.success = res.criticalWarnings.empty();
    if (res.success) {
        res.optimizationNotes.push_back("Configured " + std::to_string(requestedPorts) + " parallel pixel ports for " + res.boardName);
        res.optimizationNotes.push_back("RMT/I2S DMA multi-channel mapping prevents CPU execution jitter during WiFi reception.");
    }
    return res;
}

std::string ESP32HardwareOptimizationResult::GenerateESPixelStickJsonConfig() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"device\": {\n";
    ss << "    \"id\": \"" << boardName << "\",\n";
    ss << "    \"blanktime\": 5,\n";
    ss << "    \"miso_pin\": -1,\n";
    ss << "    \"mosi_pin\": -1,\n";
    ss << "    \"clk_pin\": -1,\n";
    ss << "    \"cs_pin\": -1\n";
    ss << "  },\n";
    ss << "  \"outputs\": [\n";
    for (size_t i = 0; i < portAssignments.size(); ++i) {
        const auto& p = portAssignments[i];
        ss << "    {\n";
        ss << "      \"type\": \"Pixel\",\n";
        ss << "      \"id\": " << i << ",\n";
        ss << "      \"gpio\": " << p.assignedGpio << ",\n";
        ss << "      \"pixel_count\": " << p.pixelCount << ",\n";
        ss << "      \"color_order\": \"rgb\",\n";
        ss << "      \"driver\": \"" << (p.driverType == PeripheralDriverType::RMT_CHANNEL ? "RMT" : "I2S") << "\"\n";
        ss << "    }" << (i + 1 < portAssignments.size() ? "," : "") << "\n";
    }
    ss << "  ]\n";
    ss << "}\n";
    return ss.str();
}

std::string ESP32HardwareOptimizationResult::GenerateWledJsonConfig() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"hw\": {\n";
    ss << "    \"led\": {\n";
    ss << "      \"total\": " << totalPixelsSupported << ",\n";
    ss << "      \"maxpwr\": 8500,\n";
    ss << "      \"ins\": [\n";
    for (size_t i = 0; i < portAssignments.size(); ++i) {
        const auto& p = portAssignments[i];
        ss << "        {\n";
        ss << "          \"start\": " << (i * p.pixelCount) << ",\n";
        ss << "          \"len\": " << p.pixelCount << ",\n";
        ss << "          \"pin\": [" << p.assignedGpio << "],\n";
        ss << "          \"type\": 22,\n";
        ss << "          \"order\": 0\n";
        ss << "        }" << (i + 1 < portAssignments.size() ? "," : "") << "\n";
    }
    ss << "      ]\n";
    ss << "    }\n";
    ss << "  }\n";
    ss << "}\n";
    return ss.str();
}

std::string ESP32HardwareOptimizationResult::GeneratePlatformIoIni() const {
    std::ostringstream ss;
    ss << "; Auto-generated by xLights AI ESP32 Hardware Optimizer\n";
    ss << "[env:esp32_pixel_controller]\n";
    ss << "platform = espressif32\n";
    ss << "board = esp32dev\n";
    ss << "framework = arduino\n";
    ss << "upload_speed = 921600\n";
    ss << "monitor_speed = 115200\n";
    ss << "build_flags =\n";
    ss << "  -D CORE_DEBUG_LEVEL=0\n";
    ss << "  -D CONFIG_FREERTOS_UNICORE=0\n";
    ss << "  -D NUM_PIXEL_PORTS=" << portAssignments.size() << "\n";
    for (size_t i = 0; i < portAssignments.size(); ++i) {
        ss << "  -D PIXEL_PORT_" << (i + 1) << "_GPIO=" << portAssignments[i].assignedGpio << "\n";
    }
    return ss.str();
}

std::string ESP32HardwareOptimizationResult::GenerateFormattedReport() const {
    std::ostringstream ss;
    ss << "=================================================================\n";
    ss << "   xLights AI ESP32 Hardware & DMA Pinout Optimization Report   \n";
    ss << "=================================================================\n\n";
    ss << "Target Controller: " << boardName << "\n";
    ss << "Architecture:      " << chipArchitecture << "\n";
    ss << "Total Pixels:      " << totalPixelsSupported << " nodes\n";
    ss << "Estimated Max FPS: " << std::fixed << std::setprecision(1) << estimatedMaxFps << " FPS\n";
    ss << "Peak Current Draw: " << std::fixed << std::setprecision(2) << maxTotalCurrentAmps << " Amps\n\n";
    ss << "Port Pin Assignments:\n";
    ss << "-----------------------------------------------------------------\n";
    for (const auto& p : portAssignments) {
        ss << "Port #" << p.portIndex << ": GPIO " << p.assignedGpio << " (" << p.portLabel << ")\n";
        ss << "  - Driver:      " << (p.driverType == PeripheralDriverType::RMT_CHANNEL ? "RMT Channel" : "I2S DMA") << "\n";
        ss << "  - Nodes:       " << p.pixelCount << " pixels\n";
        ss << "  - Resistor:    " << p.inlineResistorOhms << " Ohms inline\n";
        ss << "  - Voltage Sag: " << std::fixed << std::setprecision(2) << p.estimatedVoltageDropVolts << "V (Strip sees: " << p.voltageAtPixelStrip << "V)\n";
        if (!p.warningMessage.empty()) {
            ss << "  - Warning:     " << p.warningMessage << "\n";
        }
        ss << "  - Notes:       " << p.recommendation << "\n\n";
    }
    return ss.str();
}

} // namespace xLights::AI
