module;
#include <unordered_map>
#include <string>
#include <cstdint>

export module AttributesComponent;

import HashedString;

export class AttributeInstance 
{
public:
    // This value is usually at a specific offset.
    // In Flarial it uses a dynamic offset.
    // We'll just define the structure for now.
    char padding[0x100]; // Placeholder for full instance structure
    
    float getValue() const
    {
        // Assuming the value is at offset 0x84 (common for 1.21)
        return *reinterpret_cast<const float*>(reinterpret_cast<std::uintptr_t>(this) + 0x84);
    }
};

export class Attribute 
{
public:
    std::int8_t redefinitionNode = 0;
    bool syncable = true;
    short unknown = 0;
    int idValue;
    HashedString name;

    Attribute(int id, const std::string& attrName) : idValue(id), name(attrName) {}
};

export class BaseAttributeMap 
{
public:
    std::unordered_map<unsigned int, AttributeInstance> instances;
    char padding[0x20];
};

export struct AttributesComponent 
{
    BaseAttributeMap baseAttributes;
};
