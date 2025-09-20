module;
#include <string>

export module Command;

export class Command
{
public:
    Command(std::string name, std::string description)
        : name(std::move(name)), description(std::move(description)) {
    }

    virtual ~Command() = default;

private:
    std::string name;
    std::string description;
};