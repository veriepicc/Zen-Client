module;
#include <cstdint>
#include <string>

export module ResourceLocation;

import SDK;

export enum class ResourceFileSystem : std::int32_t {
    UserPackage = 0x0000,
    AppPackage = 0x0001,
    Raw = 0x0002,
    RawPersistent = 0x0003,
    SettingsDir = 0x0004,
    ExternalDir = 0x0005,
    ServerPackage = 0x0006,
    DataDir = 0x0007,
    UserDir = 0x0008,
    ScreenshotsDir = 0x0009,
    StoreCache = 0x000a,
    Invalid = 0x000b,
};

export class ResourceLocation {
public:
    ResourceFileSystem fileSystem{};
    std::string filePath;
    std::uint64_t pathHash{};
    std::uint64_t fullHash{};

    ResourceLocation() = default;

    ResourceLocation(const std::string& filePath, bool external) {
        this->filePath = filePath;
        if (external)
            this->fileSystem = ResourceFileSystem::Raw;
        else this->fileSystem = ResourceFileSystem::UserPackage;

        computeHashes();
    }

    bool operator==(const ResourceLocation& other) const {
        if (this->fileSystem != other.fileSystem || this->pathHash != other.pathHash) return false;
        return this->filePath == other.filePath;
    }

    struct Less {
        bool operator()(const ResourceLocation& a, const ResourceLocation& b) const noexcept {
            if (a.fileSystem != b.fileSystem) return static_cast<int>(a.fileSystem) < static_cast<int>(b.fileSystem);
            if (a.pathHash != b.pathHash) return a.pathHash < b.pathHash;
            return a.filePath < b.filePath;
        }
    };

private:
    void computeHashes()
    {
        const std::uint64_t FNV_OFFSET_BASIS = 0xCBF29CE484222325ull;
        const std::uint64_t FNV_PRIME = 0x100000001B3ull;

        std::uint64_t path = FNV_OFFSET_BASIS;
        if (!this->filePath.empty()) {
            for (unsigned char c : this->filePath) {
                path *= FNV_PRIME;
                path ^= c;
            }
        } else {
            path = 0;
        }

        std::uint64_t fileSystemHash = FNV_OFFSET_BASIS ^ static_cast<std::uint8_t>(this->fileSystem);
        fileSystemHash *= FNV_PRIME;

        this->pathHash = path;
        this->fullHash = path ^ fileSystemHash;
    }
};

static SDK::CoreRegistration Register{"ResourceLocation"};


