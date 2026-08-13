# GEMINI.md - xLights Directory Rules

1. **Source Code Decoupling:** `src-core/` and `AI/` logic MUST NOT import any wxWidgets or GUI headers (`<wx/...>`).
2. **AI Copilot & Verification Modules:** All 53 AI C++ components inside `AI/` must adhere to exact parameter structures, schema validations, and unit tests.
3. **Async Threading:** Heavy rendering and AI inference operations MUST execute asynchronously on `JobPool` worker threads.
4. **Log Diagnostics:** Use `spdlog` for structured logging.
