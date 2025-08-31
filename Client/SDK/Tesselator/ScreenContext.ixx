module;

export module ScreenContext;

import OffsetManager;
import Memory;
export import Tesselator;
import SDK;

struct MCCColor;

export class ScreenContext {
public:
    MCCColor* getColorHolder() {
        return Memory::MemberAt<MCCColor*>(this, Offsets::ScreenContext_colorHolder);
    }


    Tessellator* getTessellator() {
        return Memory::MemberAt<Tessellator*>(this, Offsets::ScreenContext_tessellator);
    }
};
static SDK::CoreRegistration Register{"ScreenContext"};


