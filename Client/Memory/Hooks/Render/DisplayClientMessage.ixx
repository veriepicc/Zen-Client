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
    if (originalFunction)
        originalFunction(guidata, message, a, b);
}

export namespace Hooks::Render::DisplayClientMessage {
    inline bool Install()
    {
        void* target = SigManager::GuiData_DisplayClientMessage;
        if (!target)
        {
            return false;
        }

        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<DisplayClientMessageFunction>(
            target,
            GuiData_DisplayClientMessage,
            &originalFunction
        );
        return ok;
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Render::DisplayClientMessage::Install };