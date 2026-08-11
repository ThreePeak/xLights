/**
 * Test Suite for xLights MCP Server
 * Validates JSON-RPC 2.0 stdio protocols and REST API tool execution.
 */

import { exec } from 'child_process';
import path from 'path';
import { startMockRestServer } from './mock_rest_api';

describe('xLights MCP Server Test Suite', () => {
    let mockServer: any;

    beforeAll(async () => {
        mockServer = await startMockRestServer();
    });

    afterAll(() => {
        if (mockServer) mockServer.close();
    });

    it('should list all registered MCP tools via tools/list request', (done) => {
        const serverPath = path.join(__dirname, '../dist/index.js');
        const child = exec(`node "${serverPath}"`, (error, stdout, stderr) => {
            expect(error).toBeNull();
            const response = JSON.parse(stdout.trim());
            expect(response.jsonrpc).toBe("2.0");
            expect(response.id).toBe(1);
            expect(response.result.tools).toBeDefined();
            expect(response.result.tools.length).toBeGreaterThanOrEqual(8);

            const toolNames = response.result.tools.map((t: any) => t.name);
            expect(toolNames).toContain("get_show_layout");
            expect(toolNames).toContain("xlights_get_version");
            expect(toolNames).toContain("xlights_generate_value_curve");
            done();
        });

        const req = JSON.stringify({ jsonrpc: "2.0", id: 1, method: "tools/list" }) + "\n";
        child.stdin?.write(req);
        child.stdin?.end();
    });

    it('should execute get_show_layout tool call against mock REST API', (done) => {
        const serverPath = path.join(__dirname, '../dist/index.js');
        const child = exec(`node "${serverPath}"`, (error, stdout, stderr) => {
            expect(error).toBeNull();
            const response = JSON.parse(stdout.trim());
            expect(response.jsonrpc).toBe("2.0");
            expect(response.id).toBe(2);
            expect(response.result.content).toBeDefined();

            const textContent = JSON.parse(response.result.content[0].text);
            expect(textContent.models).toBeDefined();
            expect(textContent.models.length).toBe(3);
            expect(textContent.models[0].name).toBe("MegaTree");
            done();
        });

        const req = JSON.stringify({
            jsonrpc: "2.0",
            id: 2,
            method: "tools/call",
            params: { name: "get_show_layout", arguments: {} }
        }) + "\n";
        child.stdin?.write(req);
        child.stdin?.end();
    });
});
