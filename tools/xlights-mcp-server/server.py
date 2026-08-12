#!/usr/bin/env python3
"""
xLights Model Context Protocol (MCP) Server
Interfaces external AI agents with xLights REST API running on port 49913.
"""

import sys
import json
import urllib.request
import urllib.parse

XLIGHTS_REST_URL = "http://localhost:49913"

def http_get(endpoint: str):
    url = f"{XLIGHTS_REST_URL}{endpoint}"
    req = urllib.request.Request(url, headers={"User-Agent": "xLights-MCP-Server/1.0"})
    try:
        with urllib.request.urlopen(req, timeout=5) as response:
            data = response.read().decode('utf-8')
            return json.loads(data)
    except Exception as e:
        return {"status": "error", "message": f"xLights REST API connection failed at {url}: {str(e)}"}

def http_post(endpoint: str, payload: dict):
    url = f"{XLIGHTS_REST_URL}{endpoint}"
    data_bytes = json.dumps(payload).encode('utf-8')
    req = urllib.request.Request(url, data=data_bytes, headers={"Content-Type": "application/json", "User-Agent": "xLights-MCP-Server/1.0"}, method="POST")
    try:
        with urllib.request.urlopen(req, timeout=5) as response:
            data = response.read().decode('utf-8')
            return json.loads(data)
    except Exception as e:
        return {"status": "error", "message": f"xLights REST API POST failed at {url}: {str(e)}"}

# MCP Tools Definition
MCP_TOOLS = [
    {
        "name": "xlights_get_version",
        "description": "Get xLights version and REST API status on port 49913",
        "inputSchema": {"type": "object", "properties": {}}
    },
    {
        "name": "get_show_layout",
        "description": "Queries active models, submodels, pixel counts, and channel bounds from xLights REST API",
        "inputSchema": {"type": "object", "properties": {}}
    },
    {
        "name": "xlights_get_models",
        "description": "List all physical models in the active xLights show layout",
        "inputSchema": {"type": "object", "properties": {}}
    },
    {
        "name": "get_sequence_state",
        "description": "Fetches active sequence media file path, total duration in milliseconds, frame rate, and track structures",
        "inputSchema": {"type": "object", "properties": {}}
    },
    {
        "name": "xlights_get_sequence_info",
        "description": "Get active sequence details including duration, frame rate, and media file",
        "inputSchema": {"type": "object", "properties": {}}
    },
    {
        "name": "trigger_render",
        "description": "Sends a render command payload to xLights REST API port 49913 to trigger sequence rendering",
        "inputSchema": {
            "type": "object",
            "properties": {
                "model_name": {"type": "string", "description": "Optional specific model name to render, or empty for full sequence render"}
            }
        }
    },
    {
        "name": "xlights_render_sequence",
        "description": "Trigger full sequence re-rendering across all models",
        "inputSchema": {"type": "object", "properties": {}}
    },
    {
        "name": "xlights_play_sequence",
        "description": "Start real-time sequence playback in xLights preview",
        "inputSchema": {"type": "object", "properties": {}}
    },
    {
        "name": "xlights_stop_sequence",
        "description": "Stop sequence playback",
        "inputSchema": {"type": "object", "properties": {}}
    },
    {
        "name": "insert_effect",
        "description": "Pushes effect parameters (target model, effect name, start/end timing, color palette) to active timeline tracks",
        "inputSchema": {
            "type": "object",
            "properties": {
                "model_name": {"type": "string", "description": "Target display model name e.g. MegaTree, House Outline"},
                "effect_name": {"type": "string", "description": "Effect type e.g. SingleStrand, ColorWash, Twinkle, Bars, Fire, Butterfly"},
                "start_ms": {"type": "integer", "description": "Start time in milliseconds"},
                "end_ms": {"type": "integer", "description": "End time in milliseconds"},
                "palette": {"type": "array", "items": {"type": "string"}, "description": "Color hex strings array e.g. ['#FF0000', '#00FF00']"},
                "settings": {"type": "object", "description": "Optional effect settings key-value object"}
            },
            "required": ["model_name", "effect_name", "start_ms", "end_ms"]
        }
    },
    {
        "name": "separate_audio_stems",
        "description": "Splits song into isolated stems (Vocals/Drums/Bass) and generates transient timing tracks.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "model_path": {"type": "string", "description": "Path to ONNX or CoreML HTDemucs model file"},
                "output_directory": {"type": "string", "description": "Directory to save exported stem WAV files"},
                "transient_sensitivity": {"type": "number", "description": "Transient onset sensitivity threshold (default 0.12)"}
            }
        }
    },
    {
        "name": "xlights_extract_audio_stems",
        "description": "Executes deep learning HTDemucs source separation on active sequence audio to isolate Vocals, Drums, Bass, and Other stems",
        "inputSchema": {
            "type": "object",
            "properties": {
                "model_path": {"type": "string", "description": "Path to ONNX or CoreML HTDemucs model file"},
                "output_directory": {"type": "string", "description": "Directory to save exported stem WAV files"},
                "transient_sensitivity": {"type": "number", "description": "Transient onset sensitivity threshold (default 0.12)"}
            }
        }
    },
    {
        "name": "xlights_import_xtiming",
        "description": "Imports a compiled .xtiming XML file or payload into active xLights sequence timing tracks",
        "inputSchema": {
            "type": "object",
            "properties": {
                "timing_name": {"type": "string", "description": "Timing track name e.g. 'AI Stems - Drums & Onsets'"},
                "xtiming_xml": {"type": "string", "description": "Raw .xtiming XML content payload"},
                "file_path": {"type": "string", "description": "Path to .xtiming XML file"}
            },
            "required": ["timing_name"]
        }
    },
    {
        "name": "xlights_recommend_layer_blend",
        "description": "Evaluates multi-layer effect stacks and recommends optimal blend modes (Additive, Layered, Mask) and transition durations",
        "inputSchema": {
            "type": "object",
            "properties": {
                "top_effect": {"type": "string", "description": "Top effect type e.g. Twinkle, Meteors, Strobe"},
                "bottom_effect": {"type": "string", "description": "Bottom effect type e.g. ColorWash, Fire, Wave"}
            },
            "required": ["top_effect", "bottom_effect"]
        }
    },
    {
        "name": "xlights_set_effect",
        "description": "Place an effect onto a target model in the active sequence",
        "inputSchema": {
            "type": "object",
            "properties": {
                "model_name": {"type": "string", "description": "Target model name"},
                "effect_name": {"type": "string", "description": "Effect name e.g. SingleStrand, ColorWash, Twinkle"},
                "start_ms": {"type": "integer", "description": "Start time in milliseconds"},
                "end_ms": {"type": "integer", "description": "End time in milliseconds"},
                "palette": {"type": "array", "items": {"type": "string"}, "description": "Color hex strings"}
            },
            "required": ["model_name", "effect_name", "start_ms", "end_ms"]
        }
    },
    {
        "name": "xlights_generate_value_curve",
        "description": "Synthesize a natural language prompt into a normalized Bezier value curve JSON array and xLights pipe string",
        "inputSchema": {
            "type": "object",
            "properties": {
                "prompt": {"type": "string", "description": "Natural language curve descriptor e.g. 'Exponential ramp up peaking at 90'"},
                "min": {"type": "number", "description": "Minimum curve bound"},
                "max": {"type": "number", "description": "Maximum curve bound"}
            },
            "required": ["prompt"]
        }
    },
    {
        "name": "map_audio_dynamics",
        "description": "Extracts song volume envelopes, spectral energy curves, valence, arousal, and harmonic tension into xLights ValueCurves.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "metric": {"type": "string", "description": "Target metric e.g. brightness, valence, arousal, tension"},
                "frame_period_ms": {"type": "number", "description": "Frame period in milliseconds (default 50)"}
            }
        }
    },
    {
        "name": "extract_audio_volume_envelope",
        "description": "Extracts song volume envelopes and spectral energy curves into xLights ValueCurves.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "stem_type": {"type": "string", "description": "Target stem e.g. Vocals, Drums, Bass, Full"},
                "frame_period_ms": {"type": "number", "description": "Frame period in milliseconds (default 50)"}
            }
        }
    },
    {
        "name": "analyze_audio_dynamics",
        "description": "Analyze full song audio track into 50ms STFT frames computing RMS Energy, Spectral Centroid, Valence, Arousal, and Harmonic Tension as xLights ValueCurve data.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "audio_file_path": {"type": "string", "description": "Absolute path to the audio file to analyze"},
                "sample_interval_ms": {"type": "number", "description": "STFT frame window in milliseconds (default 50)"},
                "metric": {"type": "string", "description": "Target metric: brightness, valence, arousal, tension, spectralCentroid"}
            },
            "required": ["audio_file_path"]
        }
    },
    {
        "name": "gray_code_pixel_mapper",
        "description": "Executes OpenCV Gray Code sequence analysis to auto-map custom prop pixel coordinates from camera feed. Generates 2K normal+inverted Gray Code light patterns (K=ceil(log2(N))), decodes camera captures using Bit_k = Frame_pattern > Frame_inverse_pattern, and exports an xLights Custom Model XML.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "total_nodes": {"type": "number", "description": "Total number of prop nodes to map"},
                "model_width": {"type": "number", "description": "Custom model grid width (columns)"},
                "model_height": {"type": "number", "description": "Custom model grid height (rows)"},
                "model_name": {"type": "string", "description": "Name for the exported Custom Model (default: AI_MappedProp)"},
                "threshold_delta": {"type": "number", "description": "Minimum brightness delta to confirm a pattern bit (default: 10.0)"}
            },
            "required": ["total_nodes", "model_width", "model_height"]
        }
    },
    {
        "name": "auto_prop_mapper",
        "description": "Runs a full OpenCV Gray Code auto-mapping session using PropMappingConfig. Generates 2K patterns, decodes camera captures, and exports an xLights Custom Model XML via POST /api/auto-map-prop.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "total_nodes":      {"type": "number",  "description": "Total LED/pixel nodes on the prop"},
                "model_width":      {"type": "number",  "description": "Custom Model grid width (columns)"},
                "model_height":     {"type": "number",  "description": "Custom Model grid height (rows)"},
                "model_name":       {"type": "string",  "description": "xLights Custom Model name (default: AI_MappedProp)"},
                "camera_index":     {"type": "number",  "description": "OpenCV camera device index (default: 0)"},
                "camera_width":     {"type": "number",  "description": "Camera capture width in pixels (default: 1280)"},
                "camera_height":    {"type": "number",  "description": "Camera capture height in pixels (default: 720)"},
                "threshold_delta":  {"type": "number",  "description": "Min brightness delta to confirm a pattern bit (default: 10.0)"},
                "export_csv":       {"type": "boolean", "description": "Also return a CSV of pixel coordinates (default: false)"},
                "output_directory": {"type": "string",  "description": "Directory to write output files (empty = in-memory only)"}
            },
            "required": ["total_nodes", "model_width", "model_height"]
        }
    }
]

def handle_tool_call(name: str, arguments: dict):
    if name == "xlights_get_version":
        return http_get("/api/version")
    elif name in ["get_show_layout", "xlights_get_show_layout", "xlights_get_models"]:
        return http_get("/api/layout")
    elif name in ["get_sequence_state", "xlights_get_sequence_state", "xlights_get_sequence_info"]:
        return http_get("/api/sequence")
    elif name in ["trigger_render", "xlights_trigger_render", "xlights_render_sequence"]:
        return http_post("/api/render", arguments)
    elif name == "xlights_play_sequence":
        return http_post("/api/play", {})
    elif name == "xlights_stop_sequence":
        return http_post("/api/stop", {})
    elif name in ["separate_audio_stems", "xlights_extract_audio_stems"]:
        return http_post("/api/audio-stems", arguments)
    elif name == "xlights_import_xtiming":
        return http_post("/api/ai/import_xtiming", arguments)
    elif name == "xlights_recommend_layer_blend":
        return http_post("/api/ai/recommend_blend", arguments)
    elif name in ["insert_effect", "xlights_insert_effect", "xlights_set_effect"]:
        return http_post("/api/effect", arguments)
    elif name == "xlights_generate_value_curve":
        return http_post("/api/ai/value_curve", arguments)
    elif name in ["map_audio_dynamics", "extract_audio_volume_envelope"]:
        return http_post("/api/ai/audio_envelope", arguments)
    elif name == "analyze_audio_dynamics":
        return http_post("/api/audio-dynamics", arguments)
    elif name == "gray_code_pixel_mapper":
        return http_post("/api/ai/gray-code-map", arguments)
    elif name in ["auto_prop_mapper", "auto_map_prop_camera"]:
        return http_post("/api/auto-map-prop", arguments)
    else:
        return {"error": f"Unknown tool: {name}"}

def main():
    for line in sys.stdin:
        if not line.strip():
            continue
        try:
            req = json.loads(line)
            method = req.get("method")
            msg_id = req.get("id")

            if method == "tools/list":
                res = {"jsonrpc": "2.0", "id": msg_id, "result": {"tools": MCP_TOOLS}}
            elif method == "tools/call":
                params = req.get("params", {})
                t_name = params.get("name")
                t_args = params.get("arguments", {})
                result_data = handle_tool_call(t_name, t_args)
                res = {"jsonrpc": "2.0", "id": msg_id, "result": {"content": [{"type": "text", "text": json.dumps(result_data, indent=2)}]}}
            else:
                res = {"jsonrpc": "2.0", "id": msg_id, "error": {"code": -32601, "message": "Method not found"}}

            sys.stdout.write(json.dumps(res) + "\n")
            sys.stdout.flush()
        except Exception as e:
            err_res = {"jsonrpc": "2.0", "id": None, "error": {"code": -32603, "message": str(e)}}
            sys.stdout.write(json.dumps(err_res) + "\n")
            sys.stdout.flush()

if __name__ == "__main__":
    main()
