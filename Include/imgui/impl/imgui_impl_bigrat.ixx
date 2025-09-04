module;
#include "../imgui.h"
#include <string>
#include <windows.h>
#include <wincodec.h>
#include <objidl.h>

export module imgui_impl_bigrat;

import MinecraftUIRenderContext;
import ScreenContext;
import Tesselator;
import MeshHelpers;
import MaterialPtr;
import HashedString;
import PrimitiveMode;
import RectangleArea;
import BedrockTextureData;
import TexturePtr;
import Math;
import Color;
import Utils;
import TextureGroup;
import BedrockTexture;
import OffsetManager;
import GuiData;

namespace BigRatGlue
{
    static constexpr bool BR_DISABLE_CLIP = true;
    inline ScreenContext* GetScreenContext(MinecraftUIRenderContext* rc)
    {
        return rc->getScreenContext();
    }

    inline Tessellator* GetTessellator(ScreenContext* sc)
    {
        return sc->getTessellator();
    }

    inline MaterialPtr* GetUITexturedMaterial()
    {
        static MaterialPtr* mat = nullptr;
        if (!mat)
        {
            HashedString hs("ui_textured");
            mat = MaterialPtr::createUIMaterial(hs);
            if (!mat)
            {
                const char* candidates[] = {
                    "ui_textured",
                    "ui_textured_alpha",
                    "ui_textured_unlit",
                    "ui_textured_and_blend"
                };
                for (auto* name : candidates)
                {
                    HashedString h(name);
                    auto* m = MaterialPtr::createMaterial(h);
                    if (m) { mat = m; break; }
                }
            }
        }
        return mat;
    }

    inline void SetClip(MinecraftUIRenderContext* rc, const SDK::RectangleArea& rect)
    {
        rc->setClippingRectangle(rect);
        rc->enableScissorTest(rect);
    }

    inline void PushClip(MinecraftUIRenderContext* rc)
    {
        rc->saveCurrentClippingRectangle();
    }

    inline void PopClip(MinecraftUIRenderContext* rc)
    {
        rc->restoreSavedClippingRectangle();
        rc->disableScissorTest();
    }

    inline void RenderMesh(ScreenContext* sc, Tessellator* tess, MaterialPtr* mat)
    {
        MeshHelpers::renderMeshImmediately(sc, tess, mat);
    }

    inline void RenderMeshTextured(ScreenContext* sc, Tessellator* tess, MaterialPtr* mat, BedrockTextureData& tex)
    {
        MeshHelpers::renderMeshImmediately2(sc, tess, mat, tex);
    }

    static bool SaveRGBAtoPNG(const wchar_t* path, const unsigned char* pixels, int width, int height)
    {
        IWICImagingFactory* pFactory = nullptr;
        IWICBitmapEncoder* pEncoder = nullptr;
        IWICBitmapFrameEncode* pFrame = nullptr;
        IPropertyBag2* pProps = nullptr;
        IWICStream* pStream = nullptr;
        IWICBitmap* pBitmap = nullptr;
        bool ok = false;
        HRESULT hr;

        hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) { return false; }
        
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pFactory->CreateStream(&pStream);
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pStream->InitializeFromFilename(path, GENERIC_WRITE);
        if (FAILED(hr)) { goto cleanup; }

        hr = pFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &pEncoder);
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
        if (FAILED(hr)) { goto cleanup; }

        hr = pEncoder->CreateNewFrame(&pFrame, &pProps);
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pFrame->Initialize(pProps);
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pFrame->SetSize(width, height);
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pFactory->CreateBitmapFromMemory(
            width, height, GUID_WICPixelFormat32bppRGBA,
            static_cast<UINT>(width * 4), static_cast<UINT>(width * 4 * height),
            const_cast<BYTE*>(pixels), &pBitmap);
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pFrame->WriteSource(pBitmap, nullptr);
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pFrame->Commit();
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pEncoder->Commit();
        if (FAILED(hr)) { goto cleanup; }
        
        ok = true;
    cleanup:
        if (pBitmap) pBitmap->Release();
        if (pFrame) pFrame->Release();
        if (pEncoder) pEncoder->Release();
        if (pStream) pStream->Release();
        if (pProps) pProps->Release();
        if (pFactory) pFactory->Release();
        CoUninitialize();
        return ok;
    }

    static std::string SaveRGBAtoPNGToMemory(const unsigned char* pixels, int width, int height)
    {
        std::string out;
        IWICImagingFactory* pFactory = nullptr;
        IWICBitmapEncoder* pEncoder = nullptr;
        IWICBitmapFrameEncode* pFrame = nullptr;
        IPropertyBag2* pProps = nullptr;
        IWICBitmap* pBitmap = nullptr;
        IStream* pStream = nullptr;
        HGLOBAL hMem = nullptr;
        HRESULT hr;

        hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) { return out; }
        
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
        if (FAILED(hr)) { goto cleanup; }

        hr = CreateStreamOnHGlobal(nullptr, TRUE, &pStream);
        if (FAILED(hr)) { goto cleanup; }

        hr = pFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &pEncoder);
        if (FAILED(hr)) { goto cleanup; }

        hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
        if (FAILED(hr)) { goto cleanup; }

        hr = pEncoder->CreateNewFrame(&pFrame, &pProps);
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pFrame->Initialize(pProps);
        if (FAILED(hr)) { goto cleanup; }

        hr = pFrame->SetSize(width, height);
        if (FAILED(hr)) { goto cleanup; }

        hr = pFactory->CreateBitmapFromMemory(
            width, height, GUID_WICPixelFormat32bppRGBA,
            static_cast<UINT>(width * 4), static_cast<UINT>(width * 4 * height),
            const_cast<BYTE*>(pixels), &pBitmap);
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pFrame->WriteSource(pBitmap, nullptr);
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pFrame->Commit();
        if (FAILED(hr)) { goto cleanup; }
        
        hr = pEncoder->Commit();
        if (FAILED(hr)) { goto cleanup; }

        hr = GetHGlobalFromStream(pStream, &hMem);
        if (FAILED(hr)) { goto cleanup; }
        
        if (hMem)
        {
            STATSTG st = {};
            if (SUCCEEDED(pStream->Stat(&st, STATFLAG_NONAME)))
            {
                SIZE_T sz = static_cast<SIZE_T>(st.cbSize.QuadPart);
                void* data = GlobalLock(hMem);
                if (data && sz > 0)
                {
                    out.assign(reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + sz);
                    GlobalUnlock(hMem);
                }
            }
        }

    cleanup:
        if (pBitmap) pBitmap->Release();
        if (pFrame) pFrame->Release();
        if (pEncoder) pEncoder->Release();
        if (pProps) pProps->Release();
        if (pStream) pStream->Release();
        if (pFactory) pFactory->Release();
        CoUninitialize();
        return out;
    }
}

struct BigRatState
{
    MinecraftUIRenderContext* rc{};
    TexturePtr fontTexture{};
};

static BigRatState* GetState()
{
    static BigRatState s; return &s;
}

export namespace ImGui_ImplBigRat
{
    bool LoadFontAtlas(MinecraftUIRenderContext* rc) {
        ImGuiIO& io = ImGui::GetIO();

        unsigned char* pixels = nullptr; int w = 0, h = 0; int bpp = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h, &bpp);
        if (!(pixels && w > 0 && h > 0)) return true;

        std::string atlasPath = GetRoamingPath() + "\\imgui_atlas.png";

        GetState()->fontTexture = rc->createTexture(atlasPath.c_str(), true, true);
        if (GetState()->fontTexture.clientTexture)
        {
            io.Fonts->SetTexID((void*)&GetState()->fontTexture);
            return true;
        }

        std::string pngBytes = BigRatGlue::SaveRGBAtoPNGToMemory(pixels, w, h);
        if (!pngBytes.empty())
        {
            TexturePtr tp;
            ResourceLocation rl("bigrat/imgui_atlas", false);
            rc->getZippedTexture(tp, pngBytes, rl, /*forceReload*/true);
            rc->touchTexture(rl);
            TexturePtr fetched;
            rc->getTexture(fetched, rl, /*forceReload*/false);
            if (fetched.clientTexture)
            {
                GetState()->fontTexture = fetched;
                io.Fonts->SetTexID((void*)&GetState()->fontTexture);
                return true;
            }
        }
    }

    bool Init(MinecraftUIRenderContext* rc)
    {
        if (ImGui::GetCurrentContext() == nullptr)
        {
            ImGui::CreateContext();
        }
        IMGUI_CHECKVERSION();
        GetState()->rc = rc;
        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererName = "imgui_impl_bigrat";
        io.IniFilename = nullptr;

        //

        ImGui::GetStyle().AntiAliasedLines = false;
        ImGui::GetStyle().AntiAliasedFill = true;

        if (io.Fonts->Fonts.Size == 0)
        {
            io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 48.0f);
            if (io.Fonts->Fonts.Size == 0)
            {
                io.Fonts->AddFontDefault();
            }
        }

        unsigned char* pixels = nullptr; int w = 0, h = 0; int bpp = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h, &bpp);
        if (!(pixels && w > 0 && h > 0)) return true;

        std::string atlasPath = GetRoamingPath() + "\\imgui_atlas.png";
        bool fileOk = BigRatGlue::SaveRGBAtoPNG(std::wstring(atlasPath.begin(), atlasPath.end()).c_str(), pixels, w, h);

        if (fileOk)
            return LoadFontAtlas(rc);
        return false;
    }

    void Shutdown()
    {
        GetState()->rc = nullptr;
    }

    void AddMousePosEvent(float x, float y)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(x, y);
    }

    void AddMouseButtonEvent(int button, bool down)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(button, down);
    }

    void AddMouseWheelEvent(float wheel)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseWheelEvent(0.0f, wheel);
    }

    void AddKeyEvent(int imguiKey, bool down)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent((ImGuiKey)imguiKey, down);
    }

    void AddInputCharactersUTF8(const char* text)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharactersUTF8(text);
    }

    void NewFrame(float delta_time, float display_w, float display_h, float scale_x, float scale_y)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = (delta_time > 0.f) ? delta_time : (1.0f / 60.0f);

        if (GetState()->rc)
        {
            void* ci = GetState()->rc->getClientInstance();
            if (ci)
            {
                GuiData* gd = GuiData::fromClientInstance(ci);
                if (gd)
                {
                    auto ss = gd->getScreenSize();
                    auto sss = gd->getScreenSizeScaled();
                    float gs = gd->getGuiScale();
                    if (ss.x > 0 && ss.y > 0)
                    {
                        io.DisplaySize = ImVec2(ss.x, ss.y);
                        if (ss.x != 0.0f && ss.y != 0.0f && sss.x > 0 && sss.y > 0)
                        {
                            io.DisplayFramebufferScale = ImVec2(
                                sss.x / ss.x,
                                sss.y / ss.y
                            );
                        }
                        else
                        {
                            io.DisplayFramebufferScale = ImVec2(gs, gs);
                        }
                    }
                    else
                    {
                        io.DisplaySize = ImVec2(display_w, display_h);
                        io.DisplayFramebufferScale = ImVec2(scale_x, scale_y);
                    }
                }
                else
                {
                    io.DisplaySize = ImVec2(display_w, display_h);
                    io.DisplayFramebufferScale = ImVec2(scale_x, scale_y);
                }
            }
            else
            {
                io.DisplaySize = ImVec2(display_w, display_h);
                io.DisplayFramebufferScale = ImVec2(scale_x, scale_y);
            }
        }
        else
        {
            io.DisplaySize = ImVec2(display_w, display_h);
            io.DisplayFramebufferScale = ImVec2(scale_x, scale_y);
        }
    }

    void RenderDrawData(ImDrawData* draw_data, MinecraftUIRenderContext* rc)
    {
        static int i = 0;
        i++;

        if (i > 300) {
            LoadFontAtlas(rc);
            i = 0;
        }

        if (!rc || draw_data->CmdListsCount == 0)
        {
            return;
        }

        ScreenContext* sc = BigRatGlue::GetScreenContext(rc);
        Tessellator* tess = BigRatGlue::GetTessellator(sc);
        if (tess) {
            tess->resetTransform(false);
        }
        MaterialPtr* material = BigRatGlue::GetUITexturedMaterial();
        rc->saveCurrentClippingRectangle();

        ImVec2 clip_off = draw_data->DisplayPos;
        ImVec2 clip_scale = draw_data->FramebufferScale;

        int logged = 0;
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
            const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback) continue;

                TexturePtr* texPtr = reinterpret_cast<TexturePtr*>(pcmd->GetTexID());
                BedrockTextureData* texture = texPtr && texPtr->clientTexture ? texPtr->clientTexture.get() : nullptr;

                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply or disable scissor per flag
                if constexpr (!BigRatGlue::BR_DISABLE_CLIP)
                {
                    auto rect = SDK::RectangleArea::FromLTRB(clip_min.x, clip_min.y, clip_max.x, clip_max.y);
                    BigRatGlue::SetClip(rc, rect);
                }

                // ImGui indices are 16-bit absolute; pass 0 for maxVertices
                tess->begin(mce::PrimitiveMode::TriangleList, 0, true);
                tess->resetTransform(false);
                for (unsigned int idx = 0; idx < pcmd->ElemCount; idx += 3)
                {
                    // Use 2,1,0 for CW winding
                    const ImDrawIdx i0 = idx_buffer[pcmd->IdxOffset + idx + 2];
                    const ImDrawIdx i1 = idx_buffer[pcmd->IdxOffset + idx + 1];
                    const ImDrawIdx i2 = idx_buffer[pcmd->IdxOffset + idx + 0];
                    const ImDrawVert& v0 = vtx_buffer[pcmd->VtxOffset + i0];
                    const ImDrawVert& v1 = vtx_buffer[pcmd->VtxOffset + i1];
                    const ImDrawVert& v2 = vtx_buffer[pcmd->VtxOffset + i2];
                    auto colToFloat = [](unsigned int c){
                        return ImVec4(
                            (float)((c)&0xFF)/255.f,
                            (float)((c>>8)&0xFF)/255.f,
                            (float)((c>>16)&0xFF)/255.f,
                            (float)((c>>24)&0xFF)/255.f
                        );
                    };
                    ImVec4 c0 = colToFloat(v0.col);
                    ImVec4 c1 = colToFloat(v1.col);
                    ImVec4 c2 = colToFloat(v2.col);
                    // Convert to framebuffer space
                    const float x0 = (v0.pos.x - clip_off.x) * clip_scale.x;
                    const float y0 = (v0.pos.y - clip_off.y) * clip_scale.y;
                    const float x1 = (v1.pos.x - clip_off.x) * clip_scale.x;
                    const float y1 = (v1.pos.y - clip_off.y) * clip_scale.y;
                    const float x2 = (v2.pos.x - clip_off.x) * clip_scale.x;
                    const float y2 = (v2.pos.y - clip_off.y) * clip_scale.y;
                    tess->color(c0.x, c0.y, c0.z, c0.w);
                    tess->vertexUV(x0, y0, 0.0f, v0.uv.x, v0.uv.y);
                    tess->color(c1.x, c1.y, c1.z, c1.w);
                    tess->vertexUV(x1, y1, 0.0f, v1.uv.x, v1.uv.y);
                    tess->color(c2.x, c2.y, c2.z, c2.w);
                    tess->vertexUV(x2, y2, 0.0f, v2.uv.x, v2.uv.y);
                }

        if (texture)
        {
            BigRatGlue::RenderMeshTextured(sc, tess, material, *texture);
        }
        else
        {
            BigRatGlue::RenderMesh(sc, tess, material);
        }
            // After drawing, disable scissor if it was enabled
            if constexpr (!BigRatGlue::BR_DISABLE_CLIP)
                rc->disableScissorTest();
        }
    }
    // Restore whatever clipping state the game had before our pass
    rc->restoreSavedClippingRectangle();
}

    void Demo(MinecraftUIRenderContext* rc, float delta_time, float display_w, float display_h, float scale_x = 1.0f, float scale_y = 1.0f)
    {
        if (!rc) return;
        NewFrame(delta_time, display_w, display_h, scale_x, scale_y);
        ImGui::NewFrame();
        bool show = true;
        ImGui::ShowDemoWindow(&show);
        //ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
        //ImGui::SetNextWindowSize(ImVec2(360, 200), ImGuiCond_FirstUseEver);
        //ImGui::Begin("BigRat Demo");
        //ImGui::TextUnformatted("Hello from BigRat backend");
        //static float v = 0.5f;
        //ImGui::SliderFloat("Value", &v, 0.f, 1.f);
        //ImGui::Text("Tex OK: %s", GetState()->fontTexture.clientTexture ? "yes" : "no");
        //ImGui::Text("RL ptr: %p", GetState()->fontTexture.clientTexture.get());

        ImVec2 center_of_circle = ImVec2(100.0f, 100.0f);
        float circle_radius = 50.0f;
        ImU32 circle_color_outline = IM_COL32(255, 0, 0, 255); // Red outline
        ImU32 circle_color_filled = IM_COL32(0, 255, 0, 255); // Green filled
        int segments = 32; // For a reasonably smooth circle
        float line_thickness = 2.0f;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Draw an outlined circle
        draw_list->AddCircle(center_of_circle, circle_radius, circle_color_outline, segments, line_thickness);

        // Draw a filled circle (offset for visibility)
        draw_list->AddCircleFilled(ImVec2(center_of_circle.x + 120.0f, center_of_circle.y), circle_radius, circle_color_filled, segments);

        ImGui::EndFrame();
        //ImGui::End();
        ImGui::Render();
        RenderDrawData(ImGui::GetDrawData(), rc);
    }
}


