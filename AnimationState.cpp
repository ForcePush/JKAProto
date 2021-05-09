#include "AnimationState.h"

#include <cassert>
#include <cmath>

float AnimationState::getAnimationProgress(int32_t currentTime) const
{
    // TODO: highly inaccurate, need to check for speedScale
    // TODO: inter/extrapolation
    // TODO: looping
    auto diff = endTime - startTime;
    if (diff == 0) {
        return 0.0f;
    }

    if (currentTime < startTime) {
        return 0.0f;
    }

    if (currentTime > endTime) {
        return 1.0f;
    }

    return static_cast<float>(currentTime - startTime) / static_cast<float>(diff);
}

bool AnimationState::startNewAnimation(JKA::animNumber_t newAnim, int32_t currentTime)
{
    if (newAnim < 0 || newAnim >= JKA::MAX_ANIMATIONS) {
        assert(false);
        return false;
    }

    const auto & animations = JKA::getHumanoidAnimationsInfo();
    anim = newAnim;
    startTime = currentTime;
    endTime = startTime + animations[anim].numFrames * std::abs(animations[anim].frameLerp);
    return true;
}

bool AnimationState::startNewAnimation(int32_t newAnim, int32_t currentTime)
{
    if (newAnim < 0 || newAnim >= JKA::MAX_ANIMATIONS) {
        assert(false);
        return false;
    }

    return startNewAnimation(static_cast<JKA::animNumber_t>(newAnim), currentTime);
}
