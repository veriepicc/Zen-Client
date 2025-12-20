module;
#include <iostream>
#include <string>
#include <memory>
#include "imgui/imgui.h"

export module SetupAndRender;

import SigManager;
import MinecraftUIRenderContext;
import HookManager;
import MeshHelpers;
import MaterialPtr;
import ScreenContext;
import HashedString;
import Tesselator;
import Paul;
import TexturePtr;
import Color;
import BedrockTextureData;
import RectangleArea;
import imgui_impl_bigrat;
import Module;
import ResourceLocation;
import Utils;
import Global;

namespace Hooks::Render::SetupAndRender
{
    using SetupAndRenderFn = void(*)(void*, MinecraftUIRenderContext*);
    using DrawImageFn = void(*)(MinecraftUIRenderContext*, BedrockTextureData*, Paul::Vec2f*, Paul::Vec2f*, Paul::Vec2f*, Paul::Vec2f*, bool);

    struct State
    {
        static inline SetupAndRenderFn originalFunction = nullptr;
        static inline DrawImageFn originalDrawImage = nullptr;
        static inline TexturePtr martTexture{};
        static inline bool initialized = false;
        static inline bool drewThisFrame = false;
    };

    /**
     * detoured drawImage to inject imgui before anything else renders
     */
    inline void DrawImageDetour(MinecraftUIRenderContext* ctx, BedrockTextureData* tex, Paul::Vec2f* p, Paul::Vec2f* s, Paul::Vec2f* u1, Paul::Vec2f* u2, bool f)
    {
        if (!State::drewThisFrame)
        {
            // Update texture handle if resource location is valid
            if (!State::martTexture.clientTexture && State::martTexture.resourceLocation)
            {
                TexturePtr fetched;
                ctx->getTexture(fetched, *State::martTexture.resourceLocation, false);
                if (fetched.clientTexture) State::martTexture = fetched;
            }

            ImGui_ImplBigRat::NewFrame(1.0f / 60.0f, 0.0f, 0.0f, 1.0f, 1.0f);
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
            ImGui::Begin("Mart", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            if (State::martTexture.clientTexture)
                ImGui::Image((ImTextureID)&State::martTexture, ImVec2(128, 128));
            else
                ImGui::TextUnformatted("Summoning Mart...");
            ImGui::End();

            Modules::RenderTick(ctx);

            ImGui::Render();
            ImGui_ImplBigRat::RenderDrawData(ImGui::GetDrawData(), ctx);
            State::drewThisFrame = true;
        }
        State::originalDrawImage(ctx, tex, p, s, u1, u2, f);
    }

    /**
     * main rendering detour
     */
    inline void Detour(void* screenView, MinecraftUIRenderContext* rc)
    {
        if (!rc) {
            if (State::originalFunction) State::originalFunction(screenView, rc);
            return;
        }

        Global::setClientInstance(rc->getClientInstance());
        
        // freshen global matrices/camera
        if (auto* ci = Global::getClientInstance())
        {
            Global::viewMatrix = ci->getViewMatrix();
            Global::fov = ci->getFov();
            if (auto* lr = ci->getLevelRenderer())
            {
                if (auto* player = lr->getLevelRendererPlayer())
                {
                    Global::cameraPos = player->getCameraPos();
                }
            }
        }

        State::drewThisFrame = false;

        // one-time init
        if (!State::initialized)
        {
            std::string path = Utils::GetRoamingPath();
            if (!path.empty())
            {
                for (char& c : path) if (c == '\\') c = '/';
                path += "/image.png";

                auto rl = std::make_shared<ResourceLocation>(path, true);
                State::martTexture.resourceLocation = rl;
                rc->createTexture(path, true, true);
                rc->touchTexture(*rl);
            }

            // Hook drawImage (index 7) for injection
            void** vtable = *reinterpret_cast<void***>(rc);
            GetHookManager().hook<DrawImageFn>(vtable[7], DrawImageDetour, &State::originalDrawImage);

            ImGui_ImplBigRat::Init(rc);
            State::initialized = true;
        }

        if (State::originalFunction)
            State::originalFunction(screenView, rc);
    }
}

export namespace Hooks::Render::SetupAndRender
{
    inline TexturePtr* GetMartTexture() { return &State::martTexture; }

    inline bool Install()
    {
        void* target = SigManager::Setupandrender;
        if (!target) return false;
        return GetHookManager().hook<SetupAndRenderFn>(target, Detour, &State::originalFunction);
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Render::SetupAndRender::Install };
