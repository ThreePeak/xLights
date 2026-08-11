"use strict";
/**
 * xLights Model Context Protocol (MCP) Server (TypeScript)
 * Interfacing external AI agents with xLights REST API running on http://localhost:49913
 */
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const http_1 = __importDefault(require("http"));
const readline_1 = __importDefault(require("readline"));
const XLIGHTS_REST_PORT = 49913;
const XLIGHTS_HOST = 'localhost';
function httpGet(endpoint) {
    return new Promise((resolve) => {
        const options = {
            hostname: XLIGHTS_HOST,
            port: XLIGHTS_REST_PORT,
            path: endpoint,
            method: 'GET',
            timeout: 5000
        };
        const req = http_1.default.request(options, (res) => {
            let data = '';
            res.on('data', (chunk) => data += chunk);
            res.on('end', () => {
                try {
                    resolve(JSON.parse(data));
                }
                catch (e) {
                    resolve({ status: 'ok', raw: data });
                }
            });
        });
        req.on('error', (err) => {
            resolve({ status: 'error', message: `xLights REST connection failed on port ${XLIGHTS_REST_PORT}: ${err.message}` });
        });
        req.end();
    });
}
function httpPost(endpoint, payload) {
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
        const req = http_1.default.request(options, (res) => {
            let data = '';
            res.on('data', (chunk) => data += chunk);
            res.on('end', () => {
                try {
                    resolve(JSON.parse(data));
                }
                catch (e) {
                    resolve({ status: 'ok', raw: data });
                }
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
async function handleToolCall(name, args) {
    switch (name) {
        case "xlights_get_version": return await httpGet("/api/version");
        case "get_show_layout":
        case "xlights_get_show_layout":
        case "xlights_get_models": return await httpGet("/api/layout");
        case "xlights_get_sequence_info": return await httpGet("/api/sequence");
        case "xlights_get_show_folder": return await httpGet("/api/showfolder");
        case "xlights_get_controllers": return await httpGet("/api/controllers");
        case "xlights_get_effect_presets": return await httpGet("/api/presets");
        case "xlights_render_sequence": return await httpPost("/api/render", {});
        case "xlights_play_sequence": return await httpPost("/api/play", {});
        case "xlights_stop_sequence": return await httpPost("/api/stop", {});
        case "xlights_set_effect": return await httpPost("/api/effect", args);
        case "xlights_generate_value_curve": return await httpPost("/api/ai/value_curve", args);
        default: return { error: `Unknown tool: ${name}` };
    }
}
const rl = readline_1.default.createInterface({ input: process.stdin, output: process.stdout, terminal: false });
rl.on('line', async (line) => {
    if (!line.trim())
        return;
    try {
        const req = JSON.parse(line);
        const method = req.method;
        const msgId = req.id;
        if (method === "tools/list") {
            console.log(JSON.stringify({ jsonrpc: "2.0", id: msgId, result: { tools: MCP_TOOLS } }));
        }
        else if (method === "tools/call") {
            const params = req.params || {};
            const resultData = await handleToolCall(params.name, params.arguments || {});
            console.log(JSON.stringify({
                jsonrpc: "2.0",
                id: msgId,
                result: { content: [{ type: "text", text: JSON.stringify(resultData, null, 2) }] }
            }));
        }
        else {
            console.log(JSON.stringify({ jsonrpc: "2.0", id: msgId, error: { code: -32601, message: "Method not found" } }));
        }
    }
    catch (e) {
        console.log(JSON.stringify({ jsonrpc: "2.0", id: null, error: { code: -32603, message: e.message } }));
    }
});
