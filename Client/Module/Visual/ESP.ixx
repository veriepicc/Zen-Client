module;
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <mutex>
#include <string_view>
#include "imgui/imgui.h"

export module ESP;

import Module;
import MinecraftUIRenderContext;
import Global;
import Actor;
import Paul;
import RenderUtils;
import GuiData;
import AABBShapeComponent;
import SDK;
import SetupAndRender;
import TexturePtr;

export class ESP : public Module
{
public:
    ESP() : Module("ESP", "Highlight nearby entities", Category::Visual)
    {
        addSetting(Settings::boolean("3D Boxes", true));
        addSetting(Settings::decimal("Thickness", 2.0f, 0.5f, 5.0f));
        addSetting(Settings::boolean("Names", true));
        addSetting(Settings::boolean("Mart ESP", false));
        Register();
    }

    void onRender(MinecraftUIRenderContext* rc) override
    {
        auto* ci = Global::getClientInstance();
        if (!ci) return;

        auto* gd = GuiData::fromClientInstance(ci);
        if (!gd) return;

        Paul::Vec2f displaySize = gd->getScreenSize();
        if (displaySize.x <= 0 || displaySize.y <= 0) return;

        auto* drawList = ImGui::GetForegroundDrawList();
        
        // Crisp lines
        drawList->Flags &= ~ImDrawListFlags_AntiAliasedLines;
        drawList->Flags &= ~ImDrawListFlags_AntiAliasedFill;

        bool mart = *getSetting<bool>("Mart ESP");
        bool boxes = *getSetting<bool>("3D Boxes");
        bool names = *getSetting<bool>("Names");
        float thickness = *getSetting<float>("Thickness");
        
        TexturePtr* martTex = mart ? Hooks::Render::SetupAndRender::GetMartTexture() : nullptr;
        bool hasMart = martTex && martTex->clientTexture;

        auto actors = Global::getActorList();
        auto* localPlayer = Global::getLocalPlayer();

        for (auto* actor : actors)
        {
            if (actor == localPlayer || !Utils::Mem::isValidPtr(actor)) continue;

            auto* aabb = actor->getAABBShapeComponent();
            if (!aabb) continue;

            // Box points
            Paul::Vec3f corners[8] = {
                {aabb->min.x, aabb->min.y, aabb->min.z}, {aabb->max.x, aabb->min.y, aabb->min.z},
                {aabb->max.x, aabb->min.y, aabb->max.z}, {aabb->min.x, aabb->min.y, aabb->max.z},
                {aabb->min.x, aabb->max.y, aabb->min.z}, {aabb->max.x, aabb->max.y, aabb->min.z},
                {aabb->max.x, aabb->max.y, aabb->max.z}, {aabb->min.x, aabb->max.y, aabb->max.z}
            };

            ImVec2 sc[8];
            bool clipped[8];
            int visibleCorners = 0;

            // Project all 8 points
            for (int i = 0; i < 8; i++) {
                Paul::Vec2f proj;
                if (RenderUtils::WorldToScreen(corners[i], proj, Global::viewMatrix, displaySize, Global::fov, Global::cameraPos)) {
                    sc[i] = ImVec2(proj.x, proj.y);
                    clipped[i] = false;
                    visibleCorners++;
                } else {
                    clipped[i] = true;
                }
            }

            if (visibleCorners == 0) continue;

            if (mart && hasMart)
            {
                const ImU32 color = IM_COL32(255, 255, 255, 240);
                struct Face { int v[4]; float dist; };
                Face faces[6] = {
                    {{4, 0, 1, 5}, 0}, {{6, 2, 3, 7}, 0}, {{7, 4, 5, 6}, 0}, 
                    {{0, 3, 2, 1}, 0}, {{7, 3, 0, 4}, 0}, {{5, 1, 2, 6}, 0} 
                };

                // Simple painter's algo sorting
                for (int i = 0; i < 6; i++) {
                    Paul::Vec3f center = (corners[faces[i].v[0]] + corners[faces[i].v[2]]) * 0.5f;
                    faces[i].dist = (center - Global::cameraPos).lengthSq();
                }

                for (int i = 0; i < 5; i++) {
                    int max = i;
                    for (int j = i + 1; j < 6; j++) if (faces[j].dist > faces[max].dist) max = j;
                    if (max != i) std::swap(faces[i], faces[max]);
                }

                for (int i = 0; i < 6; i++) {
                    const Face& f = faces[i];
                    
                    // Subdivide into 4x4 grid in 3D to fix affine warping
                    constexpr int grid = 4;
                    ImVec2 pts[grid+1][grid+1];
                    bool subClipped[grid+1][grid+1];

                    for (int r = 0; r <= grid; r++) {
                        float ty = (float)r / grid;
                        Paul::Vec3f pStart = Paul::Lerp(corners[f.v[0]], corners[f.v[1]], ty);
                        Paul::Vec3f pEnd   = Paul::Lerp(corners[f.v[3]], corners[f.v[2]], ty);
                        
                        for (int c = 0; c <= grid; c++) {
                            float tx = (float)c / grid;
                            Paul::Vec3f p3 = Paul::Lerp(pStart, pEnd, tx);
                            Paul::Vec2f proj;
                            if (RenderUtils::WorldToScreen(p3, proj, Global::viewMatrix, displaySize, Global::fov, Global::cameraPos)) {
                                pts[r][c] = ImVec2(proj.x, proj.y);
                                subClipped[r][c] = false;
                            } else {
                                subClipped[r][c] = true;
                            }
                        }
                    }

                    // Render grid quads
                    for (int r = 0; r < grid; r++) {
                        for (int c = 0; c < grid; c++) {
                            if (!subClipped[r][c] && !subClipped[r+1][c] && !subClipped[r+1][c+1] && !subClipped[r][c+1]) {
                                drawList->AddImageQuad((ImTextureID)martTex, 
                                    pts[r][c], pts[r+1][c], pts[r+1][c+1], pts[r][c+1],
                                    ImVec2((float)c/grid, (float)r/grid), 
                                    ImVec2((float)c/grid, (float)(r+1)/grid), 
                                    ImVec2((float)(c+1)/grid, (float)(r+1)/grid), 
                                    ImVec2((float)(c+1)/grid, (float)r/grid), 
                                    color);
                            }
                        }
                    }
                }
            }
            else if (boxes)
            {
                const ImU32 color = IM_COL32(255, 255, 255, 255);
                auto edge = [&](int i, int j) {
                    if (!clipped[i] && !clipped[j]) drawList->AddLine(sc[i], sc[j], color, thickness);
                };
                for (int i = 0; i < 4; i++) {
                    edge(i, (i + 1) % 4); 
                    edge(i + 4, ((i + 1) % 4) + 4); 
                    edge(i, i + 4);
                }
            }

            if (names)
            {
                auto* pos = actor->getPosition();
                float dist = pos ? (*pos - Global::cameraPos).length() : 0.0f;
                char info[64]; 
                snprintf(info, sizeof(info), "Entity [%dm]", (int)dist);
                drawList->AddText(ImVec2(sc[4].x, sc[4].y - 15), IM_COL32(255, 255, 0, 255), info);
            }
        }
    }
};

static ESP espInstance;
