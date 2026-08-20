# xLights AI Master Feature Backlog & Ideas Repository

This master repository stores all brainstormed, proposed, deferred, and future-concept features for xLights. It acts as the perpetual innovation backlog, ensuring that no ideas, technical architectures, or UX specifications are lost across project milestones.

---

## 1. Deferred Core Roadmap Features

### Feature 17: Real-Time Voice-Controlled Layout & Hardware Tester (`VoiceLayoutTester`)

- **Status:** Deferred by user decision (reserved for a future dedicated update).
- **Core Concept:** Hands-free speech-to-packet recognition allowing users walking around their physical light display to issue spoken commands (e.g., *"Turn MegaTree Red at 50%"*, *"Test Arch 3 white strobe"*, *"All rooflines off"*).
- **Technical Architecture:**
  - Embedded Whisper-Tiny / local voice-command grammar parsing.
  - Generates direct DDP / E1.31 / ZMQ UDP frame payloads bypassing modal UI dialogs.
  - Sub-50ms latency audio capture loop to physical pixel output.

---

## 2. Active Next-Generation AI Features (Features 20–29)

### Feature 20: 3D Spatial Audio & Multi-Speaker Outdoor Panner (`SpatialAudioPannerAI`)

- **Status:** Backlog / Planned.
- **Core Concept:** Dynamically pans 3D audio across outdoor multi-speaker setups synchronized with lighting prop animations (e.g., sound swooshing across the lawn from left tree to right tree as lights travel).
- **Architecture:** Multi-stem spectral localization, HRTF/binaural mapping, and OSC/Dante/ASIO multichannel audio router.

### Feature 21: Neural Effect Harmonizer & Musical Rhythm Quantizer (`EffectHarmonizerAI`)

- **Status:** Backlog / Planned.
- **Core Concept:** Synchronizes multi-track lighting layers by snapping effect starts/ends, speed multipliers, and color fades to musical subdivisions (1/8, 1/16, triplets) across all active display models.
- **Architecture:** Spectral onset beat tracker, harmonic key detector, and automatic effect layer timing optimizer.

### Feature 22: Drone Light Show Swarm Choreographer & Trajectory Solver (`DroneSwarmChoreographerAI`)

- **Status:** Backlog / Planned.
- **Core Concept:** Transforms 3D xLights matrix/mesh effects and animations into collision-free 3D drone trajectory waypoints, geofences, and velocity/acceleration limits for hybrid drone + Christmas light displays.
- **Architecture:** 3D flocking algorithm, spatial collision avoidance, and mission export to `.csv`, MavLink, and PX4 mission formats.

### Feature 26: Moving-Head Robotic Beam Path Tracer (`MovingHeadBeamTracerAI`)

- **Status:** Backlog / Planned.
- **Core Concept:** AI motion-path solver for robotic moving-head spot/beam fixtures, generating cinematic pan/tilt sweep paths and gobo rotations synchronized with musical climax points.
- **Architecture:** DMX channel curve interpolator, mechanical slew-rate limiter, and inverse kinematics beam target solver.

### Feature 27: Multi-Show Playlist AI DJ & Intelligent Crossfader (`PlaylistSmartFaderAI`)

- **Status:** Backlog / Planned.
- **Core Concept:** Seamlessly blends consecutive songs in a continuous holiday show playlist using Camelot wheel harmonic key matching, BPM tempo matching, and ambient interlude filler lighting transitions.
- **Architecture:** Song transition key-compatibility matrix, dynamic crossfade curve generator, and automated playlist sequence bridge rendering.

### Feature 28: Live Ambient Weather & Precipitation Reactor (`WeatherAdaptiveShowAI`)

- **Status:** Backlog / Planned.
- **Core Concept:** Connects to local weather station APIs or computer vision outdoor cameras to dynamically modulate show brightness, color temperatures, and effect dynamics during snowstorms, fog, or heavy rain.
- **Architecture:** Weather API client (OpenWeather/Weather Underground), ambient luminance camera parser, and global render-time master brightness/saturation modifier.

### Feature 29: AI Projection Mapping Surface Calibrator (`ProjectionMappingCalibratorAI`)

- **Status:** Backlog / Planned.
- **Core Concept:** Automatic corner-pinning, keystone correction, and geometric mesh warping for architectural house projection mapping using structured light test patterns and camera feedback.
- **Architecture:** Homography transformation solver, OpenCV structured light decoder, and OpenGL/Vulkan projection warp mesh exporter.

---

## 3. High-Value AI Subsystem Extensions

### Feature 31: Predictive Electrical Power Sag & Voltage Drop Simulator (`PowerSagSimulatorAI`)

- **Status:** **PARTIALLY IMPLEMENTED** (Core DC voltage drop analysis, AWG resistance models, and injection point calculations completed in `PowerInjectionAnalyzer` & `PowerInjectionAIGenerator`).
- **Remaining Backlog Scope:**
  - Dynamic transient current surge simulation during full-white strobe bursts.
  - Secondary power supply capacitor sizing advisor.
  - Real-time simulation of high-inductance long DC wire runs.

### Feature 35: Interactive Spectator Jukebox & Crowd Voting Server (`SpectatorJukeboxAI`)

- **Status:** Backlog / Planned.
- **Core Concept:** Self-hosted lightweight mobile web portal (WebSockets / QR code) allowing spectators parked in front of the house to vote on upcoming songs, trigger interactive light bursts, and submit moderated matrix shoutouts.
- **Architecture:** Embedded C++ micro-HTTP/WebSocket server, LLM profanity/safety filter for matrix text shoutouts, and playlist queue insertion engine.

### Feature 38: Automatic Show Electrical Breaker & Power Phase Load Balancer (`PhaseLoadBalancerAI`)

- **Status:** Backlog / Planned (Builds upon `PowerInjectionAnalyzer` and `ThermalSafetyThrottlerAI`).
- **Core Concept:** Evaluates 120/240V split-phase and 3-phase electrical panel distributions across multiple controllers, balancing load across legs to prevent main circuit breaker trips.
- **Architecture:** Phase load distribution matrix, DMX/E1.31 universe-to-phase remapping suggestions, and peak simultaneous amperage simulator.

---

## 4. Brainstormed & Extended Innovation Concepts

1. **AI Color Harmonics & Contrast Balancer:**
   - **Status:** **PARTIALLY IMPLEMENTED** (Audio key signature and emotional mood palette generation completed in `MusicalSynesthesiaEngine`).
   - **Remaining Scope:** Dynamic sequence-wide palette correction to prevent color fatigue and physical LED chip gamut mapping (WS2811, GS8208, SK6812).
2. **Auto-Wiring & Physical Cabling Optimizer:**
   - **Status:** **PARTIALLY IMPLEMENTED** (DMX address collision detection in `DMXAddressAdvisor` and FPP layout provisioning in `FPPControllerSyncAdvisor`).
   - **Remaining Scope:** 2D/3D yard terrain pathfinding for physical wire routing, controller enclosure location recommendations, and minimum-cable-length bill of materials.
3. **Show Power Cost & Energy Estimator:**
   - **Status:** Backlog / Planned (Leverages sequence duty-cycle data from `ThermalSafetyThrottlerAI`).
   - **Scope:** Real-time kWh electricity cost estimator based on sequence duty cycle, power supply efficiency curves, and local utility tiered rate tables.
4. **AI Submodel Symmetry & Mirror Tool:**
   - **Status:** Backlog / Planned (Builds on `SubmodelDetector` and `PropModelEditEngine`).
   - **Scope:** Automatically mirrors, rotates, and symmetrizes complex custom submodel layouts across 2D/3D axes with single-click auto-indexing.

---

## 5. Completed & Integrated AI Features Reference Index

The following features have been fully designed, implemented, tested, and integrated into the xLights codebase:

| Feature / Subsystem | Core Engine Class (`src-core/` / `AI/`) | Desktop UI (`src-ui-wx/ai/`) | Status |
| :--- | :--- | :--- | :--- |
| **Feature 1: Provider Infrastructure** | `ServiceManager`, `AIEngineConfigManager` | `AIEngineSettingsDialog`, `AIInferenceSettingsPanel` | **Shipped** |
| **Feature 2: Color Palette Generator** | `aiBase.h` (`GenerateColorPalette`) | `AIColorPaletteDialog` | **Shipped** |
| **Feature 3: Image & Texture Pipeline** | `OpenAIImageGenerator`, `OpenVINOImageGenerator` | `AIImageDialog` | **Shipped** |
| **Feature 4: Model Mapping & Remapping** | `ModelMappingAIGenerator`, `SequenceRemappingAgent` | `AIModelMappingWizard`, `AISequenceRemapDialog` | **Shipped** |
| **Feature 5: Natural Language Value Curve** | `ValueCurveAIGenerator` | `ValueCurveDialog` | **Shipped** |
| **Feature 6: Deep Learning Stem Separation** | `AudioStemExtractor` (Demucs ONNX) | `AIAudioStemExtractorDialog` | **Shipped** |
| **Feature 7: Vocal & Lyric Alignment** | `PhonemeMap` (Whisper / 8 Visemes) | `AILyricVisemeAlignerDialog` | **Shipped** |
| **Feature 8: Dynamics Contour Mapper** | `DynamicsContourMapper` | `ValueCurveDialog` integration | **Shipped** |
| **Feature 9: Auto Prop Mapper / Gray Code** | `AutoPropMapper`, `GrayCodePixelMapper` | `AIGrayCodePixelMapperDialog` | **Shipped** |
| **Feature 10: Submodel & Face Detector** | `SubmodelDetector`, `SubmodelDetectorAIGenerator` | `AISubmodelDetectorDialog` | **Shipped** |
| **Feature 11: Power Injection & Sizing** | `PowerInjectionAnalyzer`, `PowerInjectionAIGenerator` | `AIPowerInjectionDialog` | **Shipped** |
| **Feature 12: Natural Language Lua Scripts** | `LuaScriptGenerator`, `LuaScriptAIGenerator` | `AILuaScriptDialog` | **Shipped** |
| **Feature 13 & 14: Diagnostics Copilot** | `SequenceDiagnosticsCopilot`, `SequenceValidatorAI` | `AIShowDiagnosticsDialog`, `AISequenceValidatorDialog` | **Shipped** |
| **Feature 15: Local Inference Engine** | `LocalInferenceEngine` (OpenVINO / ONNX) | `AIInferenceSettingsPanel` | **Shipped** |
| **Feature 16: Custom Prop Designer AI** | `CustomPropDesignerAI`, `PropModelEditEngine` | `AICustomPropDesignerDialog` | **Shipped** |
| **Feature 18: Sequence Remapping Agent** | `SequenceRemappingAgent` (16-D Cosine Sim) | `AISequenceRemapDialog` | **Shipped** |
| **Feature 19: Photorealistic Show Visualizer** | `PhotorealisticVisualizerRenderer` | `AIPhotorealisticPreviewDialog` | **Shipped** |
| **Feature 23: Thermal & Current Throttler** | `ThermalSafetyThrottlerAI` | `AIThermalSafetyThrottlerDialog` | **Shipped** |
| **Feature 24: Photo 3D Prop Reconstructor** | `Photo3DPropReconstructorAI` | `AIPhoto3DPropReconstructorDialog` | **Shipped** |
| **Feature 25: Audience Sightline Optimizer** | `AudienceViewingOptimizerAI` | `AIAudienceViewingOptimizerDialog` | **Shipped** |
| **Feature 30: Show Narrative Composer** | `ShowNarrativeComposerAI` | `AIShowNarrativeComposerDialog` | **Shipped** |
| **Feature 33: Dead Pixel Auto-Healer** | `PixelAutoHealingAI` | `AIPixelAutoHealingDialog` | **Shipped** |
| **Feature 36: Neural Shader Synthesizer** | `NeuralShaderSynthesizerAI` | `AINeuralShaderSynthesizerDialog` | **Shipped** |
| **Feature 37: VR/AR Spatial Walkthrough** | `VRShowSpatialCopilotAI`, `AtmosphericVRPreviewer` | `AIVRShowSpatialCopilotDialog` | **Shipped** |
| **Feature 39: Sequence Visual Git & Diff** | `SequenceVisualGitAI` | `AISequenceVisualGitDialog` | **Shipped** |
| **Feature 40: Audio Stem Choreographer** | `AudioChoreographerAI` | `AIAudioChoreographerDialog` | **Shipped** |
| **Snapshot History Engine** | `AISnapshotHistoryManager` | `AISnapshotHistoryDialog` | **Shipped** |
