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

    using Rect = RectT<float>;
}
