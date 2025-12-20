module;
#include <chrono>

export module MoveInputComponent;

import Paul;


export struct MoveInputState
{
    bool sneakDown;
    bool sneakToggleDown;
    bool wantDownSlow;
    bool wantUpSlow;
    bool blockSelectDown;
    bool ascendBlock;
    bool descendBlock;
    bool jumpDown;
    bool sprintDown;
    bool upLeft;
    bool upRight;
    bool downLeft;
    bool downRight;
    bool forward;
    bool backward;
    bool left;
    bool right;
    bool ascend;
    bool descend;
    bool changeHeight;
    Paul::Vec2<float> analogMoveVector;
    bool lookCenter;
    unsigned char lookSlightDirField;
    unsigned char lookNormalDirField;
    unsigned char lookSmoothDirField;
    bool sneakInputCurrentlyDown;
    bool sneakInputWasReleased;
    bool sneakInputWasPressed;
    bool jumpInputWasReleased;
    bool jumpInputWasPressed;
    bool jumpInputCurrentlyDown;
};


export struct MoveInputComponent
{
    MoveInputState inputState;
    MoveInputState rawInputState;
    std::chrono::steady_clock::time_point holdAutoJumpInWaterUntil;
    Paul::Vec2<float> moveVector;
    Paul::Vec2<float> lookDelta;
    Paul::Vec2<float> interactDirection;
    Paul::Vec3<float> displacement;
    Paul::Vec3<float> displacementDelta;
    Paul::Vec3<float> cameraOrientation;
    bool sneaking;
    bool sprinting;
    bool wantUp;
    bool wantDown;
    bool jumping;
    bool autoJumpingInWater;
    bool moveInputStateLocked;
    bool persistSneak;
    bool autoJumpEnabled;
    bool isCameraRelativeMovementEnabled;
    bool isRotationControlledByMoveDirection;
    bool isPaddling[2];
};
