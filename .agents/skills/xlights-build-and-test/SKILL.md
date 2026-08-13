---
name: xlights-build-and-test
description: Runbook for building xLights C++ core, desktop UI, AI subsystem tests, and executing verification scripts.
---

# xLights Build & Test Skill

## Overview

This skill provides step-by-step instructions for building xLights, compiling C++ modules using CMake/MSVC, and executing automated test suites.

---

## 1. CMake & MSVC Build Environment

On Windows with Visual Studio 2022 / vcpkg:

```powershell
# Set up vcpkg dependencies and CMake build directory
cmake -B build -S xLights -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Or execute the provided batch script for VS2022 vcpkg:
```powershell
xLights/VS2022_vcpkg.bat
```

---

## 2. Running Automated Verifications

To run the lightweight AI subsystem Python test suite:

```powershell
python xLights/AI/run_ai_subsystem_verification.py
```

Expected output:
```text
==================================================
   ALL XLIGHTS AI SUBSYSTEM TESTS PASSED (4/4)    
==================================================
```

---

## 3. Core Engine Decoupling Verification

When modifying files in `src-core/` or `AI/`, verify that no wxWidgets header dependencies have been introduced:

```powershell
# Grep search for prohibited GUI inclusions in core/AI code
grep -r "#include <wx/" xLights/src-core xLights/AI
```
If any match is returned, remove the GUI dependency immediately.
