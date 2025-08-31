#pragma once
#include "../SDK.hpp"

namespace mce {
    enum class PrimitiveMode : int {
		None = 0,
		QuadList = 1,
		TriangleList = 2,
		TriangleStrip = 3,
		LineList = 4,
		LineStrip = 5
    };
}
//static SDK::CoreRegistration Register{"PrimitiveMode"};


