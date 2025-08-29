module;
#include <cstdint>
#include <cstring>
#include <string>

export module HashedString;
import SDK;

export class HashedString {
public:
    std::uint64_t hash{};
    std::string text;
    const HashedString* unk{};

    HashedString() = default;

    HashedString(std::uint64_t h, std::string s)
        : hash(h), text(std::move(s)), unk(nullptr) {
    }

    explicit HashedString(const std::string& s)
        : text(s), unk(nullptr) {
        computeHash();
    }

    HashedString(const std::string& s, std::uint64_t h)
        : hash(h), text(s), unk(nullptr) {
    }

    std::uint64_t getHash() const { return hash; }

    void computeHash() {
        hash = 0xCBF29CE484222325ull;
        for (unsigned char c : text)
            hash = (hash * 0x100000001B3ull) ^ c;
    }

    bool operator==(const HashedString& rhs) const {
        return text == rhs.text && unk == &rhs && rhs.unk == this;
    }
    bool operator!=(const HashedString& rhs) const { return !(*this == rhs); }

    bool operator<(const HashedString& rhs) const {
        if (hash != rhs.hash) return hash < rhs.hash;
        return std::strcmp(text.c_str(), rhs.text.c_str()) < 0;
    }
};
static SDK::CoreRegistration Register{"HashedString"};
