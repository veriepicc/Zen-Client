module;
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

export module OffsetManager;

#define REGISTER_OFFSET(Name, Value) \
public: \
static inline std::uint32_t Name; \
private: \
static inline std::function<void()> Name##Reg = ( \
    initializers.emplace_back([&]() { Name = Value; offsets[#Name] = Name; }), \
    std::function<void()>() \
); \
public:

export class OffsetManager
{
private:
    static inline std::vector<std::function<void()>> initializers;
    static inline std::unordered_map<std::string, std::uint32_t> offsets;

public:
    static void initialize()
    {
        for (const auto& init : initializers) init();
    }

    static void deinitialize()
    {
        initializers.clear();
        offsets.clear();
    }

    static const std::unordered_map<std::string, std::uint32_t>& getOffsets()
    {
        return offsets;
    }

    // One-liner offsets
    REGISTER_OFFSET(ClientInstance_guiData, 0x5B0)
};


