export module Math;

export namespace Math {

    template <typename T>
    struct Vec2 {
        T x, y;

        Vec2() : x(0), y(0) {}
        Vec2(T x, T y) : x(x), y(y) {}
    };

    template <typename T>
    struct RectT {
        Vec2<T> min;
        Vec2<T> max;
    };

    template <typename T>
    struct Vec3 {
        T x, y, z;

        Vec3() : x(0), y(0), z(0) {}
        Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    };

    using Rect = RectT<float>;
}
