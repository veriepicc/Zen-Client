module;
#include <iostream>
#include <optional>

export module DisplayClientMessage;

import HookManager;
import GuiData;
import SigManager;

using DisplayClientMessageFunction = void(*)(GuiData*, const std::string&, std::optional<std::string>, bool);
DisplayClientMessageFunction originalFunction = nullptr;

void GuiData_DisplayClientMessage(GuiData* guidata, const std::string& message, std::optional<std::string> a, bool b) {
    static bool loggedOnce = false;
    if (originalFunction)
        originalFunction(guidata, message, a, b);

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
            GuiData_DisplayClientMessage,
            &originalFunction
        );
        if (ok) std::cout << "[DisplayClientMessage] hook installed" << std::endl;
        return ok;
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Render::DisplayClientMessage::Install };