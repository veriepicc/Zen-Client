module;
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <variant>
#include <optional>
#include <utility>
#include <functional>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <mutex>
#include <span>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

export module Module;

import Utils;

export enum class Category
{
    Combat,
    Visual,
    Movement,
    World,
    Player,
    Misc
};

namespace detail
{
    static constexpr std::array<std::string_view, 6> CategoryNames = {
        "Combat", "Visual", "Movement", "World", "Player", "Misc"
    };
}

export inline constexpr std::string_view categoryName(Category category)
{
    return detail::CategoryNames[static_cast<std::size_t>(category)];
}

export struct Setting
{
    std::string name;
    std::variant<bool, int, float, Color, std::string> value;
    std::optional<std::pair<int, int>> intBounds;
    std::optional<std::pair<float, float>> floatBounds;
};

export namespace Settings
{
    inline Setting boolean(std::string name, bool value)
    {
        return Setting { std::move(name), value, std::nullopt, std::nullopt };
    }

    inline Setting integer(std::string name, int value, int minValue, int maxValue)
    {
        return Setting { std::move(name), value, std::make_pair(minValue, maxValue), std::nullopt };
    }

    inline Setting decimal(std::string name, float value, float minValue, float maxValue)
    {
        return Setting { std::move(name), value, std::nullopt, std::make_pair(minValue, maxValue) };
    }

    inline Setting color(std::string name, Color value)
    {
        return Setting { std::move(name), value, std::nullopt, std::nullopt };
    }

    inline Setting text(std::string name, std::string value)
    {
        return Setting { std::move(name), std::move(value), std::nullopt, std::nullopt };
    }
}

export struct Keybind
{
    int vk { 0 }; // Virtual-Key code (Win32)
    bool hold { false }; // If true, enable while held; otherwise toggle on press
};

export class Module
{
public:
    Module(std::string name, std::string description, Category category)
        : name(std::move(name)), description(std::move(description)), category(category) {}

    virtual ~Module() = default;

    const std::string& nameRef() const { return name; }
    const std::string& descriptionRef() const { return description; }
    Category categoryRef() const { return category; }
    bool enabledRef() const { return enabled; }

    void toggle()
    {
        setEnabled(!enabled);
    }

    void setEnabled(bool enable)
    {
        if (enabled == enable) return;
        enabled = enable;
        if (enabled) onEnable(); else onDisable();
    }

    virtual void onEnable() {}
    virtual void onDisable() {}
    virtual void onUpdate() {}
    virtual void onRender() {}

    using SettingList = std::vector<Setting>;

    SettingList& settingsRef() { return settings; }
    const SettingList& settingsRef() const { return settings; }

    void addSetting(Setting setting)
    {
        settings.emplace_back(std::move(setting));
        const std::size_t index = settings.size() - 1;
        settingIndex.emplace(settings[index].name, index);
    }

    template <typename T>
    T* getSetting(std::string_view settingName)
    {
        if (auto it = settingIndex.find(std::string(settingName)); it != settingIndex.end())
        {
            Setting& s = settings[it->second];
            if (auto* ptr = std::get_if<T>(&s.value)) return ptr;
        }
        return nullptr;
    }
    void setKeybind(Keybind kb)
    {
        keybind = kb;
    }

    Keybind keybindRef() const { return keybind; }

private:
    std::string name;
    std::string description;
    Category category { Category::Misc };
    bool enabled { false };
    SettingList settings;
    std::unordered_map<std::string, std::size_t> settingIndex;
    Keybind keybind {};
};

export namespace Keybinds
{
    void poll(std::span<Module* const> modules);
}

namespace Keybinds
{
    namespace detail
    {
        static std::mutex stateMutex;
        static std::unordered_map<int, bool> downByVk;
    }

    inline void poll(std::span<Module* const> modules)
    {
        std::lock_guard<std::mutex> lock(detail::stateMutex);
        std::unordered_set<int> usedKeys;
        usedKeys.reserve(modules.size());

        for (auto* mod : modules)
        {
            if (!mod) continue;
            const Keybind kb = mod->keybindRef();
            if (kb.vk <= 0) continue;
            usedKeys.insert(kb.vk);

            const bool isDown = (GetAsyncKeyState(kb.vk) & 0x8000) != 0;
            const bool wasDown = detail::downByVk[kb.vk];

            if (kb.hold)
            {
                mod->setEnabled(isDown);
            }
            else
            {
                if (isDown && !wasDown) mod->toggle();
            }

            detail::downByVk[kb.vk] = isDown;
        }

        for (auto it = detail::downByVk.begin(); it != detail::downByVk.end(); )
        {
            if (!usedKeys.contains(it->first)) it = detail::downByVk.erase(it);
            else ++it;
        }
    }
}

