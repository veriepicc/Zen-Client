module;
#include <string>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <variant>
#include "imgui/imgui.h"

export module ClickGui;

import Module;
import Keys;
import Utils;
import SDK;
import MinecraftUIRenderContext;
import OffsetManager;
import LevelRenderer;
import ScreenContext;
import Tesselator;
import MaterialPtr;
import HashedString;
import PrimitiveMode;
import Math;
import MeshHelpers;

export class ClickGui : public Module
{
public:
    ClickGui()
        : Module("ClickGui", "Displays the clickable GUI for modules.", Category::Visual)
    {
        addSetting(Settings::color("ThemeColor", Color(146, 102, 204)));
        addSetting(Settings::decimal("Scale", 1.0f, 0.5f, 2.0f));
        addSetting(Settings::boolean("Rainbow", false));
        setKeybind(Keys::Insert);
        Register();
    }

    void onEnable() override
    {
        std::cout << "[ClickGui] Enabled" << std::endl;
        const auto& core = SDK::AllCore();
        std::cout << "[ClickGui] SDK core count=" << core.size() << std::endl;
        for (const auto& name : core) std::cout << "  - " << name << std::endl;
        std::cout << "[ClickGui] MUIRC offsets: clientInstance=0x" << std::hex
                  << Offsets::MinecraftUIRenderContext_clientInstance
                  << " screenContext=0x" << Offsets::MinecraftUIRenderContext_screenContext
                  << std::dec << std::endl;
    }
    void onDisable() override {}
    void onUpdate() override {}
    void onRender(MinecraftUIRenderContext* /*rc*/) override {}

    void onWorldRender(LevelRenderer* lr, ScreenContext* sc) override
    {
        if (!enabledRef()) return;
        if (!lr || !sc) return;

        Tessellator* tess = sc->getTessellator();
        if (!tess) return;

        static MaterialPtr* mat = nullptr;
        if (!mat)
        {
            HashedString name("ui_fill_color");
            mat = MaterialPtr::createUIMaterial(name);
            if (!mat) mat = MaterialPtr::createMaterial(name);
        }
        if (!mat) return;

        const auto cam = lr->getLevelRendererPlayer()->getCameraPos();
        static int logEvery = 0;

        const float panelWidth = 100.4f;
        const float panelHeight = 100.4f;
        const float sideOffset = 10.3f;     // how far to the right of the camera
        const float forwardOffset = 1.25f; // small nudge in front so not inside camera

        const float cx = cam.x + sideOffset;
        const float cy = cam.y + 1.5f;
        const float cz = cam.z - forwardOffset; // nudge in front, anchored next to camera
        if ((++logEvery % 120) == 1)
        {
            std::cout << "[ClickGui3D] cam=(" << cam.x << ", " << cam.y << ", " << cam.z << ")";
            std::cout << " panel center=(" << cx << ", " << cy << ", " << cz << ")" << std::endl;
        }

        const float x0 = cx - panelWidth * 0.5f;
        const float y0 = cy - panelHeight * 0.5f;
        const float x1 = cx + panelWidth * 0.5f;
        const float y1 = cy + panelHeight * 0.5f;
        const float z = cz;

        // Quad 1: plane perpendicular to Z axis (vertical panel)
        tess->begin(mce::PrimitiveMode::TriangleList, 6, false);
        tess->color(0.95f, 0.10f, 0.20f, 0.85f);
        // front face
        tess->vertex(x0, y0, z);
        tess->vertex(x1, y0, z);
        tess->vertex(x1, y1, z);
        tess->vertex(x0, y0, z);
        tess->vertex(x1, y1, z);
        tess->vertex(x0, y1, z);
        // back face (double-sided)
        tess->vertex(x1, y1, z);
        tess->vertex(x1, y0, z);
        tess->vertex(x0, y0, z);
        tess->vertex(x0, y1, z);
        tess->vertex(x1, y1, z);
        tess->vertex(x0, y0, z);
        MeshHelpers::renderMeshImmediately(sc, tess, mat);

        // Quad 2: plane perpendicular to X axis (vertical panel), same center
        const float zx0 = cz - panelWidth * 0.5f;
        const float zx1 = cz + panelWidth * 0.5f;
        const float xx = cx;
        tess->begin(mce::PrimitiveMode::TriangleList, 6, false);
        tess->color(0.10f, 0.70f, 0.30f, 0.85f);
        // front face
        tess->vertex(xx, y0, zx0);
        tess->vertex(xx, y0, zx1);
        tess->vertex(xx, y1, zx1);
        tess->vertex(xx, y0, zx0);
        tess->vertex(xx, y1, zx1);
        tess->vertex(xx, y1, zx0);
        // back face (double-sided)
        tess->vertex(xx, y1, zx1);
        tess->vertex(xx, y0, zx1);
        tess->vertex(xx, y0, zx0);
        tess->vertex(xx, y1, zx0);
        tess->vertex(xx, y1, zx1);
        tess->vertex(xx, y0, zx0);
        MeshHelpers::renderMeshImmediately(sc, tess, mat);

        static int drawLog = 0;
        if ((++drawLog % 180) == 2)
        {
            std::cout << "[ClickGui3D] submitted 2 quads at center z=" << z << " and center x=" << xx << std::endl;
        }
    }
};

export ClickGui ClickGuiInstance{};
