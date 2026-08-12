/**
 * Mock xLights REST API Server (Port 49913)
 * Provides synthetic REST endpoints for testing the xLights MCP Server.
 */

import http from 'http';

const PORT = process.env.TEST_PORT ? parseInt(process.env.TEST_PORT) : 49914;

export function startMockRestServer(): Promise<http.Server> {
    return new Promise((resolve) => {
        const server = http.createServer((req, res) => {
            res.setHeader('Content-Type', 'application/json');

            if (req.url === '/api/version') {
                res.writeHead(200);
                res.end(JSON.stringify({ version: "2026.12", build: "AI Expansion Edition", rest_port: 49913 }));
            } else if (req.url === '/api/layout' || req.url === '/api/models') {
                res.writeHead(200);
                res.end(JSON.stringify({
                    models: [
                        { name: "MegaTree", type: "Tree 360", pixels: 1600, start_channel: 1, end_channel: 4800 },
                        { name: "House Outline", type: "Poly Line", pixels: 800, start_channel: 4801, end_channel: 7200 },
                        { name: "Matrix 32x64", type: "Matrix", pixels: 2048, start_channel: 7201, end_channel: 13344 }
                    ]
                }));
            } else if (req.url === '/api/sequence') {
                res.writeHead(200);
                res.end(JSON.stringify({ sequence_name: "Carol of the Bells", duration_ms: 180000, fps: 40 }));
            } else if (req.url === '/api/ai/value_curve' && req.method === 'POST') {
                let body = '';
                req.on('data', chunk => body += chunk);
                req.on('end', () => {
                    res.writeHead(200);
                    res.end(JSON.stringify({
                        Type: "Custom",
                        Min: 0.0,
                        Max: 100.0,
                        Points: [{ x: 0.0, y: 0.0 }, { x: 0.5, y: 50.0 }, { x: 1.0, y: 90.0 }],
                        Serialized: "Active=TRUE|Type=Custom|Min=0.00|Max=100.00|Values=0.00:0.00;0.50:50.00;1.00:90.00|"
                    }));
                });
            } else {
                res.writeHead(200);
                res.end(JSON.stringify({ status: "ok", endpoint: req.url }));
            }
        });

        server.listen(PORT, '127.0.0.1', () => {
            resolve(server);
        });
    });
}

if (require.main === module) {
    startMockRestServer().then(() => {
        console.log(`Mock xLights REST API server listening on http://localhost:${PORT}`);
    });
}
