export module RectangleArea;

import SDK;

export namespace SDK {
    // Note: Memory layout matches engine's RectangleArea: x, width, y, height
    struct RectangleArea {
        float left;   // x
        float right;  // width
        float top;    // y
        float bottom; // height
        constexpr RectangleArea() : left(0), right(0), top(0), bottom(0) {}
        // Construct from x, y, width, height
        constexpr RectangleArea(float x, float y, float width, float height)
            : left(x), right(width), top(y), bottom(height) {}
        // Helper to construct from LTRB
        static constexpr RectangleArea FromLTRB(float l, float t, float r, float b)
        {
            return RectangleArea(l, t, r, b);
        }
    };
}

static SDK::CoreRegistration Register{"RectangleArea"};


