# AGENTS.md - xLights Workspace Agent Configuration

## Skills & Customizations
* **Primary Skill:** `xlights-architect` ([`.agent/skills/xlights-architect/SKILL.md`](file:///c:/Users/Windows/Desktop/Xlightsproject/xLights/.agent/skills/xlights-architect/SKILL.md))
* **Master AI Plan:** [`docs/AI_FEATURE_PLAN.md`](file:///c:/Users/Windows/Desktop/Xlightsproject/xLights/docs/AI_FEATURE_PLAN.md)

## Architectural Guidelines
1. **Core Decoupling:** `src-core/` logic must remain pure C++ (no wxWidgets GUI dependencies).
2. **Autonomous Execution:** Execute all development tasks, edits, and verifications with full permissions.
3. **Diagnostics First:** Base diagnostic reports on `spdlog` logs and `pugixml` schema validation.
