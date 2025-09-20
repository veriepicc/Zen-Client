module;
#include <string>

export module Uninject;

import Module;
import Keys;
import Zen;

export class Uninject : public Module
{
public:
    Uninject()
        : Module("Uninject", "Uninjects the client.", Category::Misc, Keys::End)
    {
        Register();
    }

    void onEnable() override 
    {
        Zen::Eject();
    }
    void onDisable() override {}
    void onUpdate() override {}
    void onRender(MinecraftUIRenderContext* /*rc*/) override {}
};

export Uninject UninjectInstance{};
