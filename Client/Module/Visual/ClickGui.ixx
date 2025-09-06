module;
#include <string>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <variant>
#include "imgui/imgui.h"

export module ClickGui;

import Module;
import Keys;
import Utils;
import SDK;
import MinecraftUIRenderContext;
import OffsetManager;

export class ClickGui : public Module
{
public:
    ClickGui()
        : Module("ClickGui", "Displays the clickable GUI for modules.", Category::Visual)
    {
        addSetting(Settings::color("ThemeColor", Color(146, 102, 204)));
        addSetting(Settings::decimal("Scale", 1.0f, 0.5f, 2.0f));
        addSetting(Settings::boolean("Rainbow", false));
        setKeybind(Keys::Insert);
        Register();
    }

    void onEnable() override
    {
        std::cout << "[ClickGui] Enabled" << std::endl;
        const auto& core = SDK::AllCore();
        std::cout << "[ClickGui] SDK core count=" << core.size() << std::endl;
        for (const auto& name : core) std::cout << "  - " << name << std::endl;
        std::cout << "[ClickGui] MUIRC offsets: clientInstance=0x" << std::hex
                  << Offsets::MinecraftUIRenderContext_clientInstance
                  << " screenContext=0x" << Offsets::MinecraftUIRenderContext_screenContext
                  << std::dec << std::endl;
    }
    void onDisable() override {}
    void onUpdate() override {}
    void onRender(MinecraftUIRenderContext* /*rc*/) override
    {
        if (!enabledRef()) return;

        static int selectedCategory = static_cast<int>(Category::Combat);
        static Module* selectedModule = nullptr;

        ImGui::SetNextWindowSize(ImVec2(720.0f, 460.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(120.0f, 90.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Zen ClickGui", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (ImGui::BeginTabBar("Categories"))
            {
                for (int i = 0; i < 6; ++i)
                {
                    const Category cat = static_cast<Category>(i);
                    std::string catName(std::string(categoryName(cat)));
                    if (ImGui::BeginTabItem(catName.c_str()))
                    {
                        selectedCategory = i;

                        auto view = Modules::ByCategory(cat);
                        ImGui::Columns(2, nullptr, false);

                        // Left: modules list
                        for (Module* m : view)
                        {
                            if (!m) continue;
                            bool isEnabled = m->enabledRef();
                            std::string chkId = std::string("##toggle_") + m->nameRef();
                            if (ImGui::Checkbox(chkId.c_str(), &isEnabled))
                            {
                                m->setEnabled(isEnabled);
                            }
                            ImGui::SameLine();
                            if (ImGui::Selectable(m->nameRef().c_str(), selectedModule == m))
                            {
                                selectedModule = m;
                            }
                        }

                        ImGui::NextColumn();

                        // Right: selected module settings
                        if (selectedModule)
                        {
                            ImGui::TextUnformatted(selectedModule->nameRef().c_str());
                            ImGui::Separator();

                            auto& settings = selectedModule->settingsRef();
                            for (auto& s : settings)
                            {
                                if (auto pb = std::get_if<bool>(&s.value))
                                {
                                    bool v = *pb;
                                    if (ImGui::Checkbox(s.name.c_str(), &v))
                                        *pb = v;
                                }
                                else if (auto pi = std::get_if<int>(&s.value))
                                {
                                    int v = *pi;
                                    int minV = s.intBounds ? s.intBounds->first : 0;
                                    int maxV = s.intBounds ? s.intBounds->second : 100;
                                    if (ImGui::SliderInt(s.name.c_str(), &v, minV, maxV))
                                        *pi = v;
                                }
                                else if (auto pf = std::get_if<float>(&s.value))
                                {
                                    float v = *pf;
                                    float minV = s.floatBounds ? s.floatBounds->first : 0.0f;
                                    float maxV = s.floatBounds ? s.floatBounds->second : 1.0f;
                                    if (ImGui::SliderFloat(s.name.c_str(), &v, minV, maxV))
                                        *pf = v;
                                }
                                else if (auto pc = std::get_if<Color>(&s.value))
                                {
                                    float col[4] = { pc->r / 255.0f, pc->g / 255.0f, pc->b / 255.0f, pc->a / 255.0f };
                                    if (ImGui::ColorEdit4(s.name.c_str(), col))
                                    {
                                        pc->r = static_cast<std::uint8_t>(col[0] * 255.0f);
                                        pc->g = static_cast<std::uint8_t>(col[1] * 255.0f);
                                        pc->b = static_cast<std::uint8_t>(col[2] * 255.0f);
                                        pc->a = static_cast<std::uint8_t>(col[3] * 255.0f);
                                    }
                                }
                                else if (auto ps = std::get_if<std::string>(&s.value))
                                {
                                    char buf[256] = {};
                                    const size_t copyLen = (std::min)(ps->size(), sizeof(buf) - 1);
                                    std::memcpy(buf, ps->c_str(), copyLen);
                                    if (ImGui::InputText(s.name.c_str(), buf, sizeof(buf)))
                                        *ps = std::string(buf);
                                }
                            }
                        }

                        ImGui::Columns(1);
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
};

export ClickGui ClickGuiInstance{};
