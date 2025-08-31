#pragma once
#include <string>
#include "../Module.hpp"

class AutoSprint : public Module
{
public:
    AutoSprint()
        : Module("AutoSprint", "Automatically sprints when moving.", Category::Player)
    {
        addSetting(Settings::boolean("OnlyForward", true));
        Register();
    }

    void onEnable() override {}
    void onDisable() override {}
    void onUpdate() override {}
    //void onRender() override {}
};


