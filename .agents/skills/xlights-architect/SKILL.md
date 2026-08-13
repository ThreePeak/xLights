---
name: xlights-architect
description: Core architecture guidelines, memory patterns, rendering rules, and subsystem layout for xLights C++ development.
---

# xLights Architecture Skill Guidelines

## Overview & System Philosophy

xLights is an open-source, cross-platform Christmas light sequencing software suite written in C++20 with wxWidgets for desktop UI and SwiftUI for mobile/iPadOS targets. 

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
├── AI/                                # AI Subsystem (53 C++ / Python Files)
├── src-core/
│   ├── ai/                            # AI Providers & Copilot Engines
│   ├── controllers/                   # Hardware Controller Integration
│   ├── effects/                       # Effect Parameter & Preset Engines
│   ├── lyrics/                        # Phoneme & Lip-Sync Engines
│   ├── media/                         # Audio, Video & Stem Separation
│   ├── models/                        # Custom & SubModel Grid Mathematics
│   ├── outputs/                       # DMX / E1.31 / DDP Network Output Streaming
│   ├── render/                        # Render Buffer Parallel Computation
│   └── scripting/                     # Lua Automation & Macro Execution
├── src-ui-wx/                         # wxWidgets Desktop GUI
└── tools/
    └── mcp_server/                    # xLights REST API (Port 49913) MCP Server
```

---

## 2. Coding Mandates & Best Practices

### 1. Language Standard & Core Decoupling
* Target modern **C++17 / C++20** language standards across all `src-core/` and `src-ui-wx/` modules (`std::unique_ptr`/`std::shared_ptr`, `std::optional`, `std::string_view`, constexpr, `std::atomic`).
* Enforce smart pointers for memory safety (`std::unique_ptr`, `std::shared_ptr`).
* Never include wxWidgets headers (`#include <wx/...>`) in any file under `src-core/` or `AI/`.

### 2. Thread Safety & Async Execution
* Operations such as audio stem separation, AI model inference, parallel rendering, and network streaming MUST run asynchronously on worker threads to keep UI main loops responsive.
* Maintain strict thread safety between `JobPool` worker threads and wxWidgets GUI threads (`wxQueueEvent`).

### 3. Error Diagnostics & Logging
* Use `spdlog` for structured logging:
  * `spdlog::info(...)` for normal operation metrics.
  * `spdlog::warn(...)` for recoverable anomalies.
  * `spdlog::error(...)` for failed operations or invalid parameters.
