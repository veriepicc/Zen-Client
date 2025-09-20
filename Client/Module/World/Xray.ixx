module;
#include <string>

export module Xray;

import Module;
import Utils;

export class Xray : public Module
{
public:
    Xray()
        : Module("Xray", "Highlights valuable blocks through walls.", Category::World)
    {
        addSetting(Settings::color("Highlight", Color(255, 215, 0)));
        addSetting(Settings::boolean("ShowOres", true));
        Register();
    }

    void onEnable() override {}
    void onDisable() override {}
    void onUpdate() override {}
    void onRender(MinecraftUIRenderContext* /*rc*/) override {}
};

export Xray XrayInstance{};


