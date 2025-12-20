module;
#include <string>

export module AutoSprint;

import Module;
import Global;
import MoveInputComponent;


export class AutoSprint : public Module
{
public:
    AutoSprint()
        : Module("AutoSprint", "Automatically sprints when moving.", Category::Player)
    {
        addSetting(Settings::boolean("Forward Only", true));
        Register();
    }

    void onUpdate() override
    {
        auto* localPlayer = Global::getLocalPlayer();
        if (!localPlayer) return;

        auto* moveInput = localPlayer->getMoveInputComponent();
        if (!moveInput) return;

        bool* forwardOnlySetting = getSetting<bool>("Forward Only");
        if (forwardOnlySetting && *forwardOnlySetting)
        {
            if (!moveInput->inputState.forward) return;
        }

        moveInput->sprinting = true;
        moveInput->inputState.sprintDown = true;
        moveInput->rawInputState.sprintDown = true;
    }
};


export AutoSprint autoSprintInstance{};
