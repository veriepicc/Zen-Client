module;
#include <iostream>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

export module MouseHook;

import HookManager;
import SigManager;
import imgui_impl_bigrat;

#define LOG_MOUSE_EVENTS 1

#if LOG_MOUSE_EVENTS
import Utils;
#endif

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
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent((float)mouseX, (float)mouseY);
        io.MousePos = ImVec2((float)mouseX, (float)mouseY);

        bool blockInput = false;

        if (action == 0 || action == 1)
        {
            int imguiButton = -1;
            if (button == 1) imguiButton = 0;
            else if (button == 2) imguiButton = 1;
            else if (button == 3) imguiButton = 2;
            if (imguiButton >= 0)
            {
                // Force ImGui to process current input state
                if (ImGui::GetCurrentContext() && ImGui::GetCurrentContext()->WithinFrameScope)
                {
                    ImGui::EndFrame();
                    ImGui::NewFrame();
                }

                bool wasOverWindow = io.WantCaptureMouse;

                io.AddMouseButtonEvent(imguiButton, action == 1);

                if (wasOverWindow || io.WantCaptureMouse)
                    blockInput = true;
            }

#if LOG_MOUSE_EVENTS
            FILE* logFile = nullptr;
            std::string logPath = Utils::GetRoamingPath() + "\\zen_input.txt";
            fopen_s(&logFile, logPath.c_str(), "a");
            if (logFile) {
                const char* buttonName = (button == 1) ? "Left" : (button == 2) ? "Right" : (button == 3) ? "Middle" : "Unknown";
                const char* actionName = (action == 1) ? "Down" : "Up";
                ImGuiIO& io = ImGui::GetIO();
                fprintf(logFile, "[Mouse] %s %s at (%d, %d) | ImGui pos: (%.1f, %.1f) | WantCapture: %s\n",
                    buttonName, actionName, mouseX, mouseY, io.MousePos.x, io.MousePos.y, io.WantCaptureMouse ? "YES" : "NO");
                fclose(logFile);
            }
#endif
        }
        
        if (!blockInput && State::original)
            State::original(mouseDevice, button, action, mouseX, mouseY, movementX, movementY, a8);
    }
}

export namespace Hooks::Input::Mouse
{
    inline bool Install()
    {
        void* target = SigManager::MouseDevice_feed;
        if (!target) return false;
        
        auto& hm = GetHookManager();
        return hm.hook<MouseFeedFn>(target, Detour, &State::original);
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Input::Mouse::Install };


