module;

export module GuiData;

import SDK;
import Memory;
import OffsetManager;
import Paul;

export struct ClientInstance;

export class GuiData {
public:
    Paul::Vec2<float>& getScreenSize() {
        return Memory::MemberAt<Paul::Vec2<float>>(this, Offsets::GuiData_ScreenSize);
    }

    Paul::Vec2<float>& getScreenSizeScaled() {
        return Memory::MemberAt<Paul::Vec2<float>>(this, Offsets::GuiData_ScreenSizeScaled);
    }

    float& getGuiScale() {
        return Memory::MemberAt<float>(this, Offsets::GuiData_GuiScale);
    }

    static inline GuiData* fromClientInstance(ClientInstance* clientInstance) {
        if (!clientInstance) return nullptr;
        return Memory::MemberAt<GuiData*>(clientInstance, Offsets::ClientInstance_guiData);
    }

    static inline GuiData* fromClientInstance(void* clientInstance) {
        return fromClientInstance(reinterpret_cast<ClientInstance*>(clientInstance));
    }
};

static SDK::CoreRegistration Register{"GuiData"};


