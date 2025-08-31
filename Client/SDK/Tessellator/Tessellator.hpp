#pragma once

#include "PrimitiveMode.hpp"
#include "Memory/Memory.hpp"
#include "Memory/SigManager.hpp"
#include "Utils/Math.hpp"
#include "../SDK.hpp"

class Tessellator {
public:
    void begin(mce::PrimitiveMode vertexFormat = mce::PrimitiveMode::TriangleList, int maxVertices = 0, bool buildFaceData = false) {
        void* target = SigManager::Tessellator_begin;
        Memory::call_func_fastcall<void, void*, mce::PrimitiveMode, int, bool>(target, this, vertexFormat, maxVertices, buildFaceData);
    }

    void vertex(float x, float y, float z) {
        void* target = SigManager::Tessellator_vertex;
        Memory::call_func_fastcall<void, void*, float, float, float>(target, this, x, y, z);
    }

    void vertexUV(float x, float y, float z, float u, float v) {
        void* target = SigManager::Tessellator_vertexUV_b ? SigManager::Tessellator_vertexUV_b : SigManager::Tessellator_vertexUV_a;
        Memory::call_func_fastcall<void, void*, float, float, float, float, float>(target, this, x, y, z, u, v);
    }

    void color(float r, float g, float b, float a) {
        void* target = SigManager::Tessellator_colorF;
        Memory::call_func_fastcall<void, void*, float, float, float, float>(target, this, r, g, b, a);
    }

    void setRotation(float angle, Math::Vec3<float> pivot) {
        void* target = SigManager::Tessellator_setRotation;
        Memory::call_func_fastcall<void, void*, float, Math::Vec3<float>>(target, this, angle, pivot);
    }

    void resetTransform(bool a1) {
        void* target = SigManager::Tessellator_resetTransform;
        Memory::call_func_fastcall<void, void*, bool>(target, this, a1);
    }
};

//static SDK::CoreRegistration Register{"Tesselator"};


