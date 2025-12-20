export module AABBShapeComponent;

import Paul;

export struct AABBShapeComponent
{
    Paul::Vec3f min;
    Paul::Vec3f max;
    Paul::Vec2f widthHeight; // x=width, y=height
};
