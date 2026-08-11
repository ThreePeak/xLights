# xLights Agent Rules & Skill Configuration

## Core Architectural Mandates

1. **Strict Decoupling (`src-core/` vs `src-ui-wx/`):**
   * All core C++ engines (AI services, audio decoders, render buffers, custom model math, controller uploaders, and Lua scripting) MUST reside in `src-core/`.
   * Files in `src-core/` MUST NOT include any wxWidgets GUI header (`<wx/...>`).

2. **Skill Activation (`xlights-architect`):**
   * All subagents spawned for xLights development MUST read `.agent/skills/xlights-architect/SKILL.md` before initiating code changes.

3. **Autonomous Execution:**
   * Execute code edits, file creations, and verification commands directly without halting for manual user confirmation.

4. **Empirical Diagnostics:**
   * Base bug fixes and diagnostic reports strictly on empirical log evidence extracted from `spdlog` log files or `pugixml` show directory inspects.

5. **API & Contract Preservation:**
   * Preserve all existing function signatures, comments, and serialization structures when extending classes in `src-core/`.
