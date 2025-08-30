module;
export module MinecraftUIRenderContext;

import Memory;
import OffsetManager;
import SDK;

export class MinecraftUIRenderContext
{
public:
    void* getClientInstance()
    {
        return Memory::MemberAt<void*>(this, Offsets::MinecraftUIRenderContext_clientInstance);
    }

    void* getScreenContext()
    {
        return Memory::MemberAt<void*>(this, Offsets::MinecraftUIRenderContext_screenContext);
    }
};
static SDK::CoreRegistration Register{"MinecraftUIRenderContext"};