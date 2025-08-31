#pragma once
#include <string>
#include "../Module.hpp"
 
class Flight : public Module
{
public:
    Flight()
        : Module("Flight", "Grants the ability to fly.", Category::Movement)
    {
        addSetting(Settings::decimal("Speed", 1.0f, 0.1f, 5.0f));
        addSetting(Settings::boolean("AntiKick", true));
        Register();
    }

    void onEnable() override {}
    void onDisable() override {}
    void onUpdate() override {}
    //void onRender() override {}
};


