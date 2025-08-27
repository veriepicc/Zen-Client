export module MinecraftUIRenderContext;

import Memory;
import Math;
import HashedString;
#include <string>
#include <cstdint>
class TextMeasureData {
public:
    float scale;
    int idk;
    bool displayShadow;
public:
    TextMeasureData(float scale = 1.f, bool shaddow = true) {
        this->scale = scale;
        this->displayShadow = shaddow;
    };
};

class CaretMeasureData {
public:
    int a;
    int b;
public:
    CaretMeasureData(int paramA = 0xFFFFFFFF, bool paramB = false) {
        this->a = paramA;
        this->b = paramB;
    };
};

class FilePath {//I pasted
private:
    char pad_0x0[0x18];
public:
    std::string filePathStr;
};

class TexturePtr {
private:
    char pad_0x0[0x18];
public:
    FilePath* filePath;
};

enum class ResourceFileSystem : int { //nrg sent me
    UserPackage = 0x0,
    AppPackage = 0x1,
    Raw = 0x2,
    RawPersistent = 0x3,
    SettingsDir = 0x4,
    ExternalDir = 0x5,
    ServerPackage = 0x6,
    DataDir = 0x7,
    UserDir = 0x8,
    ScreenshotsDir = 0x9,
    StoreCache = 0xA,
    Invalid = 0xB,
};

class ResourceLocation
{
public:
    ResourceFileSystem type; //0x0
private:
    char fill[0x4];
public:
    std::string path; //0x8
    __int64 pathHash;
    __int64 fullHash;
public:
    ResourceLocation(ResourceFileSystem type, std::string path) {
        this->type = type;
        this->path = path;
    }

};
export class MinecraftUIRenderContext {
public:
    void* clientInstance;
    void* screenContext;

private:
    virtual ~MinecraftUIRenderContext() = default;

public:
    virtual float getLineLength(class Font*, const std::string&, float, bool);
    virtual float getTextAlpha();
    virtual void setTextAlpha(float);// I REMOVED COLOR, ADD IT BACK LATER
    virtual __int64 drawDebugText(const float*, const std::string&, const float*, float, unsigned int, float*, void*);// I REMOVED COLOR, ADD IT BACK LATER
    virtual __int64 drawText(Font*, const float*, const std::string&, const float*, float, unsigned int, const TextMeasureData*, const CaretMeasureData*);// I REMOVED COLOR, ADD IT BACK LATER
    virtual void flushText(float);
    virtual __int64 drawImage(TexturePtr*, Math::Vec2<float>*, Math::Vec2<float>*, Math::Vec2<float>*, Math::Vec2<float>*); // I REMOVED COLOR, ADD IT BACK LATER

private:
    virtual void Function8();

public:
    virtual void flushImages(const float*, float, HashedString);

private:
    virtual void Function10();
    virtual void Function11();

public:
    virtual void drawRectangle(const float*, const float*, float, int);
    virtual void fillRectangle(const Math::Rect, const float*, float);

};
// I REMOVED COLOR, ADD IT BACK LATER// I REMOVED COLOR, ADD IT BACK LATER
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
// I REMOVED COLOR, ADD IT BACK LATER!
