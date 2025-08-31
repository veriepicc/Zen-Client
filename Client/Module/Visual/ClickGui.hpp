#pragma once
#include <string>
#include <iostream>
#include "../Module.hpp"

#include <SDK/SDK.hpp>
#include <Memory/OffsetManager.hpp>
#include <SDK/Render/HashedString.hpp>
#include <SDK/Render/MinecraftUIRenderContext.hpp>
#include <SDK/Render/MaterialPtr.hpp>
#include <SDK/Render/MeshHelper.hpp>

class ClickGui : public Module
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
    void onRender(MinecraftUIRenderContext* mcuirc, ScreenContext* sc) override
    {
        static MaterialPtr* fillColor = nullptr;

        auto tess = sc->getTessellator();
        
        tess->begin(mce::PrimitiveMode::TriangleStrip, 3);

        tess->vertex(0.f, 0.f, 0.f);
        tess->vertex(100.f, 0.f, 0.f);
        tess->vertex(100.f, 100.f, 0.f);
        tess->vertex(0.f, 100.f, 0.f);

        HashedString fillColorStr("ui_fill_color");
        if (!fillColor) fillColor = MaterialPtr::createMaterial(fillColorStr);

        if (fillColor && tess && sc) {
            MeshHelper::renderMeshImmediately(sc, tess, fillColor);
        }

    }
};

ClickGui ClickGuiInstance{};
