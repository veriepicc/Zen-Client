module;
#include <string>

export module Killaura;

import Module;
import Utils;

export class Killaura : public Module
{
public:
    Killaura()
        : Module("Killaura", "Automatically attacks nearby targets.", Category::Combat)
    {
        addSetting(Settings::boolean("OnlyPlayers", true));
        addSetting(Settings::decimal("Range", 3.2f, 1.0f, 6.0f));
        addSetting(Settings::integer("APS", 12, 1, 20));
    }

    void onEnable() override {}
    void onDisable() override {}
    void onUpdate() override {}
    void onRender() override {}
};


