# xLights Model Context Protocol (MCP) Server

The **xLights MCP Server** interfaces external AI agents (Claude, Gemini, Antigravity) directly with the xLights REST API running on port `49913`.

---

## Capabilities & Tool Reference

| Tool Name | Method & Endpoint | Description | Arguments / Inputs |
| :--- | :--- | :--- | :--- |
| `xlights_get_version` | GET `/api/version` | Gets active xLights version & REST health status | None |
| `get_show_layout` | GET `/api/layout` | Queries active display models, submodels, pixel counts, & channel bounds | None |
| `xlights_get_models` | GET `/api/models` | Lists all physical models in the active show layout | None |
| `xlights_get_sequence_info` | GET `/api/sequence` | Retrieves sequence duration, frame rate, & audio file path | None |
| `xlights_get_show_folder` | GET `/api/showfolder` | Returns active xLights show directory path | None |
| `xlights_get_controllers` | GET `/api/controllers` | Lists pixel controllers (FPP, Falcon, Kulp) & IP addresses | None |
| `xlights_get_effect_presets` | GET `/api/presets` | Lists custom multi-layer effect presets saved in show directory | None |
| `xlights_render_sequence` | POST `/api/render` | Triggers background parallel re-rendering | None |
| `xlights_play_sequence` | POST `/api/play` | Starts real-time sequence playback in xLights preview | None |
| `xlights_stop_sequence` | POST `/api/stop` | Stops active sequence playback | None |
| `xlights_set_effect` | POST `/api/effect` | Places an effect stack onto a target display model | `model_name`, `effect_name`, `start_ms`, `end_ms`, `palette` |
| `xlights_generate_value_curve` | POST `/api/ai/value_curve` | Synthesizes natural language curve prompts into JSON & pipe strings | `prompt`, `min`, `max` |

---

## Quickstart & Execution

### 1. TypeScript / Node.js Runner
```bash
cd tools/xlights-mcp-server
npm install
npm run build
npm start
```

### 2. Python Runner
```bash
cd tools/xlights-mcp-server
python server.py
```

---

## Stdio Test Invocation

Test JSON-RPC 2.0 tool discovery over `stdio`:

```bash
echo '{"jsonrpc": "2.0", "id": 1, "method": "tools/list"}' | node dist/index.js
```

Test tool execution over `stdio`:

```bash
echo '{"jsonrpc": "2.0", "id": 2, "method": "tools/call", "params": {"name": "get_show_layout", "arguments": {}}}' | node dist/index.js
```

---

## Client Integration Configuration

Add the following to your MCP client configuration (`claude_desktop_config.json` or `mcp_config.json`):

```json
{
  "mcpServers": {
    "xlights": {
      "command": "node",
      "args": ["c:/Users/Windows/Desktop/Xlightsproject/xLights/tools/xlights-mcp-server/dist/index.js"]
    }
  }
}
```
