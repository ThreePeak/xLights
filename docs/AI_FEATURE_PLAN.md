# xLights AI Expansion Roadmap: Master Architecture Specification

## Executive Summary & Architectural Vision

This specification defines the master architecture and implementation roadmap for the **15 Pre-Production AI Features** in xLights. Designed as a unified, cross-platform artificial intelligence ecosystem, this suite bridges desktop platforms (Windows, macOS, Linux) and mobile platforms (iOS/iPadOS). 

The primary architectural principle is **Core Decoupling**: all AI providers, capabilities, property definitions, and serialization mechanisms reside in `src-core/ai/` as header-only or pure C++ modules. Native UI implementations (`src-ui-wx/` for desktop wxWidgets and `src-iPad/` for SwiftUI) consume these shared components via uniform service contracts (`ServiceManager`, `ServiceProperty`, `aiBase`).

---

## Phased Expansion Roadmap

```
+-----------------------------------------------------------------------------------+
|                           xLights AI Expansion Roadmap                            |
+-----------------------------------------------------------------------------------+
  Phase 1: Core Foundation & High-Leverage Services (P1)
  ├── Feature 1: Core AI Service Manager & Provider Infrastructure
  ├── Feature 2: Color Palette Generator & Synthesizer
  ├── Feature 3: Image & Texture Generation Pipeline (with In-Sequence Embedding)
  └── Feature 5: Speech-to-Text & Lyric Quantized Alignment Engine

  Phase 2: Sequence Import & Audio Intelligence (P2)
  ├── Feature 4: Structured Model Mapping & 4-Pass LLM Import
  ├── Feature 6: Deep Learning Stem Separation (Vocals / Accompaniment)
  ├── Feature 7: Automated Onset, Beat & Musical Structure Detection
  └── Feature 8: Automated Phoneme & Face State Lip-Sync Synthesizer

  Phase 3: Generative Content & Spatial AI (P3)
  ├── Feature 9: Dynamic Effect Parameter & Preset Synthesizer
  ├── Feature 10: Automated Custom Model & Mesh Generator
  └── Feature 11: Intelligent Controller Auto-Configuration & Channel Allocation

  Phase 4: Copilots, Platform Integration & Local Acceleration (P4)
  ├── Feature 12: In-App Light Show Assistant & Conversational Copilot
  ├── Feature 13: Sequence Quality Audit & Diagnostic Copilot
  ├── Feature 14: System App Intents & Siri / Voice Command Execution
  └── Feature 15: Hardware-Accelerated Local Inference Engine (OpenVINO / CoreML)
+-----------------------------------------------------------------------------------+
```

---

## Category 1: Native In-App C++ AI Enhancements

### Feature 1: Core AI Provider Infrastructure & Service Manager (`ServiceManager`)
* **Location:** `src-core/ai/ServiceManager.h`, `src-core/ai/ServiceManager.cpp`, `src-core/ai/aiBase.h`
* **Architecture & Mechanics:**
  * Application-scoped singleton (`ServiceManager`) managing AI provider lifecycle and capability resolution.
  * Thread-safe credential management integrating OS-native secret backends (`wxSecretStore` on Desktop, iOS Keychain `kSecClassGenericPassword` on iPadOS).
  * Dynamic model enumeration (`FetchModelsAsync`) retrieving live endpoint model lists into property UI pickers.
  * Extensible C ABI plugin loader (`aiPlugin.h` / `xlCreateAIService`) supporting binary plugin extensions (`.dll`, `.dylib`, `.so`).

### Feature 2: Native C++ Palette Generator & Color Synthesizer
* **Location:** `src-core/ai/aiBase.h` (`GenerateColorPalette`), `src-ui-wx/ai/AIColorPaletteDialog.cpp`
* **Architecture & Mechanics:**
  * Prompts formatted with sequence context (song title, artist, user vibe/mood description).
  * Synthesizes an 8-swatch color array mapped to effect channels (`C_BUTTON_Palette1`..`8` and `C_CHECKBOX_Palette1`..`8`).
  * Asynchronous dispatch prevents UI main-thread blocking during cloud API round-trips.

### Feature 3: Image & Texture Generation Pipeline
* **Location:** `src-core/ai/aiBase.h` (`GenerateImage`), `src-ui-wx/ai/AIImageDialog.cpp`
* **Architecture & Mechanics:**
  * Provider routing across Gemini, Apple Intelligence (ImagePlayground), and local OpenVINO GenAI.
  * Post-processing engine featuring:
    1. Automatic near-black background removal (`RemoveBlackBackground`) with flood-fill tolerance.
    2. Interactive viewport cropping and multi-algorithm scaling (Normal, Bilinear, Bicubic, High).
  * In-sequence embedding: Persists generated assets directly inside `.xsq` files as `AIImages/ai_generated_<timestamp>.png`.

### Feature 4: Structured Model Mapping & LLM Sequence Import
* **Location:** `src-core/ai/aiBase.h` (`GenerateModelMapping`), `src-ui-wx/import_export/xLightsImportChannelMapDialog.cpp`
* **Architecture & Mechanics:**
  * **Structured JSON Mapping:** Uses Claude schema parsing passing full target model specs (nodes, submodels, groups, model types).
  * **Four-Pass Fallback Engine:** Multi-stage prompt evaluation for unmatched channels:
    1. Primary Model Matching
    2. Submodel Channel Alignment
    3. Strand Ordering Resolution
    4. Node-Level Mapping Verification

### Feature 5: Natural Language Value Curve Generator
* **Location:** `src-ui-wx/shared/dialogs/ValueCurveDialog.h`, `src-ui-wx/shared/dialogs/ValueCurveDialog.cpp`
* **Architecture & Mechanics:**
  * Natural language prompt input (`TextCtrl_AIPrompt`) directly inside the value curve dialog.
  * Queries `aiType::PROMPT` providers to parse user descriptors (*"Fast exponential ramp up peaking at 90"*) into value curve types (`Exponential Up`, `Logarithmic Up`, `Sine`, `Saw Tooth`, etc.) and parameters (`P1`..`P4`).
  * Automatically sets curve parameters on `ValueCurve` instances and triggers live viewport redraw (`ValueCurvePanel`).

---

## Category 2: Intelligent Audio & Multimodal Processing

### Feature 6: Audio Stem Separation & Demucs ONNX Integration
* **Location:** `src-core/media/AudioDecoder.h`, `src-core/media/AudioDecoder.cpp`, `src-core/media/StemSeparator.cpp`
* **Architecture & Mechanics:**
  * High-performance 4-stem source separation engine using HTDemucs ONNX models (Vocals, Drums, Bass, Other).
  * Cross-platform execution utilizing DirectML / ONNX Runtime on Windows, OpenVINO on Linux, and CoreML on macOS/iOS.
  * Encodes separated stem channels to isolated `.wav` files under `<ShowFolder>/Media/Stems/` for isolated track processing.

### Feature 7: AI Multimodal Vocal & Lyric Alignment
* **Location:** `src-core/lyrics/PhonemeMap.h`, `src-core/lyrics/PhonemeMap.cpp`
* **Architecture & Mechanics:**
  * Local Whisper / Wav2Vec2 forced alignment engine matching vocal PCM audio energy and spectral frames to lyric text.
  * Translates ARPAbet / CMU / IPA phonemes (`AA`, `EH`, `OW`, `UH`, `L`, `M`, `W`, `etc`) directly to xLights 8-state Singing Face visemes (`AI`, `E`, `L`, `M`, `O`, `U`, `W`, `etc`).
  * Includes transition debouncing (`SmoothVisemeTransitions`) to eliminate physical LED display chatter on rapid phonemes.

### Feature 8: Musical Emotion & Dynamics Contour Mapper
* **Location:** `src-core/media/AudioDynamicsMapper.h`, `src-core/media/AudioDynamicsMapper.cpp`
* **Architecture & Mechanics:**
  * Computes normalized RMS energy density, local tempo/BPM scaling, emotional Valence (-1.0 to +1.0) and Arousal (0.0 to 1.0) across audio frames.
  * Exports target brightness curves as serialized xLights `ValueCurve` custom data payloads (`Type=Custom;CustomData=0:0.1|0.5:0.8...`).

---

## Category 3: Spatial AI & Computer Vision

### Feature 9: Camera-Based Auto Prop Mapper (Gray Code Detector)
* **Location:** `src-core/models/CustomModel.h`, `src-core/models/CustomModel.cpp`
* **Architecture & Mechanics:**
  * Generates $2K$ normal and inverted binary Gray Code light patterns ($K = \lceil \log_2(N) \rceil$) to drive physical prop calibration output.
  * Processes camera capture frames, evaluating threshold deltas to decode physical pixel locations $(X_{cam}, Y_{cam})$ into 2D custom model node matrices.

### Feature 10: Automated Submodel & Face Detector (SAM Integration)
* **Location:** `src-core/models/SubModelOps.h`, `src-core/models/SubModelOps.cpp`
* **Architecture & Mechanics:**
  * Segment Anything Model (SAM) integration (`DetectSubmodelsWithSAM`) auto-segmenting prop node grids into structural submodels (Rings, Spokes, Clusters).
  * Automatically detects and generates Singing Face component submodels (`Outline`, `Eyes Open`, `Eyes Closed`, and 8 `Mouth` viseme states).

### Feature 11: AI Power Injection & Load Balancer
* **Location:** `src-core/controllers/PowerInjectionAnalyzer.h`, `src-core/controllers/PowerInjectionAnalyzer.cpp`
* **Architecture & Mechanics:**
  * Ohm's Law voltage drop calculator evaluating AWG wire sizes ($14\,\text{AWG}$ to $24\,\text{AWG}$), lead wire lengths, and pixel spacing across 5V, 12V, and 24V DC strings.
  * Identifies brownout node locations ($<80\%$ nominal voltage) and recommends exact power injection tap positions (e.g. Pixel #100, Pixel #200).

---

## Category 4: AI Sequencing & Creative Assistants

### Feature 12: Natural Language Macro / Script Generator
* **Location:** `src-core/scripting/LuaManager.h`, `src-core/scripting/LuaManager.cpp`
* **Architecture & Mechanics:**
  * Prompts LLM services (`aiType::PROMPT`) with official xLights Lua API schema (`xlights.create_effect`, `xlights.set_palette`, `xlights.apply_preset`, `xlights.render_sequence`).
  * Converts natural language requests into executable sandboxed Lua automation scripts.

### Feature 13: In-App Light Show Assistant & Conversational Copilot
* **Location:** `src-core/ai/SequenceDiagnosticsCopilot.h`, `src-core/ai/SequenceDiagnosticsCopilot.cpp`
* **Architecture & Mechanics:**
  * Context-aware prompt manager reading active sequence parameters, selected models, and controller configs.
  * Formulates real-time design recommendations, palette selections, and action shortcuts (`QueryAssistantCopilot`).

### Feature 14: Sequence Quality Audit & Error Diagnostic Copilot
* **Location:** `src-core/ai/SequenceDiagnosticsCopilot.h`, `src-core/ai/SequenceDiagnosticsCopilot.cpp`
* **Architecture & Mechanics:**
  * Automated static analyzer (`AuditSequence`) scanning sequence data for unrendered buffers, channel overlaps, and orphaned media assets.
  * Provides automated cleanup functionality (`RemoveUnusedMedia`) to delete unused files from show directories.

---

## Category 5: External Tools & Ecosystem

### Feature 15: 3D Photorealistic Visualizer Generator (ControlNet / SD Renderer)
* **Location:** `src-core/media/PhotorealisticVisualizerRenderer.h`, `src-core/media/PhotorealisticVisualizerRenderer.cpp`
* **Architecture & Mechanics:**
  * Blends `.fseq` sequence channel data with 3D house preview photos into ControlNet line-art / depth map conditioning frames.
  * Runs Stable Diffusion + ControlNet image generation pipeline (OpenVINO / CoreML / Cloud) to render photorealistic preview frames.
  * Encodes photorealistic frames and muxes original sequence audio into high-definition `.mp4` video output files (`VideoWriter`).

### Feature 16: xLights Model Context Protocol (MCP) Server
* **Location:** `tools/mcp_server/server.py`, `tools/mcp_server/index.js`, `tools/mcp_server/package.json`
* **Architecture & Mechanics:**
  * Exposes JSON-RPC 2.0 stdio MCP server tools for external AI agents (Claude, Gemini, Antigravity).
  * Interfacing directly with xLights REST API running on `http://localhost:49913` for version info, model lists, sequence info, rendering, playback, and effect placement.

### Feature 17: Real-Time Voice-Controlled Layout Tester
* **Location:** `src-core/outputs/OutputManager.h`, `src-core/outputs/OutputManager.cpp`
* **Architecture & Mechanics:**
  * Real-time speech-to-packet pipeline using Whisper-Tiny voice recognition (`ProcessVoiceTestCommand`).
  * Direct hardware frame transmission (`SendDirectVoiceTestPacket`) bypassing UI dialogs to stream channel packets over E1.31 / DDP / ZMQ output threads.

### Feature 18: Cross-Display Sequence Mapping Agent
* **Location:** `src-core/ai/SequenceRemappingAgent.h`, `src-core/ai/SequenceRemappingAgent.cpp`
* **Architecture & Mechanics:**
  * Computes 16-dimensional dense vector embeddings (`ComputeModelEmbedding`) for sequence models based on spatial position, type, node density, and strand count.
  * Matches models across different physical displays using cosine similarity vector matching (`RemapSequenceModels`) and applies remapping to `.xsq` sequence XML payloads (`ApplyRemappingToSequenceXML`).

### Feature 19: Automated Show Diagnostics & Log Analyzer
* **Location:** `src-core/ai/ShowLogDiagnosticsAnalyzer.h`, `src-core/ai/ShowLogDiagnosticsAnalyzer.cpp`
* **Architecture & Mechanics:**
  * Scans `spdlog` log files (`AnalyzeSpdlogFile`) to diagnose audio decoder errors, network socket timeouts, and memory overflows.
  * Inspects show folder XML files (`InspectShowDirectoryXML`) using `pugixml` to detect syntax corruptions and missing attributes before show execution.

---

## Security, Privacy & Credential Management

1. **Zero Secret Leakage:** Credentials are stored exclusively in OS secure stores (`wxSecretStore` / iOS Keychain).
2. **Transmission Disclosure:** Clear UI indicators notify users when data is sent to cloud APIs.
3. **Offline Privacy Guarantee:** Privacy Mode restricts AI feature execution strictly to local backends.

---

## Verification & Test Plan

* **Automated Tests:** Unit tests in `src-core/ai/tests/` validating `ServiceManager`, palette parsing, structured mapping, lyric quantization, and vector embeddings.
* **Manual Verification:** Verification checklist covering API key persistence, 8-color palette application, image embedding, model import mapping, voice testing, and photorealistic video rendering.
