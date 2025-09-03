module;

export module GuiData;

import SDK;
import Memory;
import OffsetManager;
import Math;

export struct ClientInstance;

export class GuiData {
public:
    Math::Vec2<float>& getScreenSize() {
        return Memory::MemberAt<Math::Vec2<float>>(this, Offsets::GuiData_ScreenSize);
    }

    Math::Vec2<float>& getScreenSizeScaled() {
        return Memory::MemberAt<Math::Vec2<float>>(this, Offsets::GuiData_ScreenSizeScaled);
    }

    float& getGuiScale() {
        return Memory::MemberAt<float>(this, Offsets::GuiData_GuiScale);
    }

    static GuiData* fromClientInstance(ClientInstance* clientInstance) {
        if (!clientInstance) return nullptr;
        return Memory::MemberAt<GuiData*>(clientInstance, Offsets::ClientInstance_guiData);
    }

    static GuiData* fromClientInstance(void* clientInstance) {
        return fromClientInstance(reinterpret_cast<ClientInstance*>(clientInstance));
    }
};

static SDK::CoreRegistration Register{"GuiData"};


