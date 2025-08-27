module;



export module ScreenContext;

export import OffsetManager;
import Memory;
export import Tesselator;

struct MCCColor;
class Tessellator;

export class ScreenContext {
public:
    MCCColor* getColorHolder() {
        return Memory::MemberAt<MCCColor*>(this, OffsetManager::ScreenContext_colorHolder);
    }

    Tessellator* getTessellator() {
        return Memory::MemberAt<Tessellator*>(this, OffsetManager::ScreenContext_tessellator);
    }
};


