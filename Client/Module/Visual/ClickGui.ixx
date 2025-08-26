module;
#include <string>

export module ClickGui;

import Module;
import Utils;

export class ClickGui : public Module
{
public:
    ClickGui()
        : Module("ClickGui", "Displays the clickable GUI for modules.", Category::Visual)
    {
        addSetting(Settings::color("ThemeColor", Color(146, 102, 204)));
        addSetting(Settings::decimal("Scale", 1.0f, 0.5f, 2.0f));
        addSetting(Settings::boolean("Rainbow", false));
    }

    void onEnable() override {}
    void onDisable() override {}
    void onUpdate() override {}
    void onRender() override {}
};


