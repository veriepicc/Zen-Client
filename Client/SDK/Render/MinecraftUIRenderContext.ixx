module;
export module MinecraftUIRenderContext;

import Memory;
import OffsetManager;
import SDK;
import ScreenContext;

export class MinecraftUIRenderContext
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
static SDK::CoreRegistration Register{"MinecraftUIRenderContext"};