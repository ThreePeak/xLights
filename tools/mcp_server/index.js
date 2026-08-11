/**
 * xLights Model Context Protocol (MCP) Server (Node.js)
 * Interfacing external AI agents with xLights REST API running on http://localhost:49913
 */

const http = require('http');
const readline = require('readline');

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
        name: "xlights_get_models",
        description: "List all physical models in the active xLights show layout",
        inputSchema: { type: "object", properties: {} }
    },
    {
        name: "xlights_get_sequence_info",
        description: "Get active sequence details including duration, frame rate, and media file",
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
                end_ms: { type: "integer" }
            },
            required: ["model_name", "effect_name", "start_ms", "end_ms"]
        }
    }
];

async function handleToolCall(name, args) {
    switch (name) {
        case "xlights_get_version": return await httpGet("/api/version");
        case "xlights_get_models": return await httpGet("/api/models");
        case "xlights_get_sequence_info": return await httpGet("/api/sequence");
        case "xlights_render_sequence": return await httpPost("/api/render", {});
        case "xlights_play_sequence": return await httpPost("/api/play", {});
        case "xlights_stop_sequence": return await httpPost("/api/stop", {});
        case "xlights_set_effect": return await httpPost("/api/effect", args);
        default: return { error: `Unknown tool: ${name}` };
    }
}

const rl = readline.createInterface({ input: process.stdin, output: process.stdout, terminal: false });

rl.on('line', async (line) => {
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
    } catch (e) {
        console.log(JSON.stringify({ jsonrpc: "2.0", id: null, error: { code: -32603, message: e.message } }));
    }
});
