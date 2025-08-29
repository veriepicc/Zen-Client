module;
export module MinecraftUIRenderContext;

import Memory;
import OffsetManager;
import SDK;

export class MinecraftUIRenderContext
{
public:
    void* getScreenContext()
    {
        return Memory::MemberAt<void*>(this, Offsets::MinecraftUIRenderContext_screenContext);
    }
};
static SDK::CoreRegistration Register{"MinecraftUIRenderContext"};