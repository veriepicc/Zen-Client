module;
#include <iostream>

export module KeyboardHook;

import HookManager;
import SigManager;
import imgui_impl_bigrat;
import Module;

#define LOG_KEYBOARD_EVENTS 1

#if LOG_KEYBOARD_EVENTS
import Utils;
#endif

namespace Hooks::Input::Keyboard
{
    using KeyboardFeedFn = void(*)(int, bool);

    struct State
    {
        static inline KeyboardFeedFn original = nullptr;
    };

    static void __stdcall Detour(int key, bool state)
    {
        if (key >= 0 && key <= 512)
            ImGui_ImplBigRat::AddKeyEvent(key, state);

#if LOG_KEYBOARD_EVENTS
        FILE* logFile = nullptr;
        std::string logPath = Utils::GetRoamingPath() + "\\zen_input.txt";
        fopen_s(&logFile, logPath.c_str(), "a");
        if (logFile) {
            fprintf(logFile, "[Keyboard] Key %d %s\n", key, state ? "Down" : "Up");
            fclose(logFile);
        }
#endif

        Modules::HandleKeyEvent(key, state);
        if (State::original) State::original(key, state);
    }
}

export namespace Hooks::Input::Keyboard
{
    inline bool Install()
    {
        void* target = SigManager::Keyboard_feed;
        if (!target) return false;
        
        auto& hm = GetHookManager();
        return hm.hook<KeyboardFeedFn>(target, Detour, &State::original);
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Input::Keyboard::Install };


