module;
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <span>
#include <array>
#include <unordered_map>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

export module ModuleManager;

import Module;

//Modules
import Killaura;
import ClickGui;
import Flight;
import Xray;
import AutoSprint;
import AutoGG;

export namespace Modules
{
    class Registry;

    Registry& Instance();

    std::span<Module* const> All();
    std::span<Module* const> ByCategory(Category category);
    Module* Find(std::string_view name);

    // Call this periodically to process keybinds and toggle/hold-enable modules
    void PollKeybinds();
}

namespace Modules
{
    class Registry
    {
    public:
        Registry()
        {
            // Combat
            modules_.emplace_back(std::make_unique<Killaura>());

            // Visual
            modules_.emplace_back(std::make_unique<ClickGui>());

            // Movement
            modules_.emplace_back(std::make_unique<Flight>());

            // World
            modules_.emplace_back(std::make_unique<Xray>());

            // Player
            modules_.emplace_back(std::make_unique<AutoSprint>());

            // Misc
            modules_.emplace_back(std::make_unique<AutoGG>());

            allView_.reserve(modules_.size());
            for (auto& m : modules_) {
                Module* raw = m.get();
                allView_.push_back(raw);
                nameIndex_.emplace(raw->nameRef(), raw);
            }

            for (auto* m : allView_) {
                std::size_t idx = static_cast<std::size_t>(m->categoryRef());
                categoryViews_[idx].push_back(m);
            }
        }

        std::span<Module* const> all() { return { allView_.data(), allView_.size() }; }

        std::span<Module* const> byCategory(Category category)
        {
            auto& v = categoryViews_[static_cast<std::size_t>(category)];
            return { v.data(), v.size() };
        }

        Module* find(std::string_view name)
        {
            if (auto it = nameIndex_.find(std::string(name)); it != nameIndex_.end()) return it->second;
            return nullptr;
        }

    private:
        std::vector<std::unique_ptr<Module>> modules_;
        std::vector<Module*> allView_;
        std::array<std::vector<Module*>, 6> categoryViews_;
        std::unordered_map<std::string, Module*> nameIndex_;
    };

    inline Registry& Instance()
    {
        static Registry instance;
        return instance;
    }

    inline std::span<Module* const> All() { return Instance().all(); }

    inline std::span<Module* const> ByCategory(Category category) { return Instance().byCategory(category); }

    inline Module* Find(std::string_view name)
    {
        return Instance().find(name);
    }

    inline void PollKeybinds()
    {
        Keybinds::poll(All());
    }
}


