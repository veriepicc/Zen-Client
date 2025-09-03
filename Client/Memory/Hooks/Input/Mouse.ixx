module;

export module MouseHook;

import HookManager;
import SigManager;
import imgui_impl_bigrat;

namespace Hooks::Input::Mouse
{
    using MouseFeedFn = void(*)(void*, char, char, short, short, short, short, char);

    struct State
    {
        static inline MouseFeedFn original = nullptr;
    };

    static void __stdcall Detour(void* mouseDevice, char button, char action, short mouseX, short mouseY,
                                 short movementX, short movementY, char a8)
    {
        // Feed ImGui first; buttons: left=1,right=2,middle=3 in engine → ImGui 0,1,2
        if (action == 0 || action == 1)
        {
            int imguiButton = -1;
            if (button == 1) imguiButton = 0; // left
            else if (button == 2) imguiButton = 1; // right
            else if (button == 3) imguiButton = 2; // middle
            if (imguiButton >= 0)
                ImGui_ImplBigRat::AddMouseButtonEvent(imguiButton, action == 1);
        }
        ImGui_ImplBigRat::AddMousePosEvent((float)mouseX, (float)mouseY);
        if (State::original) State::original(mouseDevice, button, action, mouseX, mouseY, movementX, movementY, a8);
    }
}

export namespace Hooks::Input::Mouse
{
    inline bool Install()
    {
        void* target = SigManager::MouseDevice_feed;
        if (!target)
        {
            return false;
        }
        auto& hm = GetHookManager();
        return hm.hook<MouseFeedFn>(target, Detour, &State::original);
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Input::Mouse::Install };


