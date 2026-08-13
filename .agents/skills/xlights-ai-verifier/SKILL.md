---
name: xlights-ai-verifier
description: Runbook for verifying xLights AI subsystems, running automated verification tests, XML schema bounds checking, and downsampling evaluation.
---

# xLights AI Subsystem Verifier Skill

## Overview

Use this skill when you need to verify, test, or validate the **xLights AI Subsystem** modules located in `xLights/AI/`.

The AI subsystem consists of 53 C++ header, source, and unit test files, along with an automated Python test harness `run_ai_subsystem_verification.py`.

---

## 1. Running the Automated Verification Suite

Run the Python verification test runner from the workspace root or `xLights/` directory:

```powershell
python xLights/AI/run_ai_subsystem_verification.py
```

### Verified Test Categories:
1. **SubmodelDetector AI Category Classification:** Verifies the 7 SAM vision categories (`OUTER_PERIMETER`, `CONCENTRIC_RING`, `RADIAL_SPOKE`, `SINGING_FACE_OUTLINE`, `SINGING_EYE`, `SINGING_MOUTH`, `CUSTOM_CLUSTER`).
2. **DynamicsContourMapper Audio Downsampling:** Tests RMS audio frame downsampling (50ms granularity) to 5 Bezier control point anchors preserving crescendo/decrescendo contours.
3. **Submodel XML Schema Bounds Validation:** Validates XML structure parsing via `pugixml` regex matching for submodel ranges and attributes.
4. **LayerBlendAdvisor Rule Engine:** Validates the 4 blending rule evaluations (Muddying Rule, Spatial Occlusion, Strobe Overpowering, Transition Smoothness).

---

## 2. Running C++ AI Subsystem Unit Tests

To compile and run C++ unit test executables in `xLights/AI/`:
1. Check CMake build target `xLights_AI_Tests`.
2. Execute individual test binaries (e.g., `test_AudioStemExtractor`, `test_AutoPropMapper`, `test_SequenceValidatorAI`).
