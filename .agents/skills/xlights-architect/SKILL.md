---
name: xlights-architect
description: Core architecture guidelines, memory patterns, and rendering rules for xLights C++ development.
---

# xLights Architecture Skill Guidelines

## Overview & System Philosophy

xLights is a open-source, cross-platform Christmas light sequencing software suite written in C++20 with wxWidgets for desktop UI and SwiftUI for mobile/iPadOS targets. 

The primary architectural mandate is **Core Decoupling**:
* **`src-core/`**: Pure C++ engine containing all rendering buffers, model math, AI services, audio decoders, controller uploaders, and output network managers. **`src-core/` MUST NOT depend on wxWidgets or any GUI header.**
* **`src-ui-wx/`**: Desktop user interface built with wxWidgets. Consumes `src-core/` classes via callbacks, events, and value objects.
* **`src-iPad/`**: Mobile/macOS app built with SwiftUI. Consumes `src-core/` classes via C++ wrappers.

---

## 1. Codebase Landscape

```
xLights/
├── docs/
│   └── AI_FEATURE_PLAN.md             # Master Architecture Specification for 15 AI Features
├── src-core/
│   ├── ai/                            # AI Providers & Copilot Engines
│   │   ├── ServiceManager.h/.cpp      # Central AI service registry & provider lifecycle
│   │   ├── aiBase.h                   # C++ AI capability contracts (Prompts, Palettes, Images)
│   │   ├── SequenceDiagnosticsCopilot.h/.cpp # Diagnostic auditor & conversational copilot
│   │   ├── SequenceRemappingAgent.h/.cpp     # Vector embedding similarity model matcher
│   │   └── ShowLogDiagnosticsAnalyzer.h/.cpp # spdlog & pugixml error analyzer
│   ├── controllers/                   # Physical Lighting Controller Hardware Integration
│   │   └── PowerInjectionAnalyzer.h/.cpp     # Ohm's Law voltage drop & load balancer
│   ├── effects/                       # Effect Parameter & Preset Engines
│   │   └── EffectPresetManager.h/.cpp # Multi-layer preset synthesis & schema validator
│   ├── lyrics/                        # Phoneme & Lip-Sync Engines
│   │   ├── PhonemeMap.h/.cpp          # Whisper/Wav2Vec2 forced alignment & visemes
│   │   └── PhonemeDictionary.h/.cpp   # ARPAbet / CMU phoneme dictionary
│   ├── media/                         # Audio, Video & Stem Separation
│   │   ├── AudioDecoder.h/.cpp        # Cross-platform decoding & Demucs ONNX stem separator
│   │   ├── AudioDynamicsMapper.h/.cpp # Musical emotion & dynamics contour mapper
│   │   ├── PhotorealisticVisualizerRenderer.h/.cpp # ControlNet / SD .fseq video renderer
│   │   └── StemSeparator.h/.cpp       # HTDemucs 4-stem C++ separation pipeline
│   ├── models/                        # Custom & SubModel Grid Mathematics
│   │   ├── CustomModel.h/.cpp         # CV spatial mesh synthesis & Gray Code camera mapper
│   │   └── SubModelOps.h/.cpp         # SAM submodel auto-detector & face isolator
│   ├── outputs/                       # DMX / E1.31 / DDP Network Output Streaming
│   │   └── OutputManager.h/.cpp       # Real-time voice-controlled layout tester
│   ├── render/                        # Render Buffer Parallel Computation
│   │   └── RenderBuffer.h/.cpp        # Parallel pixel rendering & layer blend advisor
│   └── scripting/                     # Lua Automation & Macro Execution
│       └── LuaManager.h/.cpp          # Natural language to Lua macro converter
├── src-ui-wx/                         # wxWidgets Desktop GUI
│   ├── ai/                            # AI Dialogs (Palette, Image, Copilot)
│   └── shared/dialogs/                # ValueCurveDialog with AI Curve Prompt
└── tools/
    └── mcp_server/                    # xLights REST API (Port 49913) MCP Server
        ├── server.py                  # Python JSON-RPC 2.0 MCP server
        ├── index.js                   # Node.js JSON-RPC 2.0 MCP server
        └── package.json               # MCP package metadata
```

---

## 2. Coding Mandates & Best Practices for Subagents

### 1. Language Standard & Core Decoupling
* Target modern **C++17 / C++20** language standards across all `src-core/` and `src-ui-wx/` modules (smart pointers `std::unique_ptr`/`std::shared_ptr`, `std::optional`, `std::string_view`, constexpr, `std::atomic`).
* **Smart Pointer Memory Safety:** Enforce modern smart pointers (`std::unique_ptr`, `std::shared_ptr`, `std::weak_ptr`) for all custom AI modules and service instances to eliminate raw pointer leaks (`delete` / `malloc`).
* Never include wxWidgets headers (`#include <wx/...>`) in any file under `src-core/`.
* Use standard C++ primitives (`std::string`, `std::vector`, `std::map`, `std::function`) or header-only GLM / kiss_fft in core classes.
* **AI Algorithm Isolation:** Isolate all custom AI algorithms, copilot engines, diagnostic analyzers, remapping agents, and service providers inside `src-core/ai/` (and `xLights/AI/`).

### 2. Thread Safety & Async Execution
* Heavy operations (audio stem separation, AI model inference, parallel rendering, network pinging) MUST run asynchronously on worker threads to keep UI main loops responsive.
* **Strict Thread Safety:** Maintain strict thread safety between `JobPool` worker threads and wxWidgets GUI threads (`xLightsFrame.cpp` / `MainFrame.cpp`). Never mutate wxWidgets UI controls directly from `JobPool` threads; use thread-safe `wxCommandEvent` dispatches (`wxQueueEvent`) or atomic locks (`std::mutex`, `std::lock_guard`).

### 3. Error Diagnostics & Logging
* Use `spdlog` for structured logging:
  * `spdlog::info(...)` for normal operation metrics.
  * `spdlog::warn(...)` for recoverable anomalies.
  * `spdlog::error(...)` for failed operations or invalid parameters.

### 4. Verification & Testing Strategy
* Always run automated unit tests or build commands after editing C++ core files.
* Ensure all XML parsing uses `pugixml` (`pugi::xml_document`, `pugi::xml_node`).

### 5. Multi-Threaded Rendering & Memory Architecture
* **`JobPool.cpp` / `JobPool.h`**: Manages the worker thread pool for parallel sequence rendering across multi-core CPUs.
* **`RenderBuffer.cpp` / `RenderBuffer.h`**: Operates on 2D RGBA pixel arrays (`xlColor` / `uint8_t` RGBA buffers) of size `BufferWi * BufferHt`. Utilizes multi-threaded parallel loops (`parallel_for`) and fast-path SIMD pixel math for layer blending and effect rendering.

### 6. Desktop UI Architecture & Event Loop
* **wxWidgets 3.2+ Infrastructure**: Desktop UI is built using wxWidgets 3.2+ (`xLightsApp.cpp` application entry point, `xLightsFrame.cpp` / `MainFrame.cpp` main window frame).
* **Event Dispatching**: UI events pass asynchronous worker thread data back to the main GUI loop using custom `wxCommandEvent` dispatches (`RenderCommandEvent`, `AICommandEvent`) to ensure flicker-free rendering and main-thread safety.

### 7. Layout Model Grid & Submodel Architecture
* **`Model.cpp` / `Model.h`**: Base class for all physical light display models (Matrices, Trees, Stars, Arches, SingleStrands). Defines 3D spatial coordinates, buffer styles, string counts, and node channel maps.
* **`SubModel.cpp` / `SubModel.h` & `SubModelOps.cpp`**: Sub-groupings of model node matrices. Operates on strand ranges (`"1-50,60,70-80"`) to isolate physical prop components (Singing Face eyes/mouth, snowflake spokes, star rings).
* **`CustomModel.cpp` / `CustomModel.h`**: Handles arbitrary 2D/3D custom model node grids (`std::vector<std::vector<std::vector<int>>> _locations`). Integrates computer vision mesh generation (`GenerateSpatialMeshFromImage`) and OpenCV Gray Code camera mapping.

### 8. Controller Network Protocols & Upload Systems
* **Streaming Protocols (`OutputManager.cpp`, `ControllerEthernet`)**: Real-time channel frame streaming over E1.31 (Streaming ACN), DDP (Distributed Display Protocol), Art-Net, and ZMQ output threads.
* **REST API & Controller Configuration (`FPPConnectDialog.cpp`, `FPP.h`, `Falcon.h`, `WLED.h`)**: Communicates with physical controllers (FPP, Falcon, Kulp, WLED, ESPixelStick) over HTTP/REST APIs to push sequence files, universe channel maps, port definitions, and DDP configurations directly to hardware.

### 9. Effects Class Hierarchy & Inheritance
* **`Effect.h` / `Effect.cpp`**: Base class for all xLights rendering effects (Bars, Fire, Twinkle, Meteors, SingleStrands, Morph, ColorWash, Text, Video). All built-in and AI-synthesized effects inherit from `Effect` and implement `Render(RenderBuffer& buffer)` using parameter maps (`SettingsMap mSettings`) validated against schemas in `resources/effectmetadata/*.json`.
