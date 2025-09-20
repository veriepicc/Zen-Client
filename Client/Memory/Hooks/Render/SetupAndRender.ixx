module;
#include <cstdint>
#include <iostream>
#include <string>
#include <memory>
#include "imgui/imgui.h"

export module SetupAndRender;

import HookManager;
import SigManager;
import Memory;
import MinecraftUIRenderContext;
import PrimitiveMode;
import OffsetManager;
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
// ImGui bridge
import imgui_impl_bigrat;
import Module;
import ResourceLocation;
import Utils;
import Global;


namespace Hooks::Render::SetupAndRender
{
    using SetupAndRenderFunction = void(*)(void*, MinecraftUIRenderContext*);
    using DrawImageFunction = void(*)(MinecraftUIRenderContext*, BedrockTextureData*, Paul::Vec2<float>*, Paul::Vec2<float>*, Paul::Vec2<float>*, Paul::Vec2<float>*, bool);

    struct State
    {
        static inline SetupAndRenderFunction originalFunction = nullptr;
        static inline bool imageInitialized = false;
        static inline TexturePtr imageTexture{};
        static inline DrawImageFunction originalDrawImage = nullptr;
        static inline bool drewOnceThisFrame = false;
    };

    inline void DrawImageDetour(MinecraftUIRenderContext* ctx,
                                BedrockTextureData* tex,
                                Paul::Vec2<float>* pos,
                                Paul::Vec2<float>* size,
                                Paul::Vec2<float>* uvPos,
                                Paul::Vec2<float>* uvSize,
                                bool flag)
    {
        // First image draw of the frame: submit our ImGui pass so it appears underneath game UI
        if (!State::drewOnceThisFrame)
        {
            ImGui_ImplBigRat::NewFrame(1.0f / 60.0f, 0.0f, 0.0f, 1.0f, 1.0f);
            ImGui::NewFrame();
            // Try to fetch texture handle if we have a resource location
            if (!State::imageTexture.clientTexture && State::imageTexture.resourceLocation)
            {
                TexturePtr fetched;
                ctx->getTexture(fetched, *State::imageTexture.resourceLocation, /*forceReload*/false);
                if (fetched.clientTexture)
                {
                    State::imageTexture = fetched;
                }
            }

            // MART window
            ImGui::SetNextWindowPos(ImVec2(420, 120), ImGuiCond_FirstUseEver);
            ImGui::Begin("Mart", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            if (State::imageTexture.clientTexture)
            {
                ImGui::Image((ImTextureID)&State::imageTexture, ImVec2(128, 128));
            }
            else
            {
                ImGui::TextUnformatted("Loading dog texture...");
            }
            ImGui::End();

            // Render module UIs (e.g., ClickGui)
            Modules::RenderTick(ctx);
            ImGui::Render();
            ImGui_ImplBigRat::RenderDrawData(ImGui::GetDrawData(), ctx);

            // If texture handle not yet valid, try to fetch it again (async load)
            if (!State::imageTexture.clientTexture && State::imageTexture.resourceLocation)
            {
                TexturePtr fetched;
                ctx->getTexture(fetched, *State::imageTexture.resourceLocation, /*forceReload*/false);
                if (fetched.clientTexture)
                {
                    State::imageTexture = fetched;
                }
            }

            State::drewOnceThisFrame = true;
        }
        State::originalDrawImage(ctx, tex, pos, size, uvPos, uvSize, flag);
    }

    inline void Detour(void* screenView, MinecraftUIRenderContext* renderContext)
    {
        if (!renderContext) {
            if (State::originalFunction) State::originalFunction(screenView, renderContext);
            return;
        }
		if (!Global::getClientInstance()) Global::setClientInstance(renderContext->getClientInstance());

        State::drewOnceThisFrame = false;

        // Texture bootstrap for Mart image once
        if (!State::imageInitialized)
        {
            std::string base = Utils::GetRoamingPath();
            if (!base.empty())
            {
                for (char& ch : base) if (ch == '\\') ch = '/';
                const std::string path = base + "/image.png";
                auto rl = std::make_shared<ResourceLocation>(ResourceLocation(path, /*external*/true));
                State::imageTexture.resourceLocation = rl;
                TexturePtr tp = renderContext->createTexture(path, /*external*/true, /*forceReload*/true);
                renderContext->touchTexture(*rl);
                TexturePtr fetched;
                renderContext->getTexture(fetched, *rl, /*forceReload*/false);
                if (fetched.clientTexture)
                {
                    State::imageTexture = fetched;
                }
                State::imageInitialized = State::imageTexture.clientTexture != nullptr;
            }
        }
        //
        // Install drawImage vfunc hook once so we can inject before other UI draws
        if (!State::originalDrawImage)
        {
            void** vtable = *reinterpret_cast<void***>(renderContext);
            void* target = vtable[7];
            auto& hookManager = GetHookManager();
            hookManager.hook<DrawImageFunction>(target, DrawImageDetour, &State::originalDrawImage);
        }

        // Initialize ImGui backend once
        static bool imguiInit = false;
        if (!imguiInit)
        {
            ImGui_ImplBigRat::Init(renderContext);
            imguiInit = true;
        }

        // ImGui rendering is executed in DrawImageDetour on first image draw each frame

        if (State::originalFunction)
            State::originalFunction(screenView, renderContext);
    }
}

export namespace Hooks::Render::SetupAndRender
{
    inline bool Install()
    {
        void* target = SigManager::Setupandrender;
        if (!target)
        {
            std::cout << "[SetupAndRender] signature not resolved" << std::endl;
            return false;
        }

        auto& hookManager = GetHookManager();
        return hookManager.hook<Hooks::Render::SetupAndRender::SetupAndRenderFunction>(
            target,
            Hooks::Render::SetupAndRender::Detour,
            &Hooks::Render::SetupAndRender::State::originalFunction
        );
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Render::SetupAndRender::Install };


