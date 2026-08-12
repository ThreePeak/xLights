/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "effects/EffectPresetManager.h"
#include "utils/ExternalHooks.h"
#include "XmlSerializer/BaseSerializingVisitor.h"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <list>

namespace {
// Characters wxWidgets forbids in Windows filenames (wxPATH_WIN). Kept as a
// literal so the core stays wx-free while preserving cross-platform parity
// with the desktop name fixup this manager replaced.
constexpr const char* kForbiddenFilenameChars = "/\\?*:|\"<>";

// Strips forbidden filename characters from a name. Returns true if the name
// changed. Mirrors the old EffectTreeDialog::FixName behaviour.
bool FixPresetName(std::string& name)
{
    std::string cleaned;
    cleaned.reserve(name.size());
    for (char ch : name) {
        if (std::strchr(kForbiddenFilenameChars, ch) == nullptr) {
            cleaned.push_back(ch);
        }
    }
    bool changed = (cleaned != name);
    name = std::move(cleaned);
    return changed;
}

// Split `s` on `separator`. The preset paths fed to FindItemByPath use no
// escaping, so a plain split matches the wxSplit behaviour previously relied
// upon.
std::vector<std::string> SplitString(const std::string& s, char separator)
{
    std::vector<std::string> parts;
    std::string current;
    for (char ch : s) {
        if (ch == separator) {
            parts.push_back(current);
            current.clear();
        } else {
            current.push_back(ch);
        }
    }
    parts.push_back(current);
    return parts;
}
} // namespace

// ===========================================================================
// EffectPreset
// ===========================================================================

EffectPreset::EffectPreset(const std::string& name, const std::string& settings,
                           const std::string& version, const std::string& xLightsVersion,
                           EffectPresetGroup* parent)
    : EffectPresetItem(name, parent)
    , _settings(settings)
    , _version(version)
    , _xLightsVersion(xLightsVersion)
{
}

EffectPreset::EffectPreset(pugi::xml_node node, EffectPresetGroup* parent)
    : EffectPresetItem(node.attribute("name").as_string(""), parent)
    , _settings(node.attribute("settings").as_string(""))
    , _version(node.attribute("version").as_string(""))
    , _xLightsVersion(node.attribute("xLightsVersion").as_string("4.0"))
{
}

void EffectPreset::Save(BaseSerializingVisitor& visitor) const
{
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("name", _name);
    attrs.Add("settings", _settings);
    attrs.Add("version", _version);
    attrs.Add("xLightsVersion", _xLightsVersion);
    visitor.WriteOpenTag("effect", attrs, /*selfClose=*/true);
}

nlohmann::json EffectPreset::ToJson() const
{
    return {
        {"type", "effect"},
        {"name", _name},
        {"settings", _settings},
        {"version", _version},
        {"xLightsVersion", _xLightsVersion}
    };
}

namespace {
int ToInt(const std::string& s)
{
    return (int)std::strtol(s.c_str(), nullptr, 10);
}
} // namespace

// Ported from EffectTreeDialog::ParseLayers. The "AC" variant is not
// represented separately here — iPad/desktop callers that want the
// "N - AC" label can reconstruct it; this returns the raw row count.
int EffectPreset::GetLayerCount() const
{
    if (_settings.empty())
        return 0;

    if (_settings.find('\t') != std::string::npos) {
        int start = 9999;
        int end = -1;
        bool cf1 = false;

        for (const std::string& line : SplitString(_settings, '\n')) {
            std::vector<std::string> efdata = SplitString(line, '\t');
            if (efdata.empty())
                continue;
            if (efdata[0] == "CopyFormat1") {
                cf1 = true;
            } else if (efdata[0] == "CopyFormatAC") {
                if (efdata.size() > 8) {
                    start = ToInt(efdata[7]);
                    end = ToInt(efdata[8]);
                }
                break;
            } else {
                if (cf1 && efdata.size() > 5 && efdata[0] != "None") {
                    int row = ToInt(efdata[5]);
                    if (row < start) start = row;
                    if (row > end) end = row;
                } else if (!cf1 && efdata.size() > 2 && efdata[0] != "None") {
                    int row = ToInt(efdata[efdata.size() - 2]);
                    if (row < start) start = row;
                    if (row > end) end = row;
                }
            }
        }
        return (end != -1) ? (end - start + 1) : 0;
    }

    // effect1,effect2,blend,settings ...
    std::vector<std::string> efdata = SplitString(_settings, ',');
    if (efdata.size() < 2)
        return 0;
    int res = 0;
    if (efdata[0] != "None") res++;
    if (efdata[1] != "None") res++;
    return res;
}

// Ported from EffectTreeDialog::ParseDuration.
int EffectPreset::GetDurationMS() const
{
    if (_settings.empty())
        return 0;

    int minstart = 99999999;
    int maxend = -99999999;

    if (_settings.find('\t') != std::string::npos) {
        for (const std::string& line : SplitString(_settings, '\n')) {
            std::vector<std::string> efdata = SplitString(line, '\t');
            if (efdata.empty())
                continue;
            if (efdata[0] == "CopyFormat1") {
                // nothing to do
            } else if (efdata.size() > 10 && efdata[0] == "CopyFormatAC") {
                int start = ToInt(efdata[9]);
                int end = ToInt(efdata[10]);
                minstart = std::min(start, minstart);
                maxend = std::max(end, maxend);
                break;
            } else if (efdata.size() > 4 && efdata[0] != "None") {
                int start = ToInt(efdata[3]);
                int end = ToInt(efdata[4]);
                minstart = std::min(start, minstart);
                maxend = std::max(end, maxend);
            }
        }
    } else {
        std::vector<std::string> efdata = SplitString(_settings, ',');
        if (efdata.size() < 5)
            return 0;
        minstart = ToInt(efdata[3]);
        maxend = ToInt(efdata[4]);
    }

    if (minstart == 99999999)
        return 0;
    return maxend - minstart;
}

// ===========================================================================
// EffectPresetGroup
// ===========================================================================

EffectPresetGroup::EffectPresetGroup(const std::string& name, EffectPresetGroup* parent)
    : EffectPresetItem(name, parent)
{
}

EffectPresetGroup::EffectPresetGroup(pugi::xml_node node, EffectPresetGroup* parent)
    : EffectPresetItem(node.attribute("name").as_string(""), parent)
{
    LoadChildren(node);
}

void EffectPresetGroup::LoadChildren(pugi::xml_node node)
{
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        std::string_view childName = child.name();
        if (childName == "effectGroup") {
            _children.push_back(std::make_unique<EffectPresetGroup>(child, this));
        } else if (childName == "effect") {
            // Only load presets that have tab-separated settings (valid format)
            std::string settings = child.attribute("settings").as_string("");
            if (settings.find('\t') != std::string::npos) {
                _children.push_back(std::make_unique<EffectPreset>(child, this));
            }
        }
    }
}

void EffectPresetGroup::Save(BaseSerializingVisitor& visitor) const
{
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("name", _name);
    visitor.WriteOpenTag("effectGroup", attrs);
    for (const auto& child : _children) {
        child->Save(visitor);
    }
    visitor.WriteCloseTag();
}

nlohmann::json EffectPresetGroup::ToJson() const
{
    nlohmann::json childrenArr = nlohmann::json::array();
    for (const auto& child : _children) {
        childrenArr.push_back(child->ToJson());
    }
    return {
        {"type", "group"},
        {"name", _name},
        {"children", childrenArr}
    };
}

void EffectPresetGroup::LoadChildrenFromJson(const nlohmann::json& j)
{
    if (!j.contains("children") || !j["children"].is_array())
        return;

    for (const auto& child : j["children"]) {
        std::string type = child.value("type", "");
        if (type == "group") {
            auto group = std::make_unique<EffectPresetGroup>(child.value("name", ""), this);
            group->LoadChildrenFromJson(child);
            _children.push_back(std::move(group));
        } else if (type == "effect") {
            std::string settings = child.value("settings", "");
            if (settings.find('\t') != std::string::npos) {
                _children.push_back(std::make_unique<EffectPreset>(
                    child.value("name", ""),
                    settings,
                    child.value("version", ""),
                    child.value("xLightsVersion", "4.0"),
                    this));
            }
        }
    }
}

EffectPresetItem* EffectPresetGroup::AddChild(std::unique_ptr<EffectPresetItem> child)
{
    child->SetParent(this);
    _children.push_back(std::move(child));
    return _children.back().get();
}

EffectPresetItem* EffectPresetGroup::InsertChildAfter(std::unique_ptr<EffectPresetItem> child,
                                                       EffectPresetItem* after)
{
    child->SetParent(this);
    if (after == nullptr) {
        // Prepend
        auto* ptr = child.get();
        _children.insert(_children.begin(), std::move(child));
        return ptr;
    }
    for (auto it = _children.begin(); it != _children.end(); ++it) {
        if (it->get() == after) {
            ++it;
            auto* ptr = child.get();
            _children.insert(it, std::move(child));
            return ptr;
        }
    }
    // after not found — append
    return AddChild(std::move(child));
}

std::unique_ptr<EffectPresetItem> EffectPresetGroup::RemoveChild(EffectPresetItem* child)
{
    for (auto it = _children.begin(); it != _children.end(); ++it) {
        if (it->get() == child) {
            std::unique_ptr<EffectPresetItem> removed = std::move(*it);
            _children.erase(it);
            removed->SetParent(nullptr);
            return removed;
        }
    }
    return nullptr;
}

EffectPresetItem* EffectPresetGroup::FindChildByName(const std::string& name) const
{
    for (const auto& child : _children) {
        if (child->GetName() == name) {
            return child.get();
        }
    }
    return nullptr;
}

bool EffectPresetGroup::HasChildNamed(const std::string& name) const
{
    return FindChildByName(name) != nullptr;
}

// ===========================================================================
// EffectPresetManager
// ===========================================================================

EffectPresetManager::EffectPresetManager()
    : _root("", nullptr)
{
}


void EffectPresetManager::Load(pugi::xml_node effectsNode)
{
    Reset();
    if (!effectsNode)
        return;

    _version = effectsNode.attribute("version").as_string("0000");

    for (pugi::xml_node child = effectsNode.first_child(); child; child = child.next_sibling()) {
        std::string_view childName = child.name();
        if (childName == "effectGroup") {
            _root.AddChild(std::make_unique<EffectPresetGroup>(child, &_root));
        } else if (childName == "effect") {
            std::string settings = child.attribute("settings").as_string("");
            if (settings.find('\t') != std::string::npos) {
                _root.AddChild(std::make_unique<EffectPreset>(child, &_root));
            }
        }
    }
}

void EffectPresetManager::Save(BaseSerializingVisitor& visitor) const
{
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("version", _version);
    visitor.WriteOpenTag("effects", attrs);
    for (const auto& child : _root.GetChildren()) {
        child->Save(visitor);
    }
    visitor.WriteCloseTag();
}

void EffectPresetManager::LoadFromJson(const nlohmann::json& j)
{
    Reset();
    if (!j.is_object())
        return;

    _version = j.value("version", "0000");
    _root.LoadChildrenFromJson(j);
}

nlohmann::json EffectPresetManager::SaveToJson() const
{
    nlohmann::json childrenArr = nlohmann::json::array();
    for (const auto& child : _root.GetChildren()) {
        childrenArr.push_back(child->ToJson());
    }
    return {
        {"version", _version},
        {"children", childrenArr}
    };
}

bool EffectPresetManager::LoadJsonFile(const std::string& filepath)
{
    if (!FileExists(filepath))
        return false;

    ObtainAccessToURL(filepath);
    std::ifstream ifs(filepath);
    if (!ifs.is_open())
        return false;

    try {
        nlohmann::json j = nlohmann::json::parse(ifs);
        LoadFromJson(j);
        return true;
    } catch (...) {
        return false;
    }
}

bool EffectPresetManager::SaveJsonFile(const std::string& filepath) const
{
    ObtainAccessToURL(filepath, true);
    std::ofstream ofs(filepath);
    if (!ofs.is_open())
        return false;

    try {
        nlohmann::json j = SaveToJson();
        ofs << j.dump(2);
        return ofs.good();
    } catch (...) {
        return false;
    }
}

void EffectPresetManager::Reset()
{
    _root = EffectPresetGroup("", nullptr);
    _version.clear();
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

std::vector<std::string> EffectPresetManager::GetAllPresetPaths(const std::string& separator) const
{
    std::vector<std::string> result;
    CollectPresetPaths(_root, "", separator, result);
    return result;
}

void EffectPresetManager::CollectPresetPaths(const EffectPresetGroup& group,
                                              const std::string& prefix,
                                              const std::string& separator,
                                              std::vector<std::string>& result) const
{
    for (const auto& child : group.GetChildren()) {
        if (child->IsGroup()) {
            auto newPrefix = prefix.empty() ? child->GetName()
                                            : prefix + separator + child->GetName();
            CollectPresetPaths(static_cast<const EffectPresetGroup&>(*child),
                               newPrefix, separator, result);
        } else {
            auto path = prefix.empty() ? child->GetName()
                                       : prefix + separator + child->GetName();
            result.push_back(path);
        }
    }
}

EffectPresetItem* EffectPresetManager::FindItemByPath(const std::string& path,
                                                       char separator) const
{
    std::vector<std::string> parts = SplitString(path, separator);
    if (parts.empty())
        return nullptr;

    const EffectPresetGroup* current = &_root;
    for (size_t i = 0; i < parts.size(); ++i) {
        const std::string& partName = parts[i];
        EffectPresetItem* found = current->FindChildByName(partName);
        if (found == nullptr)
            return nullptr;
        if (i == parts.size() - 1)
            return found;
        if (!found->IsGroup())
            return nullptr;
        current = static_cast<const EffectPresetGroup*>(found);
    }
    return nullptr;
}

EffectPreset* EffectPresetManager::FindPresetByPath(const std::string& path,
                                                     char separator) const
{
    EffectPresetItem* item = FindItemByPath(path, separator);
    if (item != nullptr && !item->IsGroup())
        return static_cast<EffectPreset*>(item);
    return nullptr;
}

// ---------------------------------------------------------------------------
// Mutations
// ---------------------------------------------------------------------------

EffectPreset* EffectPresetManager::AddPreset(EffectPresetGroup* parent,
                                              const std::string& name,
                                              const std::string& settings,
                                              const std::string& version,
                                              const std::string& xLightsVersion)
{
    if (parent == nullptr)
        parent = &_root;
    auto preset = std::make_unique<EffectPreset>(name, settings, version, xLightsVersion, parent);
    return static_cast<EffectPreset*>(parent->AddChild(std::move(preset)));
}

EffectPresetGroup* EffectPresetManager::AddGroup(EffectPresetGroup* parent,
                                                  const std::string& name)
{
    if (parent == nullptr)
        parent = &_root;
    auto group = std::make_unique<EffectPresetGroup>(name, parent);
    return static_cast<EffectPresetGroup*>(parent->AddChild(std::move(group)));
}

void EffectPresetManager::Remove(EffectPresetItem* item)
{
    if (item == nullptr || item->GetParent() == nullptr)
        return;
    item->GetParent()->RemoveChild(item);
    // unique_ptr destruction handles cleanup
}

void EffectPresetManager::MoveItem(EffectPresetItem* item, EffectPresetGroup* newParent,
                                    EffectPresetItem* insertAfter)
{
    if (item == nullptr || newParent == nullptr)
        return;
    EffectPresetGroup* oldParent = item->GetParent();
    if (oldParent == nullptr)
        return;

    auto owned = oldParent->RemoveChild(item);
    if (owned) {
        if (insertAfter != nullptr) {
            newParent->InsertChildAfter(std::move(owned), insertAfter);
        } else {
            newParent->AddChild(std::move(owned));
        }
    }
}

void EffectPresetManager::RenameItem(EffectPresetItem* item, const std::string& newName)
{
    if (item != nullptr)
        item->SetName(newName);
}

void EffectPresetManager::UpdatePresetSettings(EffectPreset* preset,
                                                const std::string& settings,
                                                const std::string& xLightsVersion)
{
    if (preset == nullptr)
        return;
    preset->SetSettings(settings);
    preset->SetXLightsVersion(xLightsVersion);
}

// ---------------------------------------------------------------------------
// Import
// ---------------------------------------------------------------------------

void EffectPresetManager::ImportFromXml(pugi::xml_node node, EffectPresetGroup* parent)
{
    if (!node)
        return;
    if (parent == nullptr)
        parent = &_root;

    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        std::string_view childName = child.name();
        if (childName == "effectGroup") {
            parent->AddChild(std::make_unique<EffectPresetGroup>(child, parent));
        } else if (childName == "effect") {
            parent->AddChild(std::make_unique<EffectPreset>(child, parent));
        }
    }
}

// ---------------------------------------------------------------------------
// FixRgbEffects — migrated from EffectTreeDialog
// ---------------------------------------------------------------------------

bool EffectPresetManager::FixRgbEffects()
{
    return FixGroupNames(_root);
}

bool EffectPresetManager::FixGroupNames(EffectPresetGroup& group)
{
    bool anyFixed = false;
    std::list<std::pair<std::string, int>> children;

    for (const auto& child : group.GetChildren()) {
        std::string name = child->GetName();

        // Fix illegal characters in name
        if (FixPresetName(name)) {
            child->SetName(name);
            anyFixed = true;
        }

        // Fix duplicate names at this level
        auto existingItem = std::find_if(children.begin(), children.end(),
            [&name](const std::pair<std::string, int>& b) {
                return b.first == name;
            });

        if (existingItem == children.end()) {
            children.push_back(std::make_pair(name, 0));
        } else {
            int childUniqueIdx = existingItem->second + 1;
            std::string newName = fmt::format("{} {}", name, childUniqueIdx);
            child->SetName(newName);
            existingItem->second = childUniqueIdx;
            anyFixed = true;
        }

        // Recurse into subgroups
        if (child->IsGroup()) {
            if (FixGroupNames(static_cast<EffectPresetGroup&>(*child))) {
                anyFixed = true;
            }
        }
    }

    return anyFixed;
}

EffectPreset* EffectPresetManager::GenerateAutomatedPreset(EffectPresetGroup* parent,
                                                           const std::string& name,
                                                           const std::vector<AutomatedPresetLayerSpec>& layerSpecs)
{
    if (parent == nullptr) parent = &_root;

    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("effectDB");
    root.append_attribute("version").set_value("1");
    pugi::xml_node presetNode = root.append_child("effect");
    presetNode.append_attribute("name").set_value(name.c_str());

    for (const auto& spec : layerSpecs) {
        pugi::xml_node effNode = presetNode.append_child("effect");
        effNode.append_attribute("name").set_value(spec.effectName.c_str());
        effNode.append_attribute("layer").set_value(spec.layerIndex);
        effNode.append_attribute("start").set_value(spec.startTimeMS);
        effNode.append_attribute("end").set_value(spec.endTimeMS);
        effNode.append_attribute("blend").set_value(spec.blendMode.c_str());

        for (const auto& kv : spec.parameters) {
            effNode.append_attribute(kv.first.c_str()).set_value(kv.second.c_str());
        }
    }

    std::ostringstream ss;
    doc.save(ss, "  ");
    std::string settingsXml = ss.str();

    return AddPreset(parent, name, settingsXml, "1", "2026.08");
}

EffectPreset* EffectPresetManager::SynthesizePresetFromPrompt(EffectPresetGroup* parent,
                                                               const std::string& name,
                                                               const std::string& userPrompt,
                                                               const std::string& metadataDir)
{
    std::string lowerPrompt = userPrompt;
    std::transform(lowerPrompt.begin(), lowerPrompt.end(), lowerPrompt.begin(), ::tolower);

    std::vector<AutomatedPresetLayerSpec> specs;

    // Base layer (Layer 0)
    AutomatedPresetLayerSpec baseLayer;
    baseLayer.layerIndex = 0;
    baseLayer.startTimeMS = 0;
    baseLayer.endTimeMS = 5000;
    baseLayer.blendMode = "Normal";

    if (lowerPrompt.find("fire") != std::string::npos || lowerPrompt.find("flame") != std::string::npos) {
        baseLayer.effectName = "Fire";
        baseLayer.parameters["E_CHOICE_Fire_Location"] = "Bottom";
        baseLayer.parameters["E_SLIDER_Fire_Height"] = "60";
        baseLayer.parameters["E_SLIDER_Fire_HueShift"] = "10";
    } else if (lowerPrompt.find("fireworks") != std::string::npos || lowerPrompt.find("burst") != std::string::npos) {
        baseLayer.effectName = "Fireworks";
        baseLayer.parameters["E_SLIDER_Fireworks_Count"] = "12";
        baseLayer.parameters["E_SLIDER_Fireworks_Velocity"] = "40";
    } else if (lowerPrompt.find("plasma") != std::string::npos || lowerPrompt.find("nebula") != std::string::npos) {
        baseLayer.effectName = "Plasma";
        baseLayer.parameters["E_SLIDER_Plasma_Style"] = "1";
        baseLayer.parameters["E_SLIDER_Plasma_Speed"] = "30";
    } else if (lowerPrompt.find("wave") != std::string::npos || lowerPrompt.find("ocean") != std::string::npos) {
        baseLayer.effectName = "Wave";
        baseLayer.parameters["E_CHOICE_Wave_Direction"] = "Left to Right";
    } else if (lowerPrompt.find("bars") != std::string::npos || lowerPrompt.find("stripe") != std::string::npos) {
        baseLayer.effectName = "Bars";
        baseLayer.parameters["E_CHOICE_Bars_Direction"] = "Up";
    } else {
        baseLayer.effectName = "ColorWash";
        baseLayer.parameters["E_SLIDER_ColorWash_Speed"] = "20";
    }
    specs.push_back(baseLayer);

    // Overlay layer 1 (Layer 1)
    if (lowerPrompt.find("storm") != std::string::npos || lowerPrompt.find("meteor") != std::string::npos || lowerPrompt.find("rain") != std::string::npos) {
        AutomatedPresetLayerSpec topLayer;
        topLayer.layerIndex = 1;
        topLayer.startTimeMS = 0;
        topLayer.endTimeMS = 5000;
        topLayer.blendMode = "Additive";
        topLayer.effectName = "Meteors";
        topLayer.parameters["E_SLIDER_Meteors_Count"] = "25";
        topLayer.parameters["E_SLIDER_Meteors_Length"] = "15";
        specs.push_back(topLayer);
    } else if (lowerPrompt.find("shimmer") != std::string::npos || lowerPrompt.find("sparkle") != std::string::npos || lowerPrompt.find("twinkle") != std::string::npos) {
        AutomatedPresetLayerSpec topLayer;
        topLayer.layerIndex = 1;
        topLayer.startTimeMS = 0;
        topLayer.endTimeMS = 5000;
        topLayer.blendMode = "Layered";
        topLayer.effectName = "Twinkle";
        topLayer.parameters["E_SLIDER_Twinkle_Count"] = "35";
        topLayer.parameters["E_SLIDER_Twinkle_Steps"] = "10";
        specs.push_back(topLayer);
    }

    // Top Accent layer (Layer 2)
    if (lowerPrompt.find("strobe") != std::string::npos || lowerPrompt.find("flash") != std::string::npos) {
        AutomatedPresetLayerSpec accentLayer;
        accentLayer.layerIndex = 2;
        accentLayer.startTimeMS = 0;
        accentLayer.endTimeMS = 5000;
        accentLayer.blendMode = "Additive";
        accentLayer.effectName = "Strobe";
        accentLayer.parameters["E_SLIDER_Strobe_Frequency"] = "15";
        specs.push_back(accentLayer);
    }

    return GenerateAutomatedPreset(parent, name, specs);
}
