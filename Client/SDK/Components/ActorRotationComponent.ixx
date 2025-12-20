export module ActorRotationComponent;

import Paul;

export struct ActorRotationComponent
{
    Paul::Vec2f rotation;      // x=pitch, y=yaw
    Paul::Vec2f previousRotation;
};
