module;
#include <iostream>
#include <optional>

export module DisplayClientMessage;

import HookManager;
import GuiData;
import SigManager;

using DisplayClientMessageFunction = void(*)(GuiData* guidata, std::optional<std::string>, bool);
DisplayClientMessageFunction originalFunction = nullptr;

void LevelRenderer_DisplayClientMessage(GuiData* guidata, std::optional<std::string> message, bool c) {
    std::cout << message.value();

    static bool loggedOnce = false;
    static int frameCounter = 0;
    if (originalFunction)
        originalFunction(guidata, message, c);

    if (guidata)
    {
        if (!loggedOnce)
        {
            std::cout << "[DisplayClientMessage] first call ok gd=" << guidata << std::endl;
            loggedOnce = true;
        }
    }
}

export namespace Hooks::Render::DisplayClientMessage {
    inline bool Install()
    {
        void* target = SigManager::GuiData_DisplayClientMessage;
        if (!target)
        {
            std::cout << "[DisplayClientMessage] signature not resolved" << std::endl;
            return false;
        }

        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<DisplayClientMessageFunction>(
            target,
            LevelRenderer_DisplayClientMessage,
            &originalFunction
        );
        if (ok) std::cout << "[DisplayClientMessage] hook installed" << std::endl;
        return ok;
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Render::DisplayClientMessage::Install };