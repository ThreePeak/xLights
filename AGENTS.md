# AGENTS.md - xLights Workspace Agent Configuration

Welcome to the **xLights Repository**. This workspace contains the core xLights C++ engine, desktop UI, AI subsystem, and platform build scripts.

## Registered Skills

* **`xlights-architect`** ([`.agents/skills/xlights-architect/SKILL.md`](file:///c:/Users/Windows/Desktop/Xlightsproject/.agents/skills/xlights-architect/SKILL.md))
* **`xlights-ai-verifier`** ([`.agents/skills/xlights-ai-verifier/SKILL.md`](file:///c:/Users/Windows/Desktop/Xlightsproject/.agents/skills/xlights-ai-verifier/SKILL.md))
* **`xlights-build-and-test`** ([`.agents/skills/xlights-build-and-test/SKILL.md`](file:///c:/Users/Windows/Desktop/Xlightsproject/.agents/skills/xlights-build-and-test/SKILL.md))

## Master Documentation Links

* **Project Manifest:** [`PROJECT_MANIFEST.md`](file:///c:/Users/Windows/Desktop/Xlightsproject/PROJECT_MANIFEST.md)
* **Master AI Plan:** [`docs/AI_FEATURE_PLAN.md`](file:///c:/Users/Windows/Desktop/Xlightsproject/xLights/docs/AI_FEATURE_PLAN.md)
* **AI Subsystem Verification:** [`AI/run_ai_subsystem_verification.py`](file:///c:/Users/Windows/Desktop/Xlightsproject/xLights/AI/run_ai_subsystem_verification.py)

## Architectural Guidelines

1. **Core Decoupling:** `src-core/` and `AI/` logic must remain pure C++ (no wxWidgets GUI headers).
2. **Autonomous Execution:** Execute all development tasks, edits, and verifications with full permissions.
3. **Diagnostics First:** Base diagnostic reports on `spdlog` logs and `pugixml` schema validation.
4. **Verification Requirement:** Run `python AI/run_ai_subsystem_verification.py` after editing AI components.
