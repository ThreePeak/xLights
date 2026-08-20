/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/ai/AISnapshotHistoryManager.h"
#include <iostream>
#include <cassert>

using namespace xLights::AI;

int main() {
    std::cout << "[Unit Test] Running AISnapshotHistoryManager verification..." << std::endl;

    auto& manager = AISnapshotHistoryManager::Instance();
    manager.ClearHistory();
    manager.SetEnabled(true);
    manager.SetMaxHistoryCapacity(10);

    // Test 1: Push Snapshots and History State Traversal
    {
        uint32_t id1 = manager.PushSnapshot("Step 1: MegaTree Bars", "<tree_bars/>", "AudioChoreographerAI");
        uint32_t id2 = manager.PushSnapshot("Step 2: Arch Shockwave", "<arch_shockwave/>", "AudioChoreographerAI");

        assert(manager.GetAllSnapshots().size() == 2);
        assert(manager.GetCurrentActiveSnapshotId() == id2);

        // Jump back to step 1
        std::string xml;
        bool ok = manager.JumpToSnapshot(id1, xml);
        assert(ok);
        assert(xml == "<tree_bars/>");
        assert(manager.GetCurrentActiveSnapshotId() == id1);
        std::cout << " -> Test 1 (Push Snapshots and History State Traversal): PASSED" << std::endl;
    }

    // Test 2: Granular Item Checklist Export & Import
    {
        GranularSnapshotItem item1{"item_01", "EFFECT", "MegaTree Bars", "<bars_config/>", true};
        GranularSnapshotItem item2{"item_02", "MODEL", "Custom Snowflake", "<snowflake_mesh/>", true};

        uint32_t id3 = manager.PushSnapshot("Step 3: Added Snowflake", "<full_show/>", "CustomPropDesigner", {item1, item2});

        // Export only item2 (the snowflake)
        std::string pkgJson = manager.ExportPackage(id3, {"item_02"});
        assert(pkgJson.find("Custom Snowflake") != std::string::npos);

        // Import package
        std::string importedXml;
        bool ok = manager.ImportPackage(pkgJson, importedXml);
        assert(ok);
        assert(importedXml == "<full_show/>");
        std::cout << " -> Test 2 (Granular Item Checklist Export & Import): PASSED" << std::endl;
    }

    // Test 3: Enable / Disable Zero Overhead Guardrail
    {
        manager.SetEnabled(false);
        uint32_t disabledId = manager.PushSnapshot("Step Disabled", "<none/>");
        assert(disabledId == 0); // No record added when disabled
        manager.SetEnabled(true);
        std::cout << " -> Test 3 (Enable / Disable Zero Overhead Guardrail): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] AISnapshotHistoryManager ALL TESTS PASSED!" << std::endl;
    return 0;
}
