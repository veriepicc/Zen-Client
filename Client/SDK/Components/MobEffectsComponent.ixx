module;
#include <vector>
#include <cstdint>

export module MobEffectsComponent;

export enum class MobEffectType : std::uint32_t 
{
    Empty = 0,
    Speed = 1,
    Slowness = 2,
    Haste = 3,
    MiningFatigue = 4,
    Strength = 5,
    InstantHealth = 6,
    InstantDamage = 7,
    JumpBoost = 8,
    Nausea = 9,
    Regeneration = 10,
    Resistance = 11,
    FireResistance = 12,
    WaterBreathing = 13,
    Invisibility = 14,
    Blindness = 15,
    NightVision = 16,
    Hunger = 17,
    Weakness = 18,
    Poison = 19,
    Wither = 20,
    HealthBoost = 21,
    Absorption = 22,
    Saturation = 23,
    Levitation = 24,
    FatalPoison = 25,
    ConduitPower = 26,
    SlowFalling = 27,
    BadOmen = 28,
    VillageHero = 29,
    Darkness = 30,
    TrialOmen = 31,
    WindCharged = 32,
    Weaving = 33,
    Oozing = 34,
    Infested = 35,
    RaidOmen = 36
};

// Version 1.21.30+ structure (likely same for 1.21.100)
export struct MobEffectInstance 
{
    MobEffectType id;
    int duration;
    float unknown;
    int durationEasy;
    int durationNormal;
    int durationHard;
    char padding[0x8];
    int amplifier;
    bool displayOnScreenTextureAnimation;
    bool ambient;
    bool noCounter;
    bool effectVisible;
    char finalPadding[0x60];
};

export struct MobEffectsComponent 
{
    std::vector<MobEffectInstance> effects;
};
