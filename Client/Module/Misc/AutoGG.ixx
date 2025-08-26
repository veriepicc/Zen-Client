module;
#include <string>

export module AutoGG;

import Module;

export class AutoGG : public Module
{
public:
    AutoGG()
        : Module("AutoGG", "Sends a GG message after a match.", Category::Misc)
    {
        addSetting(Settings::text("Message", "gg"));
    }

    void onEnable() override {}
    void onDisable() override {}
    void onUpdate() override {}
    void onRender() override {}
};


