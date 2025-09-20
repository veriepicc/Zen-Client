module;
#include <memory>
#include <map>
#include <string>
export module MinecraftUIRenderContext;

import ClientInstance;

import Memory;
import OffsetManager;
import SDK;
import ScreenContext;
import TextureGroup;
import TexturePtr;
import ResourceLocation;
import BedrockTexture;
import BedrockTextureData;
import Color;
import HashedString;
import Paul;
import RectangleArea;


export class MinecraftUIRenderContext
{
public:
    ClientInstance* getClientInstance()
    {
        return Memory::MemberAt<ClientInstance*>(this, Offsets::MinecraftUIRenderContext_clientInstance);
    }

    ScreenContext* getScreenContext()
    {
        return Memory::MemberAt<ScreenContext*>(this, Offsets::MinecraftUIRenderContext_screenContext);
    }

    std::shared_ptr<TextureGroup>& getTextureGroup()
    {
        return Memory::MemberAt<std::shared_ptr<TextureGroup>>(this, Offsets::MinecraftUIRenderContext_textures);
    }

    std::map<ResourceLocation, BedrockTexture, ResourceLocation::Less>& getTextures()
    {
        return getTextureGroup()->getLoadedTextures();
    }

    TexturePtr& getTexture(TexturePtr& outPtr, const ResourceLocation& location, bool forceReload)
    {
        return Memory::CallVFunc<TexturePtr&, TexturePtr&, const ResourceLocation&, bool>(29, this, outPtr, location, forceReload);
    }

    // Upload a texture from in-memory bytes (e.g., PNG). Vtable index based on SDK layout (immediately after getTexture).
    TexturePtr& getZippedTexture(TexturePtr& outPtr, const std::string& bytes, const ResourceLocation& location, bool forceReload)
    {
        return Memory::CallVFunc<TexturePtr&, TexturePtr&, const std::string&, const ResourceLocation&, bool>(30, this, outPtr, bytes, location, forceReload);
    }

    TexturePtr createTexture(const std::string& path, bool external, bool forceReload)
    {
        const auto resource = ResourceLocation(path, external);
        TexturePtr texture;
        getTexture(texture, resource, forceReload);
        return texture;
    }

    TexturePtr createTexture(const ResourceLocation& location, bool forceReload)
    {
        TexturePtr texture;
        getTexture(texture, location, forceReload);
        return texture;
    }

    void drawImage(TexturePtr& texture, Paul::Vec2<float>& pos, Paul::Vec2<float>& size, Paul::Vec2<float>& uvPos, Paul::Vec2<float>& uvSize)
    {
        Memory::CallVFunc<void, BedrockTextureData&, Paul::Vec2<float>&, Paul::Vec2<float>&, Paul::Vec2<float>&, Paul::Vec2<float>&, bool>(7, this, *texture.clientTexture, pos, size, uvPos, uvSize, false);
    }

    void flushImages(mce::Color &color, float opacity, HashedString &hashedString)
    {
        Memory::CallVFunc<void, mce::Color&, float, HashedString&>(9, this, color, opacity, hashedString);
    }

    void enableScissorTest(const SDK::RectangleArea& rect)
    {
        Memory::CallVFunc<void, const SDK::RectangleArea&>(18, this, rect);
    }

    void disableScissorTest()
    {
        Memory::CallVFunc<void>(19, this);
    }

    void setClippingRectangle(const SDK::RectangleArea& rect)
    {
        Memory::CallVFunc<void, const SDK::RectangleArea&>(20, this, rect);
    }

    void setFullClippingRectangle()
    {
        Memory::CallVFunc<void>(21, this);
    }

    void saveCurrentClippingRectangle()
    {
        Memory::CallVFunc<void>(22, this);
    }

    void restoreSavedClippingRectangle()
    {
        Memory::CallVFunc<void>(23, this);
    }

    void reloadTexture(const ResourceLocation& location)
    {
        (void)this->createTexture(location, true);
    }

    void reloadAllTextures()
    {
        auto& textures = this->getTextures();
        for (auto& kv : textures) {
            auto& resourceLocation = kv.first;
            auto& texture = kv.second;
            texture.unload();
            reloadTexture(resourceLocation);
        }
    }

    void unloadTexture(const ResourceLocation& location)
    {
        Memory::CallVFunc<void, const ResourceLocation&>(31, this, location);
    }

    void touchTexture(const ResourceLocation& location)
    {
        Memory::CallVFunc<void, const ResourceLocation&>(33, this, location);
    }

    void swapTextures(ResourceLocation& from, ResourceLocation& to)
    {
        auto& textures = this->getTextures();

        if (!textures.contains(from)) {
            reloadTexture(from);
        }

        if (!textures.contains(to)) {
            reloadTexture(to);
        }

        if (textures.contains(from) && textures.contains(to) && textures[from] != textures[to]) {
            if (textures[to].bedrockTextureData && textures[to].bedrockTextureData->textureLoadState == TextureLoadState::LoadedBit) {
                textures[from] = textures[to];
            } else {
                reloadTexture(to);
                if (textures[to].bedrockTextureData && textures[to].bedrockTextureData->textureLoadState == TextureLoadState::LoadedBit) {
                    textures[from] = textures[to];
                }
            }
        }
    }
};
static SDK::CoreRegistration Register{"MinecraftUIRenderContext"};