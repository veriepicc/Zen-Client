module;
#include <string>
#include <iostream>

export module ClickGui;

import Module;
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
    void onRender() override
    {
        static bool loggedOnce = false;
        if (!loggedOnce)
        {
            std::cout << "[ClickGui] onRender reached" << std::endl;
            loggedOnce = true;
        }
    }
};

export ClickGui ClickGuiInstance{};
