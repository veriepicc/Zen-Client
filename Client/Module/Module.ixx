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
#include <cstdint>
#include <span>

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
private:
    std::string name;
    std::string description;
    Category category { Category::Misc };
    bool enabled { false };
    SettingList settings;
    std::unordered_map<std::string, std::size_t> settingIndex;
};

