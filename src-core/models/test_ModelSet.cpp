/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "src-core/models/ModelSet.h"
#include "src-core/models/ModelSetManager.h"
#include <iostream>
#include <cassert>
#include <algorithm>

int main() {
    std::cout << "[Unit Test] Running ModelSet & ModelSetManager verification..." << std::endl;

    // Test 1: ModelSet member management
    {
        ModelSet s("TreeAndStar");
        s.AddMember("MegaTree");
        s.AddMember("StarTop");
        assert(s.HasMember("MegaTree"));
        assert(s.HasMember("StarTop"));
        assert(!s.HasMember("Arch"));
        assert(s.GetMembers().size() == 2);

        s.RenameMember("StarTop", "StarTopper");
        assert(s.HasMember("StarTopper"));
        assert(!s.HasMember("StarTop"));

        s.RemoveMember("MegaTree");
        assert(!s.HasMember("MegaTree"));
        assert(s.GetMembers().size() == 1);
        std::cout << " -> Test 1 (ModelSet Add, Remove, Rename): PASSED" << std::endl;
    }

    // Test 2: ModelSet XML round-trip
    {
        ModelSet s1("RooflineSet");
        s1.AddMember("GableLeft");
        s1.AddMember("GableRight");
        s1.AddMember("FasciaMain");

        pugi::xml_document doc;
        auto node = doc.append_child("modelSet");
        s1.Save(node);

        ModelSet s2;
        s2.Load(node);
        assert(s2.GetName() == "RooflineSet");
        assert(s2.GetMembers().size() == 3);
        assert(s2.HasMember("GableLeft"));
        assert(s2.HasMember("GableRight"));
        assert(s2.HasMember("FasciaMain"));
        std::cout << " -> Test 2 (ModelSet XML Serialization): PASSED" << std::endl;
    }

    // Test 3: ModelSetManager exclusivity & lookup
    {
        ModelSetManager mgr;
        auto* setA = mgr.CreateSet({"Prop1", "Prop2"}, "SetA");
        auto* setB = mgr.CreateSet({"Prop3", "Prop4"}, "SetB");
        assert(setA != nullptr);
        assert(setB != nullptr);

        assert(mgr.GetSetContaining("Prop1") == setA);
        assert(mgr.GetSetContaining("Prop4") == setB);
        assert(mgr.GetSetContaining("UnlinkedProp") == nullptr);

        // Add Prop1 to SetB (automatically handles index)
        mgr.AddMember(setB, "Prop1");
        assert(mgr.GetSetContaining("Prop1") == setB);
        std::cout << " -> Test 3 (ModelSetManager Single-Set Exclusivity): PASSED" << std::endl;
    }

    std::cout << "[Unit Test] ModelSet & ModelSetManager ALL TESTS PASSED!" << std::endl;
    return 0;
}
