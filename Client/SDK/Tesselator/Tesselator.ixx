module;

export module Tesselator;

import Paul;
import SigManager;
import PrimitiveMode;
import Memory;
import SDK;

export class Tessellator {
public:
    void begin(mce::PrimitiveMode vertexFormat = mce::PrimitiveMode::TriangleList, int maxVertices = 0, bool buildFaceData = false) {
        void* target = SigManager::Tessellator_begin_b ? SigManager::Tessellator_begin_b : SigManager::Tessellator_begin_a;
        Memory::CallFunc<void, void*, mce::PrimitiveMode, int, bool>(target, this, vertexFormat, maxVertices, buildFaceData);
    }

    void vertex(float x, float y, float z) {
        void* target = SigManager::Tessellator_vertex;
        Memory::CallFunc<void, void*, float, float, float>(target, this, x, y, z);
    }

    void vertexUV(float x, float y, float z, float u, float v) {
        void* target = SigManager::Tessellator_vertexUV_b ? SigManager::Tessellator_vertexUV_b : SigManager::Tessellator_vertexUV_a;
        Memory::CallFunc<void, void*, float, float, float, float, float>(target, this, x, y, z, u, v);
    }

    void color(float r, float g, float b, float a) {
        void* target = SigManager::Tessellator_colorF;
        Memory::CallFunc<void, void*, float, float, float, float>(target, this, r, g, b, a);
    }

    void setRotation(float angle, Paul::Vec3<float> pivot) {
        void* target = SigManager::Tessellator_setRotation;
        Memory::CallFunc<void, void*, float, Paul::Vec3<float>>(target, this, angle, pivot);
    }

    void resetTransform(bool a1) {
        void* target = SigManager::Tessellator_resetTransform;
        Memory::CallFunc<void, void*, bool>(target, this, a1);
    }
};

static SDK::CoreRegistration Register{"Tesselator"};


