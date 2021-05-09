#pragma once
#include <cinttypes>

#include "jka/JKAAnims.h"

// Animations (quintessence of lerpFrame_t + CG_RunLerpFrame() + etc.)
struct AnimationState {
    JKA::animNumber_t anim = JKA::BOTH_STAND1;
    int32_t startTime = 0;
    int32_t endTime = 0;

    float getAnimationProgress(int32_t currentTime) const;  // 0 for the start of anim  -->  1 for the end
    bool startNewAnimation(JKA::animNumber_t newAnim, int32_t currentTime);
    bool startNewAnimation(int32_t newAnim, int32_t currentTime);
};
