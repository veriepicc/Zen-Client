export module Color;

import SDK;

export namespace mce {
    class Color {
    public:
        float r{1.0f};
        float g{1.0f};
        float b{1.0f};
        float a{1.0f};
        Color() = default;
        Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
    };
}

static SDK::CoreRegistration Register{"Color"};


