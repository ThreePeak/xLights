/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AI/AIHelpContentRegistry.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace xLights::AI {

bool AIHelpContentRegistry::s_initialized = false;

std::map<std::string, AIHelpTopic>& AIHelpContentRegistry::GetRegistry() {
    static std::map<std::string, AIHelpTopic> s_registry;
    return s_registry;
}

std::string AIHelpTopic::GenerateHtml() const {
    std::ostringstream oss;
    oss << "<!DOCTYPE html>\n<html>\n<head>\n<style>\n"
        << "body { font-family: 'Segoe UI', Helvetica, Arial, sans-serif; background-color: #1a1e24; color: #d8dee9; margin: 15px; line-height: 1.6; }\n"
        << "h1 { color: #88c0d0; border-bottom: 2px solid #3b4252; padding-bottom: 8px; margin-top: 0; font-size: 22px; }\n"
        << "h2 { color: #81a1c1; margin-top: 20px; font-size: 17px; border-bottom: 1px solid #2e3440; padding-bottom: 4px; }\n"
        << "h3 { color: #b48ead; margin-top: 14px; font-size: 15px; }\n"
        << ".badge { display: inline-block; background-color: #434c5e; color: #eceff4; padding: 2px 8px; border-radius: 4px; font-size: 11px; margin-bottom: 10px; font-weight: bold; }\n"
        << ".summary-box { background-color: #242933; border-left: 4px solid #88c0d0; padding: 12px 16px; margin: 10px 0; border-radius: 0 6px 6px 0; }\n"
        << ".diagram-box { background-color: #0f141c; color: #a3be8c; padding: 12px; font-family: 'Consolas', 'Courier New', monospace; font-size: 12px; white-space: pre; border-radius: 6px; border: 1px solid #2e3440; overflow-x: auto; }\n"
        << "table { width: 100%; border-collapse: collapse; margin: 12px 0; font-size: 13px; }\n"
        << "th { background-color: #2e3440; color: #88c0d0; text-align: left; padding: 8px 10px; border: 1px solid #3b4252; }\n"
        << "td { padding: 8px 10px; border: 1px solid #2e3440; vertical-align: top; }\n"
        << "tr:nth-child(even) { background-color: #222730; }\n"
        << ".setting-name { font-weight: bold; color: #ebcb8b; }\n"
        << ".setting-type { color: #d08770; font-size: 11px; }\n"
        << ".effect-text { color: #e5e9f0; }\n"
        << ".rec-text { color: #a3be8c; font-style: italic; }\n"
        << ".warn-text { color: #bf616a; }\n"
        << ".step-list { padding-left: 20px; }\n"
        << ".step-item { margin-bottom: 6px; }\n"
        << ".tip-box { background-color: #2b3339; border-left: 4px solid #a3be8c; padding: 8px 12px; margin: 8px 0; font-size: 12px; }\n"
        << ".warn-box { background-color: #352528; border-left: 4px solid #bf616a; padding: 8px 12px; margin: 8px 0; font-size: 12px; }\n"
        << "</style>\n</head>\n<body>\n";

    oss << "<h1>" << title << "</h1>\n";
    oss << "<span class=\"badge\">" << category << "</span>\n";
    oss << "<div class=\"summary-box\">" << summary << "</div>\n";

    // Workflow Steps
    if (!workflowSteps.empty()) {
        oss << "<h2>📋 Standard Workflow & Operation</h2>\n<ol class=\"step-list\">\n";
        for (const auto& step : workflowSteps) {
            oss << "<li class=\"step-item\">" << step << "</li>\n";
        }
        oss << "</ol>\n";
    }

    // Interactive Architecture / Flowchart Diagram
    if (!diagram.empty()) {
        oss << "<h2>📐 Architecture & Data Flow Diagram</h2>\n";
        oss << "<div class=\"diagram-box\">" << diagram << "</div>\n";
    }

    // Setting-by-setting options table
    if (!options.empty()) {
        oss << "<h2>⚙️ Settings & Options Detailed Breakdown</h2>\n";
        oss << "<table>\n<thead>\n<tr>\n"
            << "<th style=\"width:22%;\">Option / Control</th>\n"
            << "<th style=\"width:32%;\">Description & Function</th>\n"
            << "<th style=\"width:26%;\">Direct Effect / Impact</th>\n"
            << "<th style=\"width:20%;\">Recommended Setting</th>\n"
            << "</tr>\n</thead>\n<tbody>\n";

        for (const auto& opt : options) {
            oss << "<tr>\n"
                << "<td><div class=\"setting-name\">" << opt.name << "</div><div class=\"setting-type\">" << opt.controlType << "</div></td>\n"
                << "<td>" << opt.description;
            if (!opt.warning.empty()) {
                oss << "<div class=\"warn-text\">⚠️ " << opt.warning << "</div>";
            }
            oss << "</td>\n"
                << "<td><div class=\"effect-text\">" << opt.effect << "</div></td>\n"
                << "<td><div class=\"rec-text\">" << opt.recommended << "</div></td>\n"
                << "</tr>\n";
        }
        oss << "</tbody>\n</table>\n";
    }

    // Practical Examples
    if (!examples.empty()) {
        oss << "<h2>💡 Practical Examples & Presets</h2>\n";
        for (const auto& ex : examples) {
            oss << "<div class=\"tip-box\">" << ex << "</div>\n";
        }
    }

    // Troubleshooting & Gotchas
    if (!troubleshooting.empty()) {
        oss << "<h2>🔧 Troubleshooting & Common Issues</h2>\n";
        for (const auto& tb : troubleshooting) {
            oss << "<div class=\"warn-box\">" << tb << "</div>\n";
        }
    }

    oss << "</body>\n</html>";
    return oss.str();
}

void AIHelpContentRegistry::Initialize() {
    if (s_initialized) return;

    // 1. DMX Address Conflict Advisor
    {
        AIHelpTopic t;
        t.topicId = "DMX_ADDRESS_ADVISOR";
        t.title = "AI DMX & Universe Address Conflict Advisor";
        t.category = "Layout & Hardware";
        t.summary = "Analyzes universe and channel configurations across your show controllers to detect overlapping fixtures, duplicate channel offsets, and compute conflict-free DMX universe assignments.";
        t.workflowSteps = {
            "1. Paste or load your universe XML definitions into the configuration input area.",
            "2. Click '🔍 Detect Conflicts' to run the conflict analysis algorithm across all channels.",
            "3. Review detected overlapping fixture assignments in the conflicts table.",
            "4. Click '⚡ Auto-Remap' to generate an optimized, conflict-free layout map.",
            "5. Click '📄 Export Report' to save the resolved channel mapping manifest."
        };
        t.diagram = 
            "+--------------------------------------------------------------+\n"
            "|                     DMX Universe Map Flow                    |\n"
            "+--------------------------------------------------------------+\n"
            "| [Universe XML Input] ---> [Overlap Detector & Bounds Check]  |\n"
            "|                                  |                           |\n"
            "|                                  v                           |\n"
            "|                      [Conflict Classification]               |\n"
            "|                         /                 \\                  |\n"
            "|            (Overlapping Channels)    (Clean Assignments)     |\n"
            "|                        |                      |              |\n"
            "|                        v                      v              |\n"
            "|                [Auto-Remap Solver]      [Pass Audit]         |\n"
            "|                        |                                     |\n"
            "|                        v                                     |\n"
            "|             [Export Resolved Manifest]                       |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Universe XML Input", "Multi-line Text / File Picker", "Raw XML representation of controller universes and channel blocks.", "Provides the data source for overlap inspection.", "Load directly from show.xml or controller export.", "Invalid XML syntax will prevent analysis."},
            {"Detect Conflicts Button", "Primary Action Button", "Parses all channels and identifies range intersections.", "Populates the table with conflicting fixtures and channel spans.", "Click whenever XML input changes.", ""},
            {"Auto-Remap Button", "Action Button", "Computes the next contiguous available channels across all universes without overlaps.", "Updates fixture addresses non-destructively in memory.", "Use to resolve dense matrix and prop conflicts.", "Always review the resulting map before writing to hardware."}
        };
        t.examples = {
            "MegaTree & Matrix Overlap: Detects MegaTree Universe 1:1-512 colliding with Matrix Universe 1:300-512 and shifts Matrix to Universe 2:1.",
            "Multi-Controller Audit: Scans 16 E1.31/sACN controllers and identifies duplicate start universe IDs."
        };
        t.troubleshooting = {
            "No conflicts detected but lights behave erratically: Verify that physical controller IP addresses and subnet masks match show.xml.",
            "Auto-Remap shifts universe boundaries unexpectedly: Check if individual props have 'Absolute Channel' addressing locked."
        };
        RegisterTopic(t);
    }

    // 2. FPP Multi-Sync & Controller Auto-Provisioner
    {
        AIHelpTopic t;
        t.topicId = "FPP_MULTI_SYNC";
        t.title = "AI Falcon Player (FPP) Multi-Sync & Auto-Provisioner";
        t.category = "Layout & Hardware";
        t.summary = "Auto-discovers Falcon Player (FPP) master and remote players on the local network, synchronizes sequence layouts, and exports controller configuration JSON manifests.";
        t.workflowSteps = {
            "1. Enter the IP address or hostname of your primary Falcon Player instance.",
            "2. Select your show.xml file path to match sequence channel definitions.",
            "3. Click '⚡ Analyze Controllers' to query controller topology and channel spans.",
            "4. Review mapped controller IP addresses, universes, and start channels.",
            "5. Click '📄 Export FPP JSON Manifest' to generate fpp-universes.json for upload."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|                 FPP Multi-Sync Architecture                  |\n"
            "+--------------------------------------------------------------+\n"
            "|  xLights show.xml  ---> [Network Controller Inspector]        |\n"
            "|                                  |                           |\n"
            "|                                  v                           |\n"
            "|  FPP Host (REST API) <--- [Topology Matching Engine]         |\n"
            "|                                  |                           |\n"
            "|                                  v                           |\n"
            "|                     [Auto-Provisioned Manifest]              |\n"
            "|                     (fpp-universes.json & Remotes)           |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"FPP Host IP / Hostname", "Text Ctrl", "Network address of target FPP instance.", "Directs REST API synchronization packets to the specified node.", "192.168.1.x or fpp.local", "Ensure machine has network route to the FPP subnet."},
            {"show.xml File Picker", "File Picker", "Path to local show configuration file.", "Provides the ground-truth channel counts and controller models.", "Select your active show folder show.xml.", "Selecting an outdated show.xml will cause channel mismatch."}
        };
        t.examples = {
            "Master/Remote Auto-Sync: Generates sync configs for 1 Master Raspberry Pi and 4 Remote ESP32 pixel controllers."
        };
        t.troubleshooting = {
            "FPP connection timeout: Verify FPP is booted and REST API port 80 is not blocked by firewall."
        };
        RegisterTopic(t);
    }

    // 3. AI Lua Scripting & Automation Copilot
    {
        AIHelpTopic t;
        t.topicId = "LUA_SCRIPTING";
        t.title = "AI Lua Scripting & Automation Copilot";
        t.category = "Generative & Automation";
        t.summary = "Synthesizes, sandbox-verifies, and executes custom Lua sequencing automation scripts via LLM intelligence from plain English prompts.";
        t.workflowSteps = {
            "1. Choose your AI reasoning provider (Cloud Primary, Local ONNX, or Ollama).",
            "2. Adjust temperature slider (lower for precise math, higher for creative animations).",
            "3. Enter your natural language prompt describing the desired effect or automation.",
            "4. Click '✨ Generate Script' to synthesize the Lua script code.",
            "5. Click '🛡️ Validate Sandbox' to verify code safety against unauthorized system calls.",
            "6. Click '💾 Save Script' to export your script to disk."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|                 Lua Script Synthesis Pipeline                |\n"
            "+--------------------------------------------------------------+\n"
            "| [User Prompt] ---> [LLM Reasoning Engine]                    |\n"
            "|                            |                                 |\n"
            "|                            v                                 |\n"
            "|               [Raw Lua Code Generation]                      |\n"
            "|                            |                                 |\n"
            "|                            v                                 |\n"
            "|               [AST Security Sandbox Check]                   |\n"
            "|                 /                      \\                     |\n"
            "|           (Safe APIs)             (Blocked Syscalls)         |\n"
            "|                |                          |                  |\n"
            "|                v                          v                  |\n"
            "|      [Validated Lua Script]        [Security Alert]          |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Provider", "Choice Dropdown", "AI backend used for script synthesis.", "Cloud models offer high reasoning; Local ONNX/Ollama works offline.", "Cloud Primary for complex logic; Ollama for offline.", "Ensure Ollama is running locally if selected."},
            {"Temperature", "Slider (0 - 2.0)", "Controls randomness and creativity in generated code.", "Low values (0.2-0.5) produce strict math; High values (0.7-1.0) produce artistic variations.", "0.7 for effect scripting.", "Values above 1.5 may produce syntax errors."},
            {"Strict Security Sandbox", "Checkbox", "Enforces AST analysis to block os.execute, file deletions, and infinite loops.", "Guarantees that generated scripts cannot harm your system.", "Always Enabled (Checked).", "Disabling sandbox is not recommended."}
        };
        t.examples = {
            "Rainbow Spiral: 'Generate a cascading 3D rainbow color wave across the MegaTree timed to 120 BPM.'",
            "Random Shimmer: 'Create a starry twinkling effect that randomly fades nodes between deep navy and warm gold.'"
        };
        t.troubleshooting = {
            "Script syntax error: Lower temperature to 0.4 and click Generate Script again.",
            "Sandbox rejected script: Ensure your prompt does not request file system or command line access."
        };
        RegisterTopic(t);
    }

    // 4. AI Power Injection & Voltage Drop Calculator
    {
        AIHelpTopic t;
        t.topicId = "POWER_INJECTION";
        t.title = "AI Power Injection & Voltage Drop Calculator";
        t.category = "Layout & Hardware";
        t.summary = "Simulates Ohm's Law resistive voltage drops across long pixel strings and wire feeds, calculating end-of-line voltage, total amperage draw, and optimal power injection tap locations.";
        t.workflowSteps = {
            "1. Select Supply Voltage (12V DC, 5V DC, or 24V DC).",
            "2. Select Feed Wire Gauge (AWG) and Wire Distance in feet.",
            "3. Enter total pixel count on the continuous strand.",
            "4. Click '⚡ Recalculate Power Drops' to compute voltage profile.",
            "5. Review recommended injection points in the table.",
            "6. Click '📄 Export CSV Report' to print wiring diagram sheet."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|               Power Injection Voltage Profile                |\n"
            "+--------------------------------------------------------------+\n"
            "| PSU (12.0V) ===[Feed Wire]=== Pixel 1 (11.8V)                 |\n"
            "|                                    |                         |\n"
            "|                             [String Resistance]              |\n"
            "|                                    |                         |\n"
            "|                              Pixel 150 (9.8V)  <-- INJECT!   |\n"
            "|                                    |                         |\n"
            "|                              Pixel 300 (10.4V)               |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Supply Voltage", "Choice Dropdown", "Operating DC voltage supplied by the PSU.", "Determines current draw (I = P / V). 5V strings require more frequent injection than 12V.", "12V DC for standard WS2811; 5V DC for high-density ribbons.", "Using 12V setting on 5V pixels will miscalculate resistance."},
            {"Wire Gauge (AWG)", "Choice Dropdown", "Wire thickness for main power feed lines.", "Lower AWG number means thicker wire, lower resistance, and less voltage drop.", "18 AWG or 16 AWG for runs over 20 feet.", "22 AWG causes severe voltage drop on high-current runs."},
            {"Total Pixel Count", "Text Ctrl", "Number of LED nodes on the continuous strand.", "Determines cumulative current load across the string.", "Enter exact model node count.", "Strings over 300 nodes on 12V or 100 on 5V always require injection."}
        };
        t.examples = {
            "MegaTree 300-Pixel String (12V, 18 AWG, 25ft feed): Recommends injection at Pixel 150 and Pixel 300 to maintain >10.5V.",
            "Roof Outline 600-Pixel Run (12V, 16 AWG, 50ft feed): Recommends dual-end injection plus mid-point tap."
        };
        t.troubleshooting = {
            "Pixels at end of string turning pink/dim: Indicates voltage dropped below 9.0V; add a power injection tap at recommended pixel index.",
            "Power supply tripping breaker: Total amperage draw exceeds PSU rated wattage; calculate total wattage and upgrade PSU."
        };
        RegisterTopic(t);
    }

    // 5. AI Gray Code Structured Light 3D Pixel Mapper
    {
        AIHelpTopic t;
        t.topicId = "GRAY_CODE_MAPPER";
        t.title = "AI 3D Pixel Map Camera & Gray Code Structured Light Solver";
        t.category = "Layout & Hardware";
        t.summary = "Projects structured optical binary Gray Code patterns onto physical light props and records camera frames to triangulate exact 3D X/Y/Z spatial node coordinates.";
        t.workflowSteps = {
            "1. Position your camera facing the physical prop in a dark room.",
            "2. Select your connected camera and projection bit depth (10-bit or 12-bit).",
            "3. Click '📸 Start Gray Code Capture' to flash optical sequence.",
            "4. Click '✨ Solve 3D Point Cloud' to triangulate coordinates.",
            "5. Click '💾 Export 3D Model' to save as custom .xmodel XML."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|               Gray Code Triangulation Pipeline               |\n"
            "+--------------------------------------------------------------+\n"
            "| [Pattern Generator] ---> [Projector / Screen Flashes]        |\n"
            "|                                  |                           |\n"
            "|                                  v                           |\n"
            "| [Camera Capture]    ---> [Binary Bitplane Decoding]          |\n"
            "|                                  |                           |\n"
            "|                                  v                           |\n"
            "|                     [Ray-Plane Triangulation]                |\n"
            "|                                  |                           |\n"
            "|                                  v                           |\n"
            "|                    [3D Point Cloud (X, Y, Z)]                |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Select Camera", "Choice Dropdown", "Camera capture device used to photograph light flashes.", "Higher quality lenses yield sharper node centroid detection.", "HD Webcam or DSLR via HDMI.", "Ensure camera auto-exposure and auto-focus are locked."},
            {"Pattern Bit Depth", "Choice Dropdown", "Number of binary projection stripes (10-bit = 1024 divisions, 12-bit = 4096).", "Higher bit depth yields finer spatial coordinate accuracy.", "10-bit for props <500 nodes; 12-bit for dense matrices.", "12-bit takes slightly longer to project."},
            {"Frame Delay (ms)", "Spin Ctrl", "Pause between projection flashes to allow camera exposure settling.", "Prevents frame ghosting and bit decoding errors.", "150 ms (default).", "Setting below 50ms may cause skipped frames on slow USB cameras."}
        };
        t.examples = {
            "Custom Wireframe Deer/Snowman: Auto-detects 3D pixel positions in 30 seconds without manual pixel numbering."
        };
        t.troubleshooting = {
            "Duplicate coordinates detected: Room is too bright or camera exposure is overblowing pixel highlights.",
            "Missing pixels: Prop geometry is occluded from camera angle; reposition camera or capture multiple angles."
        };
        RegisterTopic(t);
    }

    // 6. AI Segment Anything Model (SAM) Submodel Detector
    {
        AIHelpTopic t;
        t.topicId = "SUBMODEL_DETECTOR";
        t.title = "AI Segment Anything Model (SAM) Submodel Detector";
        t.category = "Layout & Hardware";
        t.summary = "Employs computer vision segmentation and DBSCAN spatial density clustering to automatically discover geometric rings, spokes, stars, and contours on complex props.";
        t.workflowSteps = {
            "1. Select the parent prop model in your layout (e.g. Star, Spinner, MegaTree).",
            "2. Configure Cluster Radius (eps) and Minimum Points (minPts).",
            "3. Set SAM confidence threshold.",
            "4. Click '✨ Detect Submodels' to discover logical segments.",
            "5. Review detected submodel ranges in the results list.",
            "6. Click '💾 Export Submodel XML' to save directly to your model definition."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|               Submodel Segmentation Pipeline                 |\n"
            "+--------------------------------------------------------------+\n"
            "| [Prop 2D/3D Node Mesh] ---> [SAM Neural Feature Extraction]  |\n"
            "|                                    |                         |\n"
            "|                                    v                         |\n"
            "|                         [DBSCAN Density Cluster]             |\n"
            "|                                    |                         |\n"
            "|                                    v                         |\n"
            "|                  [Submodel Segments (Rings/Spokes)]          |\n"
            "|                                    |                         |\n"
            "|                                    v                         |\n"
            "|                     [Native Submodel XML Tags]               |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Cluster Radius (eps)", "Slider (1 - 50)", "Maximum Euclidean neighborhood radius for grouping adjacent nodes.", "Larger values join separated strands; smaller values split dense contours.", "15 for standard 50mm prop spacing; 25 for large displays.", "Values below 5 may create orphan single-node clusters."},
            {"Min Points (minPts)", "Spin Ctrl (1 - 20)", "Minimum node count required to constitute a valid submodel.", "Filters out noise or stray pixels from creating empty submodels.", "5 (default).", "Setting too high may ignore small inner star tips."},
            {"SAM Confidence", "Slider (50 - 100)", "Neural vision threshold for boundary line recognition.", "Higher values require higher certainty before creating a split.", "85% (default).", "Setting below 60% may over-segment smooth circles."}
        };
        t.examples = {
            "200-Node Star: Auto-partitions into 'Outer Points', 'Inner Star', and 'Center Core' submodels.",
            "Rosa Grande Spinner: Auto-detects 8 curved spiral arms and 4 concentric rings."
        };
        t.troubleshooting = {
            "Too many small submodels: Increase Cluster Radius (eps) to merge nearby clusters.",
            "Whole prop grouped into one submodel: Decrease Cluster Radius (eps) and increase SAM Confidence."
        };
        RegisterTopic(t);
    }

    // 7. AI Color Palette & Mood Theme Generator
    {
        AIHelpTopic t;
        t.topicId = "COLOR_PALETTES";
        t.title = "AI Color Palette & Mood Theme Generator";
        t.category = "Generative & Automation";
        t.summary = "Synthesizes harmonious multi-layer color palettes and gradients tailored to song mood, BPM tempo, genre, and sequencing aesthetics.";
        t.workflowSteps = {
            "1. Enter song title/artist or provide a custom free-form mood prompt.",
            "2. Select AI Service provider.",
            "3. Click '🎨 Generate Palette' to synthesize complementary color swatches.",
            "4. Preview color swatches and hex codes in the HTML result panel.",
            "5. Click 'Apply Palette' to load colors directly into xLights color palette bar."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|                 Color Palette Generation Flow                |\n"
            "+--------------------------------------------------------------+\n"
            "| [Song Title / Mood Prompt] ---> [Semantic Emotion Extractor] |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|                          [HSL Harmonic Color Theory]         |\n"
            "|                          (Triadic, Split-Comp, Analog)       |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|                        [Contrast & Saturation Curve]         |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|                       [Exported xLights Palette Bar]         |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Song Mode", "Radio Button", "Extracts mood, tempo, and theme from song metadata.", "Automates palette selection based on musical context.", "Enabled when working with musical sequences.", ""},
            {"Free Form Mode", "Radio Button", "Allows entering custom descriptive color prompts (e.g. 'Cyberpunk neon icy winter midnight').", "Gives exact control over desired visual aesthetics.", "Use for custom themes and non-musical displays.", ""}
        };
        t.examples = {
            "Carol of the Bells (Trans-Siberian Orchestra): Generates high-energy Electric Cyan, Arctic White, and Royal Cobalt.",
            "Let It Go: Generates crystalline ice blues, pastel magentas, and shimmering silvers."
        };
        t.troubleshooting = {
            "Generated colors look washed out on pixels: LED pixels produce more vivid colors with higher saturation; adjust prompt to specify 'vibrant' or 'saturated'."
        };
        RegisterTopic(t);
    }

    // 8. AI Intelligent Thermal & Safety Throttler
    {
        AIHelpTopic t;
        t.topicId = "THERMAL_SAFETY_THROTTLER";
        t.title = "AI Thermal & Current Load Safety Throttler";
        t.category = "Safety & Diagnostics";
        t.summary = "Simulates PSU wattage limits and thermal dissipation across the entire show timeline, detecting over-current incidents and non-destructively applying micro-dimming curves.";
        t.workflowSteps = {
            "1. Configure PSU Limit (Watts), Thermal Limit (°C), and Ambient Temperature.",
            "2. Click '⚡ Run Thermal Simulation' to scan sequence frames.",
            "3. Inspect thermal timeline graph and peak wattage incidents.",
            "4. Check/uncheck individual observations in the remediation checklist.",
            "5. Click '💡 Apply Selected Fix' or '💾 Export Remediation XML' to apply safety curves."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|                 Thermal Simulation Pipeline                  |\n"
            "+--------------------------------------------------------------+\n"
            "| [Frame Pixel Buffers] ---> [Current & Wattage Integration]   |\n"
            "|                                    |                         |\n"
            "|                                    v                         |\n"
            "|                        [Thermal Dissipation Model]           |\n"
            "|                                    |                         |\n"
            "|                                    v                         |\n"
            "|                      [Over-Limit Incident Detection]         |\n"
            "|                         /                     \\              |\n"
            "|                 (Within Limits)         (Over 350W / 70°C)   |\n"
            "|                       |                        |             |\n"
            "|                       v                        v             |\n"
            "|                 [Pass Audit]         [Micro-Dimming Curve]   |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"PSU Limit (Watts)", "Spin Ctrl", "Maximum continuous power draw before PSU protection trips.", "Sets the safety clamp threshold.", "350W for standard 350W PSU; 700W for dual-PSU setup.", "Setting above PSU physical rating risks controller shutdown."},
            {"Thermal Limit (°C)", "Spin Ctrl", "Maximum controller enclosure temperature.", "Triggers throttling when prolonged high-white frames cause heat build up.", "70°C (default).", "Plastic enclosures in warm climates should use 60°C."},
            {"Ambient Temp (°C)", "Spin Ctrl", "Expected outdoor show temperature.", "Affects heat dissipation rate.", "25°C summer / 0°C winter.", ""}
        };
        t.examples = {
            "All-White Strobe Burst: Clamps 462W spike down to 346W smoothly over 200ms without perceptible flicker."
        };
        t.troubleshooting = {
            "Simulation reports false overload: Check if pixel brightness is set to 30% or 50% in controller hardware."
        };
        RegisterTopic(t);
    }

    // 9. AI 3D Layout VR/AR Spatial Copilot
    {
        AIHelpTopic t;
        t.topicId = "VR_SPATIAL_COPILOT";
        t.title = "AI 3D Layout VR/AR Spatial Walkthrough & Clearance Copilot";
        t.category = "3D & Simulation";
        t.summary = "Validates 1:1 scale yard clearances, structural safety bounds, pedestrian walkways, and guy-wire collision cones in full 6-DoF 3D spatial space.";
        t.workflowSteps = {
            "1. Click '📐 Recalculate 3D Spatial Clearances' to compute prop bounding boxes.",
            "2. Review spatial clearance warnings and physical distance measurements.",
            "3. Ask natural language spatial queries in the chat copilot box.",
            "4. Click '📄 Export Clearance Report' to generate yard installation diagram."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|                 3D Spatial Clearance Check                   |\n"
            "+--------------------------------------------------------------+\n"
            "| [3D Yard Layout Models] ---> [3D OBB Bounding Volume Tree]   |\n"
            "|                                    |                         |\n"
            "|                                    v                         |\n"
            "|                     [Convex Hull Collision Query]            |\n"
            "|                       /                    \\                 |\n"
            "|              (Clearance < 3.0ft)     (Clearance >= 3.0ft)    |\n"
            "|                      |                        |              |\n"
            "|                      v                        v              |\n"
            "|             [Collision Warning]          [Safe Zone]         |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Walkthrough Speed", "Slider", "Virtual camera traversal speed in 3D scene.", "Allows fast navigation across large yards.", "1.0x (default).", ""},
            {"Safety Margin (ft)", "Spin Ctrl", "Minimum required buffer distance between props and physical obstacles.", "Triggers spatial warning if props violate buffer.", "3.0 ft (default).", "Guy wires on tall MegaTrees require at least 5.0 ft buffer."}
        };
        t.examples = {
            "MegaTree vs. Tree Branch: Detects MegaTree star colliding with Oak tree branch at 18ft elevation with 0.8ft clearance."
        };
        t.troubleshooting = {
            "Props floating or buried: Verify ground elevation contour map in layout settings."
        };
        RegisterTopic(t);
    }

    // 10. AI ESP32 Hardware Capability & Pinout Optimizer
    {
        AIHelpTopic t;
        t.topicId = "ESP32_PINOUT_OPTIMIZER";
        t.title = "AI ESP32 Hardware & DMA Pinout Optimizer";
        t.category = "Hardware & Controllers";
        t.summary = "Calculates safe RMT/I2S DMA GPIO allocations for ESP32 boards (QuinLED, DevKits, S3/C3/C6), avoiding boot-strapping traps and optimizing damping resistors and voltage sag.";
        t.workflowSteps = {
            "1. Select target ESP32 board profile (e.g. QuinLED Dig-Quad, ESP32-S3, DevKit V1).",
            "2. Specify number of pixel ports and nodes per port.",
            "3. Select protocol (WS2811, SK6812, GS8208) and feed wire length/gauge.",
            "4. Click '⚡ Run AI Pinout & Signal Optimization'.",
            "5. Review assigned safe GPIOs, inline resistor recommendations, and voltage drop.",
            "6. Export ready-to-flash JSON configs for ESPixelStick v4, WLED, or PlatformIO."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|               ESP32 Hardware & DMA Pin Allocator             |\n"
            "+--------------------------------------------------------------+\n"
            "|  [ESP32 Chip] ----> [Strapping Pin Filter (0, 2, 12, 15)]    |\n"
            "|                          |                                   |\n"
            "|                          v                                   |\n"
            "|               [RMT / I2S DMA Engine]                         |\n"
            "|             (Parallel Multi-Port Output)                     |\n"
            "|                          |                                   |\n"
            "|                          v                                   |\n"
            "|      [74HCT245 Level Shifter] + [33-249 Ohm Resistor]        |\n"
            "|                          |                                   |\n"
            "|                          v                                   |\n"
            "|         [Pixel Strip Output (Zero Glitch / Jitter)]          |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Target Board", "Choice Dropdown", "Selects physical ESP32 board profile.", "Loads exact hardware pinout matrix and buffer topology.", "QuinLED Dig-Quad (default).", ""},
            {"Number of Ports", "Spin Ctrl", "Quantity of independent parallel pixel output lines.", "Determines DMA and RMT channel allocation.", "1 to 16 ports.", "High port counts (>8) on basic ESP32 switch to I2S parallel DMA."},
            {"Feed Wire Distance", "Spin / Choice", "Physical wire length and AWG from controller to first pixel.", "Calculates signal ringing damping resistor and DC voltage drop.", "5.0m @ 18 AWG.", "Lines >3m require 249-ohm damping resistor."}
        };
        t.examples = {
            "QuinLED Dig-Quad 4-Port Setup: Assigns GPIO 16, 3, 1, 4 with level-shifted 74HCT245 buffers, avoiding MTDI strapping pin GPIO 12."
        };
        t.troubleshooting = {
            "ESP32 fails to boot with pixels connected: Pixels wired to GPIO 0 or 12 pull strapping pins LOW during power-on; move to GPIO 16, 4, 13, 14."
        };
        RegisterTopic(t);
    }

    // 11. AI Predictive WiFi Packet Loss Concealment
    {
        AIHelpTopic t;
        t.topicId = "WIFI_PACKET_INTERPOLATOR";
        t.title = "AI Predictive WiFi Packet Loss Concealment & Auto-Interpolator";
        t.category = "Wireless & Networking";
        t.summary = "Synthesizes smooth replacement lighting frames during 2.4GHz WiFi packet drops, eliminating pixel stutter and blackout glitches in live shows.";
        t.workflowSteps = {
            "1. Enable master interpolation toggle.",
            "2. Select interpolation model (Adaptive Lookahead Bezier, Cubic Hermite, or Linear).",
            "3. Configure lookahead buffer window (2–10 frames).",
            "4. Run loss stress benchmark to measure frame recovery rate and throughput.",
            "5. Click 'Apply Engine Settings' to activate for real-time DDP/E1.31 streaming."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|             WiFi Packet Loss Concealment Pipeline            |\n"
            "+--------------------------------------------------------------+\n"
            "| [WiFi DDP Stream] ---> [Packet Gap Detector]                 |\n"
            "|                             |                                |\n"
            "|             [Packet Dropped Detected: Frame N+1]             |\n"
            "|                             |                                |\n"
            "|                             v                                |\n"
            "|           [Adaptive Lookahead Bezier Interpolator]           |\n"
            "|             (P0: Frame N-1, P1: Frame N, P2: Frame N+2)      |\n"
            "|                             |                                |\n"
            "|                             v                                |\n"
            "|           [Reconstructed Frame N+1 Injected to DMA]          |\n"
            "|                  (Smooth Seamless Visual Flow)               |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Master Toggle", "Checkbox", "Enables or disables packet loss frame synthesis.", "When enabled, drops are smoothly filled instead of holding frozen frames.", "Checked (Enabled).", ""},
            {"Interpolation Model", "Choice Dropdown", "Mathematical curve used to synthesize missing frames.", "Adaptive Bezier produces highest visual quality with minimal latency.", "Adaptive Lookahead Bezier (default).", ""},
            {"Lookahead Window", "Spin Ctrl", "Depth of circular frame history buffer.", "Balances interpolation accuracy against memory consumption.", "4 frames (default).", ""}
        };
        t.examples = {
            "15% Packet Drop on Roofline: Automatically generates 60 missing frames over 400-frame test with 0% visible flicker."
        };
        t.troubleshooting = {
            "Rapid motion tearing: Increase lookahead window to 6 frames for high-velocity chase effects."
        };
        RegisterTopic(t);
    }

    // 12. AI Neural Sparse FSEQ Optimizer
    {
        AIHelpTopic t;
        t.topicId = "SPARSE_FSEQ_OPTIMIZER";
        t.title = "AI Neural Sparse .FSEQ & SD Alignment Optimizer";
        t.category = "Show & Media Management";
        t.summary = "Prunes unneeded show channels per controller and aligns data blocks to 4KB FAT32 SD card cluster boundaries, preventing playback buffer underruns.";
        t.workflowSteps = {
            "1. Enter sequence name and target controller.",
            "2. Specify total show channels and target controller's assigned channels.",
            "3. Select 4096-byte FAT32 cluster alignment and enable delta compression.",
            "4. Click '⚡ Run AI Sparse FSEQ Compression'.",
            "5. Review bandwidth reduction and SPI read latency check.",
            "6. Export optimized sparse .fseq file directly for ESP32 SD card playback."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|                 Sparse FSEQ Compression Flow                 |\n"
            "+--------------------------------------------------------------+\n"
            "| [Full 96K-Channel .FSEQ] ---> [Sparse Channel Filter]        |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|                         [Delta Frame Quantization]           |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|                    [4KB FAT32 Cluster Sector Padding]        |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|             [Exported Sparse .FSEQ (80% Size Reduction)]     |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Total Channels", "Spin Ctrl", "Full sequence total show universe channels.", "Used to calculate compression ratio.", "96000 channels (default).", ""},
            {"Target Channels", "Spin Ctrl", "Channels actually connected to this physical ESP32.", "All other unmapped channels are pruned from the file.", "4800 channels (default).", ""},
            {"FAT32 Alignment", "Choice Dropdown", "Pads frame records to exact SD cluster multiples.", "Eliminates multi-sector seek latency on SPI SD card readers.", "4096 Bytes (4KB Cluster - Recommended).", ""}
        };
        t.examples = {
            "96K Channel Sequence for 4.8K MegaTree: Compresses 13.8 MB uncompressed .fseq down to 2.1 MB sparse file with 0.8ms SPI read time."
        };
        t.troubleshooting = {
            "FPP Remote stuttering on dense matrices: Ensure 4KB cluster alignment is selected and use high-speed SanDisk/Samsung Class 10 SD card."
        };
        RegisterTopic(t);
    }

    // 13. AI ESP32 Telemetry & Predictive Throttler
    {
        AIHelpTopic t;
        t.topicId = "ESP32_TELEMETRY_THROTTLER";
        t.title = "AI ESP32 Live Telemetry & Predictive Safety Throttler";
        t.category = "Hardware & Controllers";
        t.summary = "Ingests live voltage, current, and CPU temperature telemetry from ESP32 controllers, forecasting brownouts and fuse trips 10-30s in advance with micro-dimming overrides.";
        t.workflowSteps = {
            "1. Enter controller IP address and nominal voltage (12V/5V/24V).",
            "2. Set maximum continuous current and safe junction temperature thresholds.",
            "3. Enable AI autonomous micro-dimming throttler.",
            "4. Poll live telemetry or simulate a current surge.",
            "5. Review predictive alerts and time-to-failure forecasts.",
            "6. Send DDP micro-dimming command to stabilize power rails without stopping the show."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|               Predictive Telemetry Safety Loop               |\n"
            "+--------------------------------------------------------------+\n"
            "| [INA219 / ADC Sensors] ---> [Voltage Sag / Current Rate dI/dt]|\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|                   [Predictive Failure Estimator]             |\n"
            "|                 (Brownout Forecast in ~15 Seconds)           |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|                 [DDP Micro-Dimming Command Scale]            |\n"
            "|                       (-20% Brightness Trim)                 |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|              [Power Rail Stabilized / Fuse Protected]        |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Nominal Voltage", "Spin Ctrl", "Power supply rated output voltage.", "Used as baseline for brownout threshold calculation.", "12.0 V (default).", ""},
            {"Max Current (Amps)", "Spin Ctrl", "Fuse rating or maximum power supply output capacity.", "Surges exceeding 88% trigger predictive current alarm.", "30.0 A (default).", ""},
            {"Auto-Throttle", "Checkbox", "Automatically dispatches DDP brightness scale down commands.", "Prevents MCU brownouts without human intervention.", "Checked (Enabled).", ""}
        };
        t.examples = {
            "Dig-Octa Full White Surge: Current ramps to 34.5A; AI detects rate and issues -25% micro-dimming within 100ms, preventing 30A fuse blowout."
        };
        t.troubleshooting = {
            "Frequent voltage sag alarms: Check power injection wiring gauge and PSU terminal screw tightness."
        };
        RegisterTopic(t);
    }

    // 14. AI Controller Auto-Mapper & Discovery
    {
        AIHelpTopic t;
        t.topicId = "CONTROLLER_AUTO_MAPPER";
        t.title = "AI Universal Controller Discovery & Semantic Auto-Mapper";
        t.category = "Hardware & Controllers";
        t.summary = "On-demand network subnet scanner discovering ESPixelStick, WLED, FPP, and Kulp controllers, with intelligent layout-to-port mapping and full multi-level Undo/Redo.";
        t.workflowSteps = {
            "1. Enter subnet CIDR range (e.g. 192.168.1.0/24).",
            "2. Click '🔍 Scan Subnet On-Demand' (strictly on-demand, no background polling).",
            "3. Review discovered hardware controllers and proposed model bindings in the checklist.",
            "4. Check or uncheck individual proposals as desired.",
            "5. Click '✓ Apply Approved Mappings' to commit universe/channel bindings.",
            "6. Use '↶ Undo Mapping' anytime to instantly revert all changes."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|             On-Demand Discovery & Reversible Mapper          |\n"
            "+--------------------------------------------------------------+\n"
            "| [User Clicks 'Scan Subnet'] ---> [mDNS / SSDP Discovery]     |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|                     [Discovered ESP32/FPP Hardware]          |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|                 [Layout-to-Port Geometric Auto-Matcher]      |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|              [Pre-Execution Checklist Review Table]          |\n"
            "|                                       |                      |\n"
            "|                                       v                      |\n"
            "|          [Apply Mappings with Multi-Level Undo / Redo]       |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Subnet Range", "Text Input", "IP subnet range to query for mDNS/SSDP broadcasts.", "Defines network search boundary.", "192.168.1.0/24 (default).", ""},
            {"Pre-Execution Checklist", "List Control", "Interactive table showing proposed model-to-controller port mappings.", "Allows cherry-picking which bindings to approve.", "All proposals approved by default.", ""},
            {"Undo Mapping", "Button", "Rolls back layout model bindings to exact previous state.", "Full redundancy and zero risk of accidental misconfiguration.", "Available after applying mappings.", ""}
        };
        t.examples = {
            "Subnet Discovery: Discovers ESPixelStick (MegaTree), WLED (Roofline), and FPP (Matrix), auto-assigning 5 layout models to 14 ports in 1 click."
        };
        t.troubleshooting = {
            "Controller not found in scan: Ensure controller is on the same VLAN/subnet and mDNS broadcast is allowed across WiFi access points."
        };
        RegisterTopic(t);
    }

    // 15. AI FreeRTOS Multi-Core Affinity Optimizer
    {
        AIHelpTopic t;
        t.topicId = "FREERTOS_AFFINITY_OPTIMIZER";
        t.title = "AI ESP32 FreeRTOS Multi-Core Affinity & Task Scheduler";
        t.category = "Hardware & Controllers";
        t.summary = "Isolates high-speed DMA pixel output on Core 1 while pinning WiFi/LwIP stacks to Core 0, eliminating interrupt collisions and watchdog timer (WDT) resets.";
        t.workflowSteps = {
            "1. Select chip architecture (Dual-core Xtensa vs Single-core RISC-V).",
            "2. Configure active tasks (DDP UDP listener, SD FPP playback, Web/OTA, Telemetry).",
            "3. Specify number of pixel ports and pixel density.",
            "4. Click '⚡ Compute FreeRTOS Task Allocations'.",
            "5. Review task priorities, stack RAM allocations, and WDT safety margin.",
            "6. Export ready-to-compile FreeRTOS C++ code and platformio.ini build flags."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|             FreeRTOS Dual-Core Affinity Architecture         |\n"
            "+--------------------------------------------------------------+\n"
            "|          CORE 0: PROTOCOL & SYSTEM SERVICES                  |\n"
            "|  - TaskDdpReceiver       (Priority 18, 4KB Stack, UDP Socket)|\n"
            "|  - TaskAsyncWebServer    (Priority 4,  4KB Stack, REST/OTA)  |\n"
            "|  - TaskTelemetryMonitor  (Priority 2,  3KB Stack, Syslog)    |\n"
            "+--------------------------------------------------------------+\n"
            "|          CORE 1: REAL-TIME PIXEL GENERATION                 |\n"
            "|  - TaskPixelEngine       (Priority 22, 6KB Stack, RMT/I2S)   |\n"
            "|  - TaskSdCardFseqReader  (Priority 12, 8KB Stack, FAT32 SPI) |\n"
            "+--------------------------------------------------------------+\n"
            "| Result: 0% DMA Jitter Interruption from WiFi PHY Bursts      |\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Target Architecture", "Choice Dropdown", "Selects dual-core Xtensa or single-core RISC-V.", "Determines whether core pinning or dynamic prioritization is used.", "Dual-Core Xtensa LX6/LX7 (default).", ""},
            {"DDP UDP Listener", "Checkbox", "Allocates high-priority socket listener task on Core 0.", "Ensures low-latency packet reception.", "Checked (Enabled).", ""},
            {"SD FPP Playback Task", "Checkbox", "Allocates 8KB stack reader task on Core 1.", "Prevents SD SPI FAT32 reads from choking WiFi stacks.", "Checked (Enabled).", ""}
        };
        t.examples = {
            "8-Port High Density Controller: Allocates 24KB total FreeRTOS stack heap with 88.5% WDT margin, eliminating frame drops during heavy Web UI browsing."
        };
        t.troubleshooting = {
            "Guru Meditation Error / Watchdog Reset: Increase stack size for TaskSdCardFseqReader from 4KB to 8KB to handle complex FAT32 folder hierarchies."
        };
        RegisterTopic(t);
    }

    // 16. AI Distributed FPP Log Self-Healing Agent
    {
        AIHelpTopic t;
        t.topicId = "FPP_LOG_SELF_HEALING";
        t.title = "AI Distributed FPP & ESPixelStick Log Self-Healing Agent";
        t.category = "Diagnostics & Validation";
        t.summary = "Fleet-wide syslog & fppd.log error ingestion with natural-language root cause diagnosis and 1-click self-healing remediation.";
        t.workflowSteps = {
            "1. Click '🔄 Poll Fleet Controller Logs' or ingest live syslog stream.",
            "2. AI diagnoses errors, buffer underruns, packet fragmentation, and WiFi sleep latency.",
            "3. Review diagnosed problems and proposed 1-click healing actions in the table.",
            "4. Click '⚡ Apply 1-Click Fleet Remediation' to execute fixes across fleet.",
            "5. Use '↶ Undo Fix' to revert any applied remediations."
        };
        t.diagram =
            "+--------------------------------------------------------------+\n"
            "|             Distributed Log Ingestion & Self-Healing         |\n"
            "+--------------------------------------------------------------+\n"
            "| [FPP fppd.log / ESPixelStick Syslog] ---> [Log Ingestion]    |\n"
            "|                                                  |           |\n"
            "|                                                  v           |\n"
            "|                                    [AI Diagnostic Engine]    |\n"
            "|                              (SD Timeout / WiFi Sleep Spikes)|\n"
            "|                                                  |           |\n"
            "|                                                  v           |\n"
            "|                                 [1-Click Remediation Planner]|\n"
            "|                                                  |           |\n"
            "|                                                  v           |\n"
            "|                              [Automated Fleet API Patching]  |\n"
            "|                              (Sparse FSEQ / DDP MTU / PS-Off)|\n"
            "+--------------------------------------------------------------+";
        t.options = {
            {"Poll Fleet Logs", "Button", "Fetches latest syslog and fppd.log entries across all active controllers.", "Populates raw log viewer and triggers diagnosis.", "Interactive click.", ""},
            {"Apply 1-Click Remediation", "Button", "Executes recommended API patches across all affected controllers.", "Fixes buffer underruns and network fragmentation instantly.", "Interactive click with Undo support.", ""},
            {"Undo Fix", "Button", "Reverts applied controller configuration changes back to previous state.", "Provides safe fallback testing.", "Available after remediation.", ""}
        };
        t.examples = {
            "FPP Remote SD Timeout: Diagnoses uncompressed FSEQ read latency and automatically issues sparse 4KB-aligned re-export command."
        };
        t.troubleshooting = {
            "Syslog messages not arriving: Verify UDP port 514 is open and controller syslog target IP is set to xLights host IP."
        };
        RegisterTopic(t);
    }

    s_initialized = true;
}

const AIHelpTopic* AIHelpContentRegistry::GetTopic(const std::string& topicId) {
    Initialize();
    auto& reg = GetRegistry();
    auto it = reg.find(topicId);
    if (it != reg.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<AIHelpTopic> AIHelpContentRegistry::GetAllTopics() {
    Initialize();
    std::vector<AIHelpTopic> topics;
    auto& reg = GetRegistry();
    for (const auto& pair : reg) {
        topics.push_back(pair.second);
    }
    return topics;
}

std::vector<std::string> AIHelpContentRegistry::GetCategories() {
    Initialize();
    std::vector<std::string> cats;
    auto& reg = GetRegistry();
    for (const auto& pair : reg) {
        if (std::find(cats.begin(), cats.end(), pair.second.category) == cats.end()) {
            cats.push_back(pair.second.category);
        }
    }
    return cats;
}

std::vector<AIHelpTopic> AIHelpContentRegistry::GetTopicsByCategory(const std::string& category) {
    Initialize();
    std::vector<AIHelpTopic> results;
    auto& reg = GetRegistry();
    for (const auto& pair : reg) {
        if (pair.second.category == category) {
            results.push_back(pair.second);
        }
    }
    return results;
}

std::vector<AIHelpTopic> AIHelpContentRegistry::Search(const std::string& query) {
    Initialize();
    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), [](unsigned char c){ return std::tolower(c); });

    std::vector<AIHelpTopic> matches;
    auto& reg = GetRegistry();
    for (const auto& pair : reg) {
        std::string titleLower = pair.second.title;
        std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), [](unsigned char c){ return std::tolower(c); });
        
        std::string summaryLower = pair.second.summary;
        std::transform(summaryLower.begin(), summaryLower.end(), summaryLower.begin(), [](unsigned char c){ return std::tolower(c); });

        if (titleLower.find(qLower) != std::string::npos || summaryLower.find(qLower) != std::string::npos || pair.first.find(qLower) != std::string::npos) {
            matches.push_back(pair.second);
        }
    }
    return matches;
}

void AIHelpContentRegistry::RegisterTopic(AIHelpTopic topic) {
    GetRegistry()[topic.topicId] = std::move(topic);
}

} // namespace xLights::AI
