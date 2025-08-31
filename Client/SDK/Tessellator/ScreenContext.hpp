#pragma once

#include "Memory/Memory.hpp"
#include "Memory/SigManager.hpp"
#include "Utils/Math.hpp"
#include "../SDK.hpp"

#include "Memory/OffsetManager.hpp"
#include "Tessellator.hpp"

struct MCCColor;

class ScreenContext {
public:
    MCCColor* getColorHolder() {
        return Memory::MemberAt<MCCColor*>(this, Offsets::ScreenContext_colorHolder);
    }


    Tessellator* getTessellator() {
        return Memory::MemberAt<Tessellator*>(this, Offsets::ScreenContext_tessellator);
    }
};
//static SDK::CoreRegistration Register{"ScreenContext"};


