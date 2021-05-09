#pragma once
#include <cinttypes>

#include "jka/JKAAnims.h"

namespace JKA {
    // Animations (quintessence of lerpFrame_t + CG_RunLerpFrame() + etc.)
    struct AnimationState {
        animNumber_t anim = BOTH_STAND1;
        int32_t startTime = 0;
        int32_t endTime = 0;

        float getAnimationProgress(int32_t currentTime) const;  // 0 for the start of anim  -->  1 for the end
        bool startNewAnimation(animNumber_t newAnim, int32_t currentTime);
        bool startNewAnimation(int32_t newAnim, int32_t currentTime);
    };
}
