module;
#include "../imgui.h"
#include <string>
#include <windows.h>
#include <wincodec.h>
#include <objidl.h>
#include <iostream>
#include <fstream>

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

namespace BigRatGlue
{
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
            mat = MaterialPtr::createMaterial(hs);
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
    }

    inline void RenderMesh(ScreenContext* sc, Tessellator* tess, MaterialPtr* mat, BedrockTextureData& tex)
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
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) { std::cout << "[BigRat.WIC] CoInitializeEx failed: 0x" << std::hex << hr << std::endl; return false; }
        
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] CoCreateInstance failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pFactory->CreateStream(&pStream);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] CreateStream failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pStream->InitializeFromFilename(path, GENERIC_WRITE);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] InitializeFromFilename failed: 0x" << std::hex << hr << std::endl; goto cleanup; }

        hr = pFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &pEncoder);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] CreateEncoder failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] Encoder::Initialize failed: 0x" << std::hex << hr << std::endl; goto cleanup; }

        hr = pEncoder->CreateNewFrame(&pFrame, &pProps);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] CreateNewFrame failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pFrame->Initialize(pProps);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] Frame::Initialize failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pFrame->SetSize(width, height);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] SetSize failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pFactory->CreateBitmapFromMemory(
            width, height, GUID_WICPixelFormat32bppRGBA,
            static_cast<UINT>(width * 4), static_cast<UINT>(width * 4 * height),
            const_cast<BYTE*>(pixels), &pBitmap);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] CreateBitmapFromMemory failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pFrame->WriteSource(pBitmap, nullptr);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] WriteSource failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pFrame->Commit();
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] Frame::Commit failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pEncoder->Commit();
        if (FAILED(hr)) { std::cout << "[BigRat.WIC] Encoder::Commit failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
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
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) { std::cout << "[BigRat.WIC.Mem] CoInitializeEx failed: 0x" << std::hex << hr << std::endl; return out; }
        
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] CoCreateInstance failed: 0x" << std::hex << hr << std::endl; goto cleanup; }

        hr = CreateStreamOnHGlobal(nullptr, TRUE, &pStream);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] CreateStreamOnHGlobal failed: 0x" << std::hex << hr << std::endl; goto cleanup; }

        hr = pFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &pEncoder);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] CreateEncoder failed: 0x" << std::hex << hr << std::endl; goto cleanup; }

        hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] Encoder::Initialize failed: 0x" << std::hex << hr << std::endl; goto cleanup; }

        hr = pEncoder->CreateNewFrame(&pFrame, &pProps);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] CreateNewFrame failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pFrame->Initialize(pProps);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] Frame::Initialize failed: 0x" << std::hex << hr << std::endl; goto cleanup; }

        hr = pFrame->SetSize(width, height);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] SetSize failed: 0x" << std::hex << hr << std::endl; goto cleanup; }

        hr = pFactory->CreateBitmapFromMemory(
            width, height, GUID_WICPixelFormat32bppRGBA,
            static_cast<UINT>(width * 4), static_cast<UINT>(width * 4 * height),
            const_cast<BYTE*>(pixels), &pBitmap);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] CreateBitmapFromMemory failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pFrame->WriteSource(pBitmap, nullptr);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] WriteSource failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pFrame->Commit();
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] Frame::Commit failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
        hr = pEncoder->Commit();
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] Encoder::Commit failed: 0x" << std::hex << hr << std::endl; goto cleanup; }

        hr = GetHGlobalFromStream(pStream, &hMem);
        if (FAILED(hr)) { std::cout << "[BigRat.WIC.Mem] GetHGlobalFromStream failed: 0x" << std::hex << hr << std::endl; goto cleanup; }
        
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
    bool Init(MinecraftUIRenderContext* rc)
    {
        if (ImGui::GetCurrentContext() == nullptr)
        {
            ImGui::CreateContext();
            std::cout << "[BigRat] Created ImGui context" << std::endl;
        }
        IMGUI_CHECKVERSION();
        GetState()->rc = rc;
        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererName = "imgui_impl_bigrat";
        io.IniFilename = nullptr; // avoid off-screen persisted state

        // Log relevant vtable slots to verify indices
        void** vt = *reinterpret_cast<void***>(rc);
        std::cout << "[BigRat] vtbl[29]=" << vt[29] << " [30]=" << vt[30] << " [31]=" << vt[31] << " [33]=" << vt[33] << std::endl;

        if (io.Fonts->Fonts.Size == 0)
        {
            io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 18.0f);
            if (io.Fonts->Fonts.Size == 0)
            {
                io.Fonts->AddFontDefault();
                std::cout << "[BigRat] Fallback default font added" << std::endl;
            }
            else
            {
                std::cout << "[BigRat] Added segoeui.ttf" << std::endl;
            }
        }

        // Build RGBA32 atlas
        unsigned char* pixels = nullptr; int w = 0, h = 0; int bpp = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h, &bpp);
        std::cout << "[BigRat] Atlas size=" << w << "x" << h << " bpp=" << bpp << " pixels=" << (void*)pixels << std::endl;
        if (!(pixels && w > 0 && h > 0)) return true;

        // First try in-memory PNG bytes using WIC + HGLOBAL stream
        std::string pngBytes = BigRatGlue::SaveRGBAtoPNGToMemory(pixels, w, h);
        std::cout << "[BigRat] Memory blob size=" << pngBytes.size() << std::endl;
        if (!pngBytes.empty())
        {
            // Use a unique namespace not colliding with default packs
            TexturePtr tp;
            ResourceLocation rl("bigrat/imgui_atlas", /*external*/false);
            rc->getZippedTexture(tp, pngBytes, rl, /*forceReload*/true);
            std::cout << "[BigRat] Uploaded via RL='bigrat/imgui_atlas' -> clientTexture=" << (void*)tp.clientTexture.get() << std::endl;

            // Nudge the engine and fetch by RL to ensure pointer resolution
            rc->touchTexture(rl);
            TexturePtr fetched;
            rc->getTexture(fetched, rl, /*forceReload*/false);
            std::cout << "[BigRat] Fetched by RL -> clientTexture=" << (void*)fetched.clientTexture.get() << std::endl;

            if (fetched.clientTexture)
            {
                GetState()->fontTexture = fetched;
                io.Fonts->SetTexID((void*)GetState()->fontTexture.clientTexture.get());
                return true;
            }
        }

        // Fallback to file-based path
        const wchar_t* atlasPath = L"C:/Users/veriepic/AppData/Local/Packages/Microsoft.MinecraftUWP_8wekyb3d8bbwe/RoamingState/imgui_atlas.png";
        bool fileOk = BigRatGlue::SaveRGBAtoPNG(atlasPath, pixels, w, h);
        std::cout << "[BigRat] Save atlas PNG " << (fileOk ? "ok" : "fail") << std::endl;
        if (fileOk)
        {
            GetState()->fontTexture = rc->createTexture("C:/Users/veriepic/AppData/Local/Packages/Microsoft.MinecraftUWP_8wekyb3d8bbwe/RoamingState/imgui_atlas.png", true, true);
            std::cout << "[BigRat] Loaded atlas via file: clientTexture=" << (void*)GetState()->fontTexture.clientTexture.get() << std::endl;
            if (GetState()->fontTexture.clientTexture)
            {
                io.Fonts->SetTexID((void*)GetState()->fontTexture.clientTexture.get());
                return true;
            }
        }

        return true;
    }

    void Shutdown()
    {
        GetState()->rc = nullptr;
    }

    void NewFrame(float delta_time, float display_w, float display_h, float scale_x, float scale_y)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = (delta_time > 0.f) ? delta_time : (1.0f / 60.0f);
        io.DisplaySize = ImVec2(display_w, display_h);
        io.DisplayFramebufferScale = ImVec2(scale_x, scale_y);
        static int frame = 0; if ((frame++ % 60) == 0)
            std::cout << "[BigRat] NewFrame: size=" << display_w << "x" << display_h << " scale=" << scale_x << "," << scale_y << std::endl;
    }

    void RenderDrawData(ImDrawData* draw_data, MinecraftUIRenderContext* rc)
    {
        if (!rc || draw_data->CmdListsCount == 0)
        {
            std::cout << "[BigRat] RenderDrawData: empty rc=" << (void*)rc << " lists=" << (draw_data ? draw_data->CmdListsCount : -1) << std::endl;
            return;
        }

        ScreenContext* sc = BigRatGlue::GetScreenContext(rc);
        Tessellator* tess = BigRatGlue::GetTessellator(sc);
        MaterialPtr* material = BigRatGlue::GetUITexturedMaterial();
        std::cout << "[BigRat] Render: lists=" << draw_data->CmdListsCount << " totalVtx=" << draw_data->TotalVtxCount
                  << " sc=" << (void*)sc << " tess=" << (void*)tess << " mat=" << (void*)material << std::endl;

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

                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                                (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                                (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                BedrockTextureData* texture = (BedrockTextureData*)pcmd->GetTexID();
                if (logged < 8)
                {
                    std::cout << "[BigRat] cmd: elem=" << pcmd->ElemCount << " clip=(" << clip_min.x << "," << clip_min.y << ")-(" << clip_max.x << ") tex=" << (void*)texture << std::endl;
                    ++logged;
                }

                BigRatGlue::PushClip(rc);
                auto rect = SDK::RectangleArea::FromLTRB(clip_min.x, clip_min.y, clip_max.x, clip_max.y);
                BigRatGlue::SetClip(rc, rect);

                if (!texture) { BigRatGlue::PopClip(rc); continue; }

                tess->begin(mce::PrimitiveMode::TriangleList, (int)pcmd->ElemCount, false);
                for (unsigned int idx = 0; idx < pcmd->ElemCount; idx += 3)
                {
                    const ImDrawIdx i0 = idx_buffer[pcmd->IdxOffset + idx + 0];
                    const ImDrawIdx i1 = idx_buffer[pcmd->IdxOffset + idx + 1];
                    const ImDrawIdx i2 = idx_buffer[pcmd->IdxOffset + idx + 2];
                    const ImDrawVert& v0 = vtx_buffer[pcmd->VtxOffset + i0];
                    const ImDrawVert& v1 = vtx_buffer[pcmd->VtxOffset + i1];
                    const ImDrawVert& v2 = vtx_buffer[pcmd->VtxOffset + i2];
                    tess->color((float)((v0.col)&0xFF)/255.f, (float)((v0.col>>8)&0xFF)/255.f, (float)((v0.col>>16)&0xFF)/255.f, (float)((v0.col>>24)&0xFF)/255.f);
                    tess->vertexUV(v0.pos.x, v0.pos.y, 0.0f, v0.uv.x, v0.uv.y);
                    tess->color((float)((v1.col)&0xFF)/255.f, (float)((v1.col>>8)&0xFF)/255.f, (float)((v1.col>>16)&0xFF)/255.f, (float)((v1.col>>24)&0xFF)/255.f);
                    tess->vertexUV(v1.pos.x, v1.pos.y, 0.0f, v1.uv.x, v1.uv.y);
                    tess->color((float)((v2.col)&0xFF)/255.f, (float)((v2.col>>8)&0xFF)/255.f, (float)((v2.col>>16)&0xFF)/255.f, (float)((v2.col>>24)&0xFF)/255.f);
                    tess->vertexUV(v2.pos.x, v2.pos.y, 0.0f, v2.uv.x, v2.uv.y);
                }

                BigRatGlue::RenderMesh(sc, tess, material, *texture);
                BigRatGlue::PopClip(rc);
            }
        }
    }

    void Demo(MinecraftUIRenderContext* rc, float delta_time, float display_w, float display_h, float scale_x = 1.0f, float scale_y = 1.0f)
    {
        if (!rc) return;
        NewFrame(delta_time, display_w, display_h, scale_x, scale_y);
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(360, 200), ImGuiCond_Always);
        ImGui::Begin("BigRat Demo");
        ImGui::TextUnformatted("Hello from BigRat backend");
        static float v = 0.5f;
        ImGui::SliderFloat("Value", &v, 0.f, 1.f);
        ImGui::End();
        ImGui::Render();
        std::cout << "[BigRat] Demo: lists=" << (ImGui::GetDrawData() ? ImGui::GetDrawData()->CmdListsCount : -1) << std::endl;
        RenderDrawData(ImGui::GetDrawData(), rc);
    }
}


