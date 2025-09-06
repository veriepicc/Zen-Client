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
import MinecraftUIRenderContext;

export enum class Category
{
    Combat,
    Visual,
    Movement,
    World,
    Player,
    Misc
};

export inline constexpr std::string_view categoryName(Category category)
{
    switch (category)
    {
        case Category::Combat:   return "Combat";
        case Category::Visual:   return "Visual";
        case Category::Movement: return "Movement";
        case Category::World:    return "World";
        case Category::Player:   return "Player";
        case Category::Misc:     return "Misc";
        default:                 return "Unknown";
    }
}

// Export a minimal VK-style constant to avoid including Windows headers in modules
 


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
    int keybindRef() const { return keybind; }

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

    void setKeybind(int key)
    {
        keybind = key;
    }

    virtual void onEnable() {}
    virtual void onDisable() {}
    virtual void onUpdate() {}
    virtual void onRender(MinecraftUIRenderContext* /*rc*/) {}

    using SettingList = std::vector<Setting>;

    SettingList& settingsRef() { return settings; }
    const SettingList& settingsRef() const { return settings; }

    void addSetting(Setting setting)
    {
        settings.emplace_back(std::move(setting));
        const std::size_t index = settings.size() - 1;
        settingIndex.emplace(settings[index].name, index);
    }

protected:
    void Register();

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
    int keybind { 0 };
    SettingList settings;
    std::unordered_map<std::string, std::size_t> settingIndex;
};

export namespace Modules
{
    class ModuleRegistry
    {
    public:
        std::span<Module* const> allModules()
        {
            return { allModulesView.data(), allModulesView.size() };
        }

        std::span<Module* const> modulesByCategory(Category category)
        {
            auto& view = modulesByCategoryView[static_cast<std::size_t>(category)];
            return { view.data(), view.size() };
        }

        Module* findByName(std::string_view name)
        {
            if (auto it = nameToModuleIndex.find(std::string(name)); it != nameToModuleIndex.end()) return it->second;
            return nullptr;
        }

        void addModule(Module* module)
        {
            if (module == nullptr) return;
            if (nameToModuleIndex.find(module->nameRef()) != nameToModuleIndex.end()) return;

            allModulesView.push_back(module);
            nameToModuleIndex.emplace(module->nameRef(), module);
            modulesByCategoryView[static_cast<std::size_t>(module->categoryRef())].push_back(module);
        }
    private:
        std::vector<Module*> allModulesView;
        std::array<std::vector<Module*>, 6> modulesByCategoryView;
        std::unordered_map<std::string, Module*> nameToModuleIndex;
    };

    inline ModuleRegistry& getRegistry()
    {
        static ModuleRegistry instance;
        return instance;
    }

    export inline std::span<Module* const> All()
    {
        return getRegistry().allModules();
    }

    export inline std::span<Module* const> ByCategory(Category category)
    {
        return getRegistry().modulesByCategory(category);
    }

    export inline Module* Find(std::string_view name)
    {
        return getRegistry().findByName(name);
    }

    export inline void Register(Module* module)
    {
        getRegistry().addModule(module);
    }

    export inline void HandleKeyEvent(int key, bool isDown)
    {
        static std::array<bool, 1024> keyDown{};
        if (key < 0 || key >= static_cast<int>(keyDown.size())) return;
        const bool wasDown = keyDown[key];
        keyDown[key] = isDown;

        if (isDown && !wasDown)
        {
            for (Module* module : All())
            {
                if (module && module->keybindRef() == key)
                {
                    module->toggle();
                }
            }
        }
    }

    export inline void RenderTick(MinecraftUIRenderContext* rc)
    {
        for (Module* module : All())
        {
            if (module != nullptr && module->enabledRef())
            {
                module->onRender(rc);
            }
        }
    }
}

inline void Module::Register()
{
    Modules::Register(this);
}

// VK constants moved to Keys module

