module;
#include <string>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <variant>
#include "imgui/imgui.h"
#include <unordered_map>
#include <memory>

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
import Paul;
import MeshHelpers;
import Global;
import Memory;
import SigManager;

export class ClickGui : public Module
{
public:
	bool run = false;

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
		run = false;
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

		//((void(__fastcall*)(_QWORD, __int64, _QWORD))RenderChunkCoordinator::_setAllDirty)(i[3], v5, 0i64);

		//auto theMap = lr->renderChunkCoordinators;
		//
		//for (const auto& pair : theMap) {
		//	pair.second->_setAllDirty();
		//}

        static MaterialPtr* mat = nullptr;
        if (!mat)
        {
            HashedString name("name_tag_depth_tested");
            mat = MaterialPtr::createUIMaterial(name);
            if (!mat) mat = MaterialPtr::createMaterial(name);
        }
        if (!mat) return;

        const auto cam = lr->getLevelRendererPlayer()->getCameraPos();
        static int logEvery = 0;
		bool outline = true;

		auto pos = Paul::Vec3<float>(295.f, -59.f, 93.f);
		auto upper = Paul::Vec3<float>(pos.x + 1.f, pos.y + 1.f, pos.z + 1.f);
		auto lower = Paul::Vec3<float>(pos.x - 1.f, pos.y - 1.f, pos.z - 1.f);

		Paul::Vec3<float> diff = Paul::Vec3<float>(upper.x - lower.x, upper.y - lower.y, upper.z - lower.z); //upper.sub(lower);
		Paul::Vec3<float> newLower = Paul::Vec3<float>(lower.x - cam.x, lower.y - cam.y, lower.z - cam.z);
		Paul::Vec3<float> vertices[8] = {
			{newLower.x, newLower.y, newLower.z},
			{newLower.x + diff.x, newLower.y, newLower.z},
			{newLower.x, newLower.y, newLower.z + diff.z},
			{newLower.x + diff.x, newLower.y, newLower.z + diff.z},

			{newLower.x, newLower.y + diff.y, newLower.z},
			{newLower.x + diff.x, newLower.y + diff.y, newLower.z},
			{newLower.x, newLower.y + diff.y, newLower.z + diff.z},
			{newLower.x + diff.x, newLower.y + diff.y, newLower.z + diff.z} };
		// Transform vertices using Paul
		Paul::Mat4 rotationMatrix = Paul::Mat4::Identity();
		Paul::Vec3<float> newLowerReal = Paul::Vec3<float>(newLower.x + 0.5f, newLower.y + 0.5f, newLower.z + 0.5f);
		for (int i = 0; i < 8; i++) {
			Paul::Vec4f rotatedVertex4 = rotationMatrix.multiply(Paul::Vec4f(
				vertices[i].x - newLowerReal.x,
				vertices[i].y - newLowerReal.y,
				vertices[i].z - newLowerReal.z,
				0.0f
			));
			vertices[i] = Paul::Vec3<float>{ rotatedVertex4.x + newLowerReal.x, rotatedVertex4.y + newLowerReal.y, rotatedVertex4.z + newLowerReal.z };
		}

		tess->begin(mce::PrimitiveMode::QuadList);
		static int v[48] = { 5, 7, 6, 4, 4, 6, 7, 5, 1, 3, 2, 0, 0, 2, 3, 1, 4, 5, 1, 0, 0, 1, 5, 4, 6, 7, 3, 2, 2, 3, 7, 6, 4, 6, 2, 0, 0, 2, 6, 4, 5, 7, 3, 1, 1, 3, 7, 5 };
		for (int i = 0; i < 48; i++) tess->vertex(vertices[v[i]].x, vertices[v[i]].y, vertices[v[i]].z);
		MeshHelpers::renderMeshImmediately(sc, tess, mat);

		if (!outline) return;
		tess->begin(mce::PrimitiveMode::LineList, 12);
#define line(m, n)                      \
	tess->vertex(m.x, m.y, m.z); \
	tess->vertex(n.x, n.y, n.z);

		// Top square
		line(vertices[4], vertices[5]);
		line(vertices[5], vertices[7]);
		line(vertices[7], vertices[6]);
		line(vertices[6], vertices[4]);

		// Bottom Square
		line(vertices[0], vertices[1]);
		line(vertices[1], vertices[3]);
		line(vertices[3], vertices[2]);
		line(vertices[2], vertices[0]);

		// Sides
		line(vertices[0], vertices[4]);
		line(vertices[1], vertices[5]);
		line(vertices[2], vertices[6]);
		line(vertices[3], vertices[7]);

#undef line
		MeshHelpers::renderMeshImmediately(sc, tess, mat);
    }
};

export ClickGui ClickGuiInstance{};
