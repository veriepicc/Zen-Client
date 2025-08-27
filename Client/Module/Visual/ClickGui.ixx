module;
#include <string>

export module ClickGui;

import Module;
import Utils;

export class ClickGui : public Module
{
public:
    ClickGui()
        : Module("ClickGui", "Displays the clickable GUI for modules.", Category::Visual)
    {
        addSetting(Settings::color("ThemeColor", Color(146, 102, 204)));
        addSetting(Settings::decimal("Scale", 1.0f, 0.5f, 2.0f));
        addSetting(Settings::boolean("Rainbow", false));
    }

    void onEnable() override {}
    void onDisable() override {}
    void onUpdate() override {}
    void onRender() override {}
};




// THIS SHIT CRASHES FOR SOME REASON?? maybe bad offset idk.
/*/
export class ClickGui : public Module
{
public:
    ClickGui()
        : Module("ClickGui", "Displays the clickable GUI for modules.", Category::Visual)
    {
        addSetting(Settings::color("ThemeColor", Color(146, 102, 204)));
        addSetting(Settings::decimal("Scale", 1.0f, 0.5f, 2.0f));
        addSetting(Settings::boolean("Rainbow", false));
        ClickGuiInternal::HookSetupAndRender();
        setEnabled(true);
    }

    void onEnable() override { std::cout << "[ClickGui] Enabled" << std::endl; }
    void onDisable() override { std::cout << "[ClickGui] Disabled" << std::endl; }
    void onUpdate() override {}
    void onRender() override
    {
        auto* ctx = ClickGuiInternal::g_lastCtx;
        if (!ctx) {
            std::cout << "[ClickGui] onRender: no render context yet" << std::endl;
            return;
        }
        if (!ctx) return;

        void* sc = ctx->screenContext;
        if (!sc) { std::cout << "[ClickGui] onRender: no screenContext" << std::endl; return; }

        void* tess = Memory::MemberAt<void*>((char*)sc, OffsetManager::ScreenContext_tessellator);
        if (!tess) { std::cout << "[ClickGui] onRender: tessellator null" << std::endl; return; }

        void* beginFn = SigManager::Tessellator_begin_b ? SigManager::Tessellator_begin_b : SigManager::Tessellator_begin_a;
        void* colorFn = SigManager::Tessellator_colorF;
        void* vertexFn = SigManager::Tessellator_vertex;

        if (!beginFn || !vertexFn || !colorFn) { std::cout << "[ClickGui] onRender: missing tess fn(s)" << std::endl; return; }

        Memory::CallFunc<void, void*, mce::PrimitiveMode, int, bool>(beginFn, tess, mce::PrimitiveMode::TriangleList, 3, false);
        Memory::CallFunc<void, void*, float, float, float, float>(colorFn, tess, 1.0f, 1.0f, 1.0f, 1.0f);
        Memory::CallFunc<void, void*, float, float, float>(vertexFn, tess, 10.0f, 10.0f, 0.0f);
        Memory::CallFunc<void, void*, float, float, float>(vertexFn, tess, 110.0f, 10.0f, 0.0f);
        Memory::CallFunc<void, void*, float, float, float>(vertexFn, tess, 60.0f, 110.0f, 0.0f);
    }
};

/*/
