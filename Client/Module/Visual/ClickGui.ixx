module;
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
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
import TexturePtr;
import ResourceLocation;
import Color;
import BedrockTextureData;

export class ClickGui : public Module
{
private:
    bool worldImageInitialized = false;
    BedrockTextureData* worldImageTexture = nullptr;

public:
    ClickGui()
        : Module("ClickGui", "Displays the clickable GUI for modules.", Category::Visual)
    {
        addSetting(Settings::color("ThemeColor", Color(146, 102, 204)));
        addSetting(Settings::decimal("Scale", 1.0f, 0.5f, 2.0f));
        addSetting(Settings::boolean("Rainbow", false));
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
    void onRender(MinecraftUIRenderContext* rc) override 
    {
        // Try to get texture from the same source as SetupAndRender "Mart" window
        if (!worldImageInitialized && rc)
        {
            initializeWorldImageTexture(rc);
        }
    }

    void onWorldRender(LevelRenderer* lr, ScreenContext* sc) override
    {
        if (!enabledRef()) return;
        if (!lr || !sc) return;

        Tessellator* tess = sc->getTessellator();
        if (!tess) return;

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

		auto pos = Math::Vec3<float>(295.f, -59.f, 93.f);
		auto upper = Math::Vec3<float>(pos.x + 1.f, pos.y + 1.f, pos.z + 1.f);
		auto lower = Math::Vec3<float>(pos.x - 1.f, pos.y - 1.f, pos.z - 1.f);

		Math::Vec3<float> diff = Math::Vec3<float>(upper.x - lower.x, upper.y - lower.y, upper.z - lower.z); //upper.sub(lower);
		Math::Vec3<float> newLower = Math::Vec3<float>(lower.x - cam.x, lower.y - cam.y, lower.z - cam.z);
		Math::Vec3<float> vertices[8] = {
			{newLower.x, newLower.y, newLower.z},
			{newLower.x + diff.x, newLower.y, newLower.z},
			{newLower.x, newLower.y, newLower.z + diff.z},
			{newLower.x + diff.x, newLower.y, newLower.z + diff.z},

			{newLower.x, newLower.y + diff.y, newLower.z},
			{newLower.x + diff.x, newLower.y + diff.y, newLower.z},
			{newLower.x, newLower.y + diff.y, newLower.z + diff.z},
			{newLower.x + diff.x, newLower.y + diff.y, newLower.z + diff.z} };
		// Scale vertices using glm
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.f), 0.f, glm::vec3(1.0f, 1.0f, 1.0f));
		Math::Vec3<float> newLowerReal = Math::Vec3<float>(newLower.x + 0.5f, newLower.y + 0.5f, newLower.z + 0.5f);//newLower.add(0.5f, 0.5f, 0.5f);  // .add(0.4375f, 0.4375f, 0.4375f) is chest
		for (int i = 0; i < 8; i++) {
			glm::vec4 rotatedVertex = rotationMatrix * glm::vec4(vertices[i].x - newLowerReal.x, vertices[i].y - newLowerReal.y, vertices[i].z - newLowerReal.z, 0.0f);
			vertices[i] = Math::Vec3{ rotatedVertex.x + newLowerReal.x, rotatedVertex.y + newLowerReal.y, rotatedVertex.z + newLowerReal.z };
		}

		// Render Mart panel if we have the texture, otherwise wireframe cube
		if (worldImageTexture)
		{
			renderMartPanel(lr, sc, tess);
		}
		else
		{
			tess->begin(mce::PrimitiveMode::QuadList);
			static int v[48] = { 5, 7, 6, 4, 4, 6, 7, 5, 1, 3, 2, 0, 0, 2, 3, 1, 4, 5, 1, 0, 0, 1, 5, 4, 6, 7, 3, 2, 2, 3, 7, 6, 4, 6, 2, 0, 0, 2, 6, 4, 5, 7, 3, 1, 1, 3, 7, 5 };
			for (int i = 0; i < 48; i++) tess->vertex(vertices[v[i]].x, vertices[v[i]].y, vertices[v[i]].z);
			MeshHelpers::renderMeshImmediately(sc, tess, mat);
		}

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

private:
    void initializeWorldImageTexture(MinecraftUIRenderContext* rc)
    {
        std::string base = Utils::GetRoamingPath();
        if (!base.empty())
        {
            for (char& ch : base) if (ch == '\\') ch = '/';
            const std::string path = base + "/image.png";
            auto rl = std::make_shared<ResourceLocation>(ResourceLocation(path, true));
            
            // Try to create and load the texture
            TexturePtr tp = rc->createTexture(path, true, true);
            rc->touchTexture(*rl);
            
            TexturePtr fetched;
            rc->getTexture(fetched, *rl, false);
            if (fetched.clientTexture)
            {
                // Try to get BedrockTextureData from the TexturePtr
                worldImageTexture = fetched.clientTexture.get();
                worldImageInitialized = true;
                std::cout << "[ClickGui] World image texture loaded successfully" << std::endl;
            }
            else
            {
                std::cout << "[ClickGui] Failed to load world image texture: " << path << std::endl;
                std::cout << "[ClickGui] Make sure image.png exists in your AppData/Roaming folder" << std::endl;
            }
        }
    }

    void renderMartPanel(LevelRenderer* lr, ScreenContext* sc, Tessellator* tess)
    {
        // Get a textured material
        static MaterialPtr* texMat = nullptr;
        static bool materialDebugPrinted = false;
        if (!texMat)
        {
            HashedString names[] = {
                HashedString("ui_textured"),
                HashedString("ui"),
                HashedString("name_tag_depth_tested"),
                HashedString("entity_alphatest"),
                HashedString("entity"),
                HashedString("basic")
            };
            
            // Try UI materials first
            for (int i = 0; i < 2 && !texMat; i++) {
                texMat = MaterialPtr::createUIMaterial(names[i]);
                if (texMat && !materialDebugPrinted) {
                    std::cout << "[ClickGui] Using UI material for Mart panel: " << (i == 0 ? "ui_textured" : "ui") << std::endl;
                    materialDebugPrinted = true;
                }
            }
            
            // Try regular materials
            for (int i = 2; i < 6 && !texMat; i++) {
                texMat = MaterialPtr::createMaterial(names[i]);
                if (texMat && !materialDebugPrinted) {
                    std::cout << "[ClickGui] Using material for Mart panel: " << 
                        (i == 2 ? "name_tag_depth_tested" : 
                         i == 3 ? "entity_alphatest" :
                         i == 4 ? "entity" : "basic") << std::endl;
                    materialDebugPrinted = true;
                }
            }
            
            if (!texMat && !materialDebugPrinted) {
                std::cout << "[ClickGui] Failed to create any material for Mart panel" << std::endl;
                materialDebugPrinted = true;
            }
        }
        if (!texMat) return;

        const auto cam = lr->getLevelRendererPlayer()->getCameraPos();
        
        // Create a simple vertical panel at a clean block position
        auto centerPos = Math::Vec3<float>(295.5f, -58.0f, 93.5f); // Centered on a block
        float panelWidth = 2.0f;
        float panelHeight = 3.0f;
        
        // Create vertical panel vertices (facing north, adjust as needed)
        Math::Vec3<float> panelVertices[4] = {
            {centerPos.x - panelWidth/2 - cam.x, centerPos.y - cam.y,                centerPos.z - cam.z}, // Bottom-left
            {centerPos.x + panelWidth/2 - cam.x, centerPos.y - cam.y,                centerPos.z - cam.z}, // Bottom-right
            {centerPos.x + panelWidth/2 - cam.x, centerPos.y + panelHeight - cam.y,  centerPos.z - cam.z}, // Top-right
            {centerPos.x - panelWidth/2 - cam.x, centerPos.y + panelHeight - cam.y,  centerPos.z - cam.z}  // Top-left
        };

        // UV coordinates for texture mapping
        Math::Vec2<float> uv[4] = {
            {0.0f, 1.0f}, // Bottom-left
            {1.0f, 1.0f}, // Bottom-right
            {1.0f, 0.0f}, // Top-right
            {0.0f, 0.0f}  // Top-left
        };

        tess->begin(mce::PrimitiveMode::QuadList);
        
        // Ensure fully opaque color
        tess->color(1.0f, 1.0f, 1.0f, 1.0f);
        
        // Add vertices with UV coordinates for single vertical panel
        for (int i = 0; i < 4; i++)
        {
            tess->vertexUV(panelVertices[i].x, panelVertices[i].y, panelVertices[i].z, uv[i].x, uv[i].y);
        }
        
        // Render with texture binding
        static bool renderDebugPrinted = false;
        if (!renderDebugPrinted) {
            std::cout << "[ClickGui] Rendering Mart vertical panel!  (Flat)" << std::endl;
            renderDebugPrinted = true;
        }
        MeshHelpers::renderMeshImmediately2(sc, tess, texMat, *worldImageTexture);
    }
};

export ClickGui ClickGuiInstance{};
