#pragma once

#include "Memory/Memory.hpp"
#include "Memory/SigManager.hpp"
#include "Memory/OffsetManager.hpp"
#include "Utils/Math.hpp"
#include "../SDK.hpp"

#include "../Tessellator/ScreenContext.hpp"

class MinecraftUIRenderContext
{
public:
    void* getClientInstance()
    {
        return Memory::MemberAt<void*>(this, Offsets::MinecraftUIRenderContext_clientInstance);
    }

    ScreenContext* getScreenContext()
    {
        return Memory::MemberAt<ScreenContext*>(this, Offsets::MinecraftUIRenderContext_screenContext);
    }
};
//static SDK::CoreRegistration Register{"MinecraftUIRenderContext"};