# xLights Model Context Protocol (MCP) Server

The **xLights MCP Server** interfaces external AI agents (Claude, Gemini, Antigravity) with the xLights REST API running on port `49913`.

## Features & Tools

| Tool Name | Description | REST Endpoint |
| :--- | :--- | :--- |
| `xlights_get_version` | Retrieves xLights version and REST API status | `/api/version` |
| `xlights_get_models` | Lists all physical models in active show layout | `/api/models` |
| `xlights_get_sequence_info` | Retrieves active sequence details | `/api/sequence` |
| `xlights_render_sequence` | Triggers full sequence rendering | `/api/render` |
| `xlights_play_sequence` | Starts real-time preview playback | `/api/play` |
| `xlights_stop_sequence` | Stops sequence playback | `/api/stop` |
| `xlights_set_effect` | Places an effect stack onto a target model | `/api/effect` |
| `xlights_generate_value_curve` | Synthesizes natural language curve prompts | `/api/ai/value_curve` |

## Stdio Quickstart

### Node.js
```bash
node index.js
```

### Python
```bash
python server.py
```
