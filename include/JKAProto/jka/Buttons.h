#pragma once

namespace JKA {
    constexpr auto BUTTON_ATTACK = 1;
    constexpr auto BUTTON_TALK = 2;                 // displays talk balloon and disables actions
    constexpr auto BUTTON_USE_HOLDABLE = 4;
    constexpr auto BUTTON_GESTURE = 8;
    constexpr auto BUTTON_WALKING = 16;             // walking can't just be infered from MOVE_RUN
    // because a key pressed late in the frame will
    // only generate a small move value for that frame
    // walking will use different animations and
    // won't generate footsteps
    constexpr auto BUTTON_USE = 32;                  // the ol' use key returns!
    constexpr auto BUTTON_FORCEGRIP = 64;
    constexpr auto BUTTON_ALT_ATTACK = 128;

    constexpr auto BUTTON_ANY = 256;                  // any key whatsoever

    constexpr auto BUTTON_FORCEPOWER = 512;           // use the "active" force power
    constexpr auto BUTTON_FORCE_LIGHTNING = 1024;
    constexpr auto BUTTON_FORCE_DRAIN = 2048;
}
