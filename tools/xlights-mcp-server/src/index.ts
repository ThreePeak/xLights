/**
 * xLights Model Context Protocol (MCP) Server (TypeScript)
 * Interfacing external AI agents with xLights REST API running on http://localhost:49913
 */

import http from 'http';
import readline from 'readline';

const XLIGHTS_REST_PORT = 49913;
const XLIGHTS_HOST = 'localhost';

export interface MCPToolCallArgs {
  [key: string]: any;
}

function httpGet(endpoint: string): Promise<any> {
    return new Promise((resolve) => {
        const options = {
            hostname: XLIGHTS_HOST,
            port: XLIGHTS_REST_PORT,
            path: endpoint,
            method: 'GET',
            timeout: 5000
        };

        const req = http.request(options, (res) => {
            let data = '';
            res.on('data', (chunk) => data += chunk);
            res.on('end', () => {
                try { resolve(JSON.parse(data)); }
                catch (e) { resolve({ status: 'ok', raw: data }); }
            });
        });

        req.on('error', (err) => {
            resolve({ status: 'error', message: `xLights REST connection failed on port ${XLIGHTS_REST_PORT}: ${err.message}` });
        });

        req.end();
    });
}

function httpPost(endpoint: string, payload: any): Promise<any> {
    return new Promise((resolve) => {
        const dataBytes = Buffer.from(JSON.stringify(payload));
        const options = {
            hostname: XLIGHTS_HOST,
            port: XLIGHTS_REST_PORT,
            path: endpoint,
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Content-Length': dataBytes.length
            },
            timeout: 5000
        };

        const req = http.request(options, (res) => {
            let data = '';
            res.on('data', (chunk) => data += chunk);
            res.on('end', () => {
                try { resolve(JSON.parse(data)); }
                catch (e) { resolve({ status: 'ok', raw: data }); }
            });
        });

        req.on('error', (err) => {
            resolve({ status: 'error', message: `xLights REST POST failed on port ${XLIGHTS_REST_PORT}: ${err.message}` });
        });

        req.write(dataBytes);
        req.end();
    });
}

const MCP_TOOLS = [
    {
        name: "xlights_get_version",
        description: "Get xLights version and REST API status on port 49913",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "get_show_layout",
        description: "Queries active models, submodels, pixel counts, and channel bounds from xLights REST API",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "xlights_get_models",
        description: "List all physical display models in the active xLights layout",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "get_sequence_state",
        description: "Fetches active sequence media file path, total duration in milliseconds, frame rate, and track structures",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "xlights_get_sequence_info",
        description: "Get active sequence details including duration, frame rate, and media file",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "xlights_get_show_folder",
        description: "Get current active xLights show directory path",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "xlights_get_controllers",
        description: "List configured pixel controllers (FPP, Falcon, Kulp) and IP addresses",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "xlights_get_effect_presets",
        description: "List custom multi-layer effect presets saved in the show directory",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "trigger_render",
        description: "Sends a render command payload to xLights REST API port 49913 to trigger sequence rendering",
        inputSchema: {
            type: "object",
            properties: {
                model_name: { type: "string", description: "Optional specific model name to render, or empty for full sequence render" }
            }
        }
    },
    {
        name: "xlights_render_sequence",
        description: "Trigger full sequence re-rendering across all models",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "xlights_play_sequence",
        description: "Start real-time sequence playback in xLights preview",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "xlights_stop_sequence",
        description: "Stop sequence playback",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "insert_effect",
        description: "Pushes effect parameters (target model, effect name, start/end timing, color palette) to active timeline tracks",
        inputSchema: {
            type: "object",
            properties: {
                model_name: { type: "string", description: "Target display model name e.g. MegaTree, House Outline" },
                effect_name: { type: "string", description: "Effect type e.g. SingleStrand, ColorWash, Twinkle, Bars, Fire, Butterfly" },
                start_ms: { type: "integer", description: "Start time in milliseconds" },
                end_ms: { type: "integer", description: "End time in milliseconds" },
                palette: { type: "array", items: { type: "string" }, description: "Color hex strings array e.g. ['#FF0000', '#00FF00']" },
                settings: { type: "object", description: "Optional effect settings key-value object" }
            },
            required: ["model_name", "effect_name", "start_ms", "end_ms"]
        }
    },
    {
        name: "xlights_set_effect",
        description: "Place an effect onto a target model in the active sequence",
        inputSchema: {
            type: "object",
            properties: {
                model_name: { type: "string" },
                effect_name: { type: "string" },
                start_ms: { type: "integer" },
                end_ms: { type: "integer" },
                palette: { type: "array", items: { type: "string" } }
            },
            required: ["model_name", "effect_name", "start_ms", "end_ms"]
        }
    },
    {
        name: "xlights_extract_audio_stems",
        description: "Executes deep learning HTDemucs source separation on active sequence audio to isolate Vocals, Drums, Bass, and Other stems",
        inputSchema: {
            type: "object",
            properties: {
                model_path: { type: "string", description: "Path to ONNX or CoreML HTDemucs model file" },
                output_directory: { type: "string", description: "Directory to save exported stem WAV files" },
                transient_sensitivity: { type: "number", description: "Transient onset sensitivity threshold (default 0.12)" }
            }
        }
    },
    {
        name: "xlights_import_xtiming",
        description: "Imports a compiled .xtiming XML file or payload into active xLights sequence timing tracks",
        inputSchema: {
            type: "object",
            properties: {
                timing_name: { type: "string", description: "Timing track name e.g. 'AI Stems - Drums & Onsets'" },
                xtiming_xml: { type: "string", description: "Raw .xtiming XML content payload" },
                file_path: { type: "string", description: "Path to .xtiming XML file" }
            },
            required: ["timing_name"]
        }
    },
    {
        name: "xlights_recommend_layer_blend",
        description: "Evaluates multi-layer effect stacks and recommends optimal blend modes (Additive, Layered, Mask) and transition durations",
        inputSchema: {
            type: "object",
            properties: {
                top_effect: { type: "string", description: "Top effect type e.g. Twinkle, Meteors, Strobe" },
                bottom_effect: { type: "string", description: "Bottom effect type e.g. ColorWash, Fire, Wave" }
            },
            required: ["top_effect", "bottom_effect"]
        }
    },
    {
        name: "xlights_synthesize_effect_preset",
        description: "Synthesizes multi-layer effect stack presets from natural language prompts e.g. 'Fire effect with twinkling stars'",
        inputSchema: {
            type: "object",
            properties: {
                preset_name: { type: "string", description: "Name for generated preset" },
                prompt: { type: "string", description: "Natural language description e.g. 'Fireworks with decaying twinkling background'" }
            },
            required: ["preset_name", "prompt"]
        }
    },
    {
        name: "xlights_generate_value_curve",
        description: "Synthesize a natural language prompt into a normalized Bezier value curve JSON array and xLights pipe string",
        inputSchema: {
            type: "object",
            properties: {
                prompt: { type: "string", description: "Natural language curve descriptor" },
                min: { type: "number" },
                max: { type: "number" }
            },
            required: ["prompt"]
        }
    }
];

async function handleToolCall(name: string, args: MCPToolCallArgs): Promise<any> {
    switch (name) {
        case "xlights_get_version": return await httpGet("/api/version");
        case "get_show_layout":
        case "xlights_get_show_layout":
        case "xlights_get_models": return await httpGet("/api/layout");
        case "get_sequence_state":
        case "xlights_get_sequence_state":
        case "xlights_get_sequence_info": return await httpGet("/api/sequence");
        case "xlights_get_show_folder": return await httpGet("/api/showfolder");
        case "xlights_get_controllers": return await httpGet("/api/controllers");
        case "xlights_get_effect_presets": return await httpGet("/api/presets");
        case "trigger_render":
        case "xlights_trigger_render":
        case "xlights_render_sequence": return await httpPost("/api/render", args);
        case "xlights_play_sequence": return await httpPost("/api/play", {});
        case "xlights_stop_sequence": return await httpPost("/api/stop", {});
        case "insert_effect":
        case "xlights_insert_effect":
        case "xlights_set_effect": return await httpPost("/api/effect", args);
        case "xlights_extract_audio_stems": return await httpPost("/api/audio-stems", args);
        case "xlights_import_xtiming": return await httpPost("/api/ai/import_xtiming", args);
        case "xlights_recommend_layer_blend": return await httpPost("/api/ai/recommend_blend", args);
        case "xlights_synthesize_effect_preset": return await httpPost("/api/ai/synthesize_preset", args);
        case "xlights_generate_value_curve": return await httpPost("/api/ai/value_curve", args);
        default: return { error: `Unknown tool: ${name}` };
    }
}

const rl = readline.createInterface({ input: process.stdin, output: process.stdout, terminal: false });

rl.on('line', async (line: string) => {
    if (!line.trim()) return;
    try {
        const req = JSON.parse(line);
        const method = req.method;
        const msgId = req.id;

        if (method === "tools/list") {
            console.log(JSON.stringify({ jsonrpc: "2.0", id: msgId, result: { tools: MCP_TOOLS } }));
        } else if (method === "tools/call") {
            const params = req.params || {};
            const resultData = await handleToolCall(params.name, params.arguments || {});
            console.log(JSON.stringify({
                jsonrpc: "2.0",
                id: msgId,
                result: { content: [{ type: "text", text: JSON.stringify(resultData, null, 2) }] }
            }));
        } else {
            console.log(JSON.stringify({ jsonrpc: "2.0", id: msgId, error: { code: -32601, message: "Method not found" } }));
        }
    } catch (e: any) {
        console.log(JSON.stringify({ jsonrpc: "2.0", id: null, error: { code: -32603, message: e.message } }));
    }
});
