export module RectangleArea;

import SDK;

export namespace SDK {
    // Note: Memory layout matches engine's RectangleArea: x, width, y, height
    struct RectangleArea {
        float left;   // x0
        float right;  // x1
        float top;    // y0
        float bottom; // y1

        constexpr RectangleArea() : left(0), right(0), top(0), bottom(0) {}
        
        // Internal constructor matching engine layout
        constexpr RectangleArea(float l, float r, float t, float b)
            : left(l), right(r), top(t), bottom(b) {}

        // Proper LTRB helper
        static constexpr RectangleArea FromLTRB(float l, float t, float r, float b)
        {
            return RectangleArea(l, r, t, b);
        }
    };
}

static SDK::CoreRegistration Register{"RectangleArea"};


