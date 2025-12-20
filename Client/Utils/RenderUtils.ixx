module;
#include <cmath>

export module RenderUtils;

import Paul;
import GLMatrix;

export namespace RenderUtils
{
    inline bool WorldToScreen(Paul::Vec3f worldPos, Paul::Vec2f& screenPos, const GLMatrix& viewMatrix, const Paul::Vec2f& displaySize, Paul::Vec2f fov, Paul::Vec3f cameraPos)
    {
        Paul::Mat4 mat;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                mat.m[i][j] = viewMatrix.matrix[i + j * 4];
            }
        }

        Paul::Vec3f relativePos = worldPos - cameraPos;
        Paul::Vec4f clipSpacePos = mat.multiply(Paul::Vec4f(relativePos.x, relativePos.y, relativePos.z, 1.0f));

        if (clipSpacePos.z >= 0.0f) return false;

        float mX = displaySize.x / 2.0f;
        float mY = displaySize.y / 2.0f;

        screenPos.x = mX + (mX * clipSpacePos.x / -clipSpacePos.z * fov.x);
        screenPos.y = mY - (mY * clipSpacePos.y / -clipSpacePos.z * fov.y);

        return true;
    }
}
