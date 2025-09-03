module;
#include <cstdint>
#include <iostream>

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
import Math;
import TexturePtr;
import Color;
import BedrockTextureData;
import RectangleArea;
// ImGui bridge
import imgui_impl_bigrat;

namespace Hooks::Render::SetupAndRender
{
    using SetupAndRenderFunction = void(*)(void*, MinecraftUIRenderContext*);
    using DrawImageFunction = void(*)(MinecraftUIRenderContext*, BedrockTextureData*, Math::Vec2<float>*, Math::Vec2<float>*, Math::Vec2<float>*, Math::Vec2<float>*, bool);

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
                                Math::Vec2<float>* pos,
                                Math::Vec2<float>* size,
                                Math::Vec2<float>* uvPos,
                                Math::Vec2<float>* uvSize,
                                bool flag)
    {
        if (!State::drewOnceThisFrame && State::imageInitialized && State::imageTexture.clientTexture)
        {
            auto p = Math::Vec2<float>(50.f, 50.f);
            auto s = Math::Vec2<float>(256.f, 256.f);
            auto up = Math::Vec2<float>(0.f, 0.f);
            auto us = Math::Vec2<float>(1.f, 1.f);
            // Clip to a smaller rect to test scissor; save/restore around our draw only
            ctx->saveCurrentClippingRectangle();
            // Engine expects x,y,width,height. Set then enable.
            auto clip = SDK::RectangleArea::FromLTRB(80.f, 80.f, 220.f, 220.f);
            ctx->setClippingRectangle(clip);
            ctx->enableScissorTest(clip);
            State::originalDrawImage(ctx, State::imageTexture.clientTexture.get(), &p, &s, &up, &us, false);
            mce::Color color(1.f, 1.f, 1.f, 1.f);
            HashedString pass("ui_textured");
            ctx->flushImages(color, 1.f, pass);
            ctx->restoreSavedClippingRectangle();
            ctx->disableScissorTest();
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

        // Per-frame begin: reset once-per-frame guard
        State::drewOnceThisFrame = false;

        //// Initialize external PNG texture once
        //if (!State::imageInitialized)
        //{
        //    const std::string path = "C:/Users/veriepic/AppData/Local/Packages/Microsoft.MinecraftUWP_8wekyb3d8bbwe/RoamingState/image.png";
        //    // Strict PNG: simple extension check
        //    if (path.size() >= 4 && (path.ends_with(".png") || path.ends_with(".PNG"))) {
        //        auto tmp = renderContext->createTexture(path, /*external*/true, /*forceReload*/true);
        //        State::imageTexture = std::move(tmp);
        //        State::imageTexture.resourceLocation = std::make_shared<ResourceLocation>(ResourceLocation(path, true));
        //        if (!State::imageTexture.clientTexture) {
        //            // Hint the engine to pull it in
        //            ResourceLocation rl(path, true);
        //            renderContext->touchTexture(rl);
        //        }
        //        State::imageInitialized = State::imageTexture.clientTexture != nullptr;
        //    }
        //}
        //
        //// Install drawImage vfunc hook once so we can inject before other UI draws
        //if (!State::originalDrawImage)
        //{
        //    void** vtable = *reinterpret_cast<void***>(renderContext);
        //    void* target = vtable[7];
        //    auto& hookManager = GetHookManager();
        //    hookManager.hook<DrawImageFunction>(target, DrawImageDetour, &State::originalDrawImage);
        //}

        // Ensure we let the game render first so our ImGui overlay draws on top
        if (State::originalFunction)
            State::originalFunction(screenView, renderContext);

        // ImGui demo: init once, then render after original UI to ensure top-most
        static bool imguiInit = false;
        if (!imguiInit)
        {
            ImGui_ImplBigRat::Init(renderContext);
            imguiInit = true;
        }

        // Demo frame with approximate sizes; these can be sourced from GuiData later
        ImGui_ImplBigRat::Demo(renderContext, 1.0f / 60.0f, 1280.0f, 720.0f, 1.0f, 1.0f);

        // Quick validation draw using tessellator + our dog texture via MeshHelpers
        // If this textured quad appears, the material/texture path is good.
        //if (State::imageTexture.clientTexture)
        //{
        //    ScreenContext* sc = renderContext->getScreenContext();
        //    Tessellator* tess = sc->getTessellator();
        //    HashedString pass("ui_textured");
        //    MaterialPtr* mat = MaterialPtr::createMaterial(pass);
        //    tess->resetTransform(false);
        //    float x = 420.f, y = 120.f, w = 128.f, h = 128.f;
        //    tess->begin(mce::PrimitiveMode::TriangleList, 6, false);
        //    mce::Color c(1.f, 1.f, 1.f, 1.f);
        //    // tri 1
        //    tess->color(c.r, c.g, c.b, c.a);
        //    tess->vertexUV(x, y, 0.f, 0.f, 0.f);
        //    tess->color(c.r, c.g, c.b, c.a);
        //    tess->vertexUV(x + w, y, 0.f, 1.f, 0.f);
        //    tess->color(c.r, c.g, c.b, c.a);
        //    tess->vertexUV(x + w, y + h, 0.f, 1.f, 1.f);
        //    // tri 2
        //    tess->color(c.r, c.g, c.b, c.a);
        //    tess->vertexUV(x, y, 0.f, 0.f, 0.f);
        //    tess->color(c.r, c.g, c.b, c.a);
        //    tess->vertexUV(x + w, y + h, 0.f, 1.f, 1.f);
        //    tess->color(c.r, c.g, c.b, c.a);
        //    tess->vertexUV(x, y + h, 0.f, 0.f, 1.f);
        //    MeshHelpers::renderMeshImmediately2(sc, tess, mat, *State::imageTexture.clientTexture);
        //}
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


