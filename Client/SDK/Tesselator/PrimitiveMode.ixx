
export module PrimitiveMode;
import SDK;


export namespace mce {
    enum class PrimitiveMode : int {
        TriangleList = 0,
        TriangleStrip = 1,
        LineList = 2,
        LineStrip = 3,
        PointList = 4,
    };
}
static SDK::CoreRegistration Register{"PrimitiveMode"};


