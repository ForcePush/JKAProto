#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "JKAConstants.h"
#include "JKAEnums.h"
#include "JKAStructs.h"

namespace JKA
{
    struct netField_t {
        std::string_view name;
        size_t offset;
        int32_t bits;        // 0 = float
    };

#define NETF(x) #x,offsetof(entityState_t,x)
#define PSF(x) #x,offsetof(playerState_t,x)

    inline constexpr std::array entityStateFields
    {
        netField_t{ NETF(pos.trTime), 32 },
        netField_t{ NETF(pos.trBase[1]), 0 },
        netField_t{ NETF(pos.trBase[0]), 0 },
        netField_t{ NETF(apos.trBase[1]), 0 },
        netField_t{ NETF(pos.trBase[2]), 0 },
        netField_t{ NETF(apos.trBase[0]), 0 },
        netField_t{ NETF(pos.trDelta[0]), 0 },
        netField_t{ NETF(pos.trDelta[1]), 0 },
        netField_t{ NETF(eType), 8 },
        netField_t{ NETF(angles[1]), 0 },
        netField_t{ NETF(pos.trDelta[2]), 0 },
        netField_t{ NETF(origin[0]), 0 },
        netField_t{ NETF(origin[1]), 0 },
        netField_t{ NETF(origin[2]), 0 },
        // does this need to be 8 bits?
        netField_t{ NETF(weapon), 8 },
        netField_t{ NETF(apos.trType), 8 },
        // changed from 12 to 16
        netField_t{ NETF(legsAnim), 16 },            // Maximum number of animation sequences is 2048.  Top bit is reserved for the togglebit
                                        // suspicious
        netField_t{ NETF(torsoAnim), 16 },        // Maximum number of animation sequences is 2048.  Top bit is reserved for the togglebit
                                        // large use beyond GENTITYNUM_BITS - should use generic1 insead
        netField_t{ NETF(genericenemyindex), 32 }, //Do not change to GENTITYNUM_BITS, used as a time offset for seeker
        netField_t{ NETF(eFlags), 32 },
        netField_t{ NETF(pos.trDuration), 32 },
        // might be able to reduce
        netField_t{ NETF(teamowner), 8 },
        netField_t{ NETF(groundEntityNum), GENTITYNUM_BITS },
        netField_t{ NETF(pos.trType), 8 },
        netField_t{ NETF(angles[2]), 0 },
        netField_t{ NETF(angles[0]), 0 },
        netField_t{ NETF(solid), 24 },
        // flag states barely used - could be moved elsewhere
        netField_t{ NETF(fireflag), 2 },
        netField_t{ NETF(event), 10 },            // There is a maximum of 256 events (8 bits transmission, 2 high bits for uniqueness)
                                        // used mostly for players and npcs - appears to be static / never changing
        netField_t{ NETF(customRGBA[3]), 8 }, //0-255
                                    // used mostly for players and npcs - appears to be static / never changing
        netField_t{ NETF(customRGBA[0]), 8 }, //0-255
                                    // only used in fx system (which rick did) and chunks
        netField_t{ NETF(speed), 0 },
        // why are npc's clientnum's that big?
        netField_t{ NETF(clientNum), GENTITYNUM_BITS }, //with npc's clientnum can be > MAX_CLIENTS so use entnum bits now instead.
        netField_t{ NETF(apos.trBase[2]), 0 },
        netField_t{ NETF(apos.trTime), 32 },
        // used mostly for players and npcs - appears to be static / never changing
        netField_t{ NETF(customRGBA[1]), 8 }, //0-255
                                    // used mostly for players and npcs - appears to be static / never changing
        netField_t{ NETF(customRGBA[2]), 8 }, //0-255
                                    // multiple meanings
        netField_t{ NETF(saberEntityNum), GENTITYNUM_BITS },
        // could probably just eliminate and assume a big number
        netField_t{ NETF(g2radius), 8 },
        netField_t{ NETF(otherEntityNum2), GENTITYNUM_BITS },
        // used all over the place
        netField_t{ NETF(owner), GENTITYNUM_BITS },
        netField_t{ NETF(modelindex2), 8 },
        // why was this changed from 0 to 8 ?
        netField_t{ NETF(eventParm), 8 },
        // unknown about size?
        netField_t{ NETF(saberMove), 8 },
        netField_t{ NETF(apos.trDelta[1]), 0 },
        netField_t{ NETF(boneAngles1[1]), 0 },
        // why raised from 8 to -16?
        netField_t{ NETF(modelindex), -16 },
        // barely used, could probably be replaced
        netField_t{ NETF(emplacedOwner), 32 }, //As above, also used as a time value (for electricity render time)
        netField_t{ NETF(apos.trDelta[0]), 0 },
        netField_t{ NETF(apos.trDelta[2]), 0 },
        // shouldn't these be better off as flags?  otherwise, they may consume more bits this way
        netField_t{ NETF(torsoFlip), 1 },
        netField_t{ NETF(angles2[1]), 0 },
        // used mostly in saber and npc
        netField_t{ NETF(lookTarget), GENTITYNUM_BITS },
        netField_t{ NETF(origin2[2]), 0 },
        // randomly used, not sure why this was used instead of svc_noclient
        //    if (cent->currentState.modelGhoul2 == 127)
        //    { //not ready to be drawn or initialized..
        //        return;
        //    }
        netField_t{ NETF(modelGhoul2), 8 },
        netField_t{ NETF(loopSound), 8 },
        netField_t{ NETF(origin2[0]), 0 },
        // multiple purpose bit flag
        netField_t{ NETF(shouldtarget), 1 },
        // widely used, does not appear that they have to be 16 bits
        netField_t{ NETF(trickedentindex), 16 }, //See note in PSF
        netField_t{ NETF(otherEntityNum), GENTITYNUM_BITS },
        netField_t{ NETF(origin2[1]), 0 },
        netField_t{ NETF(time2), 32 },
        netField_t{ NETF(legsFlip), 1 },
        // fully used
        netField_t{ NETF(bolt2), GENTITYNUM_BITS },
        netField_t{ NETF(constantLight), 32 },
        netField_t{ NETF(time), 32 },
        // why doesn't lookTarget just indicate this?
        netField_t{ NETF(hasLookTarget), 1 },
        netField_t{ NETF(boneAngles1[2]), 0 },
        // used for both force pass and an emplaced gun - gun is just a flag indicator
        netField_t{ NETF(activeForcePass), 6 },
        // used to indicate health
        netField_t{ NETF(health), 10 }, //if something's health exceeds 1024, then.. too bad!
                              // appears to have multiple means, could be eliminated by indicating a sound set differently
        netField_t{ NETF(loopIsSoundset), 1 },
        netField_t{ NETF(saberHolstered), 2 },
        //NPC-SPECIFIC:
        // both are used for NPCs sabers, though limited
        netField_t{ NETF(npcSaber1), 9 },
        netField_t{ NETF(maxhealth), 10 },
        netField_t{ NETF(trickedentindex2), 16 },
        // appear to only be 18 powers?
        netField_t{ NETF(forcePowersActive), 32 },
        // used, doesn't appear to be flexible
        netField_t{ NETF(iModelScale), 10 }, //0-1024 (guess it's gotta be increased if we want larger allowable scale.. but 1024% is pretty big)
                                   // full bits used
        netField_t{ NETF(powerups), 16 },
        // can this be reduced?
        netField_t{ NETF(soundSetIndex), 8 }, //rww - if MAX_AMBIENT_SETS is changed from 256, REMEMBER TO CHANGE THIS
                                    // looks like this can be reduced to 4? (ship parts = 4, people parts = 2)
        netField_t{ NETF(brokenLimbs), 8 }, //up to 8 limbs at once (not that that many are used)
        netField_t{ NETF(csSounds_Std), 8 }, //soundindex must be 8 unless max sounds is changed
                                   // used extensively
        netField_t{ NETF(saberInFlight), 1 },
        netField_t{ NETF(angles2[0]), 0 },
        netField_t{ NETF(frame), 16 },
        netField_t{ NETF(angles2[2]), 0 },
        // why not use torsoAnim and set a flag to do the same thing as forceFrame (saberLockFrame)
        netField_t{ NETF(forceFrame), 16 }, //if you have over 65536 frames, then this will explode. Of course if you have that many things then lots of things will probably explode.
        netField_t{ NETF(generic1), 8 },
        // do we really need 4 indexes?
        netField_t{ NETF(boneIndex1), 6 }, //up to 64 bones can be accessed by this indexing method
                                 // only 54 classes, could cut down 2 bits
        netField_t{ NETF(NPC_class), 8 },
        netField_t{ NETF(apos.trDuration), 32 },
        // there appears to be only 2 different version of parms passed - a flag would better be suited
        netField_t{ NETF(boneOrient), 9 }, //3 bits per orientation dir
                                 // this looks to be a single bit flag
        netField_t{ NETF(bolt1), 8 },
        netField_t{ NETF(trickedentindex3), 16 },
        // in use for vehicles
        netField_t{ NETF(m_iVehicleNum), GENTITYNUM_BITS }, // 10 bits fits all possible entity nums (2^10 = 1024). - AReis
        netField_t{ NETF(trickedentindex4), 16 },
        // but why is there an opposite state of surfaces field?
        netField_t{ NETF(surfacesOff), 32 },
        netField_t{ NETF(eFlags2), 10 },
        // should be bit field
        netField_t{ NETF(isJediMaster), 1 },
        // should be bit field
        netField_t{ NETF(isPortalEnt), 1 },
        // possible multiple definitions
        netField_t{ NETF(heldByClient), 6 },
        // this does not appear to be used in any production or non-cheat fashion - REMOVE
        netField_t{ NETF(ragAttach), GENTITYNUM_BITS },
        // used only in one spot for seige
        netField_t{ NETF(boltToPlayer), 6 },
        netField_t{ NETF(npcSaber2), 9 },
        netField_t{ NETF(csSounds_Combat), 8 },
        netField_t{ NETF(csSounds_Extra), 8 },
        netField_t{ NETF(csSounds_Jedi), 8 },
        // used only for surfaces on NPCs
        netField_t{ NETF(surfacesOn), 32 }, //allow up to 32 surfaces in the bitflag
        netField_t{ NETF(boneIndex2), 6 },
        netField_t{ NETF(boneIndex3), 6 },
        netField_t{ NETF(boneIndex4), 6 },
        netField_t{ NETF(boneAngles1[0]), 0 },
        netField_t{ NETF(boneAngles2[0]), 0 },
        netField_t{ NETF(boneAngles2[1]), 0 },
        netField_t{ NETF(boneAngles2[2]), 0 },
        netField_t{ NETF(boneAngles3[0]), 0 },
        netField_t{ NETF(boneAngles3[1]), 0 },
        netField_t{ NETF(boneAngles3[2]), 0 },
        netField_t{ NETF(boneAngles4[0]), 0 },
        netField_t{ NETF(boneAngles4[1]), 0 },
        netField_t{ NETF(boneAngles4[2]), 0 },

        netField_t{ NETF(userInt1), 1 },
        netField_t{ NETF(userInt2), 1 },
        netField_t{ NETF(userInt3), 1 },
        netField_t{ NETF(userFloat1), 1 },
        netField_t{ NETF(userFloat2), 1 },
        netField_t{ NETF(userFloat3), 1 },
        netField_t{ NETF(userVec1[0]), 1 },
        netField_t{ NETF(userVec1[1]), 1 },
        netField_t{ NETF(userVec1[2]), 1 },
        netField_t{ NETF(userVec2[0]), 1 },
        netField_t{ NETF(userVec2[1]), 1 },
        netField_t{ NETF(userVec2[2]), 1 }
    };

    inline constexpr std::array playerStateFields
    {
        netField_t{ PSF(commandTime), 32 },
        netField_t{ PSF(origin[1]), 0 },
        netField_t{ PSF(origin[0]), 0 },
        netField_t{ PSF(viewangles[1]), 0 },
        netField_t{ PSF(viewangles[0]), 0 },
        netField_t{ PSF(origin[2]), 0 },
        netField_t{ PSF(velocity[0]), 0 },
        netField_t{ PSF(velocity[1]), 0 },
        netField_t{ PSF(velocity[2]), 0 },
        netField_t{ PSF(bobCycle), 8 },
        netField_t{ PSF(weaponTime), -16 },
        netField_t{ PSF(delta_angles[1]), 16 },
        netField_t{ PSF(speed), 0 }, //sadly, the vehicles require negative speed values, so..
        netField_t{ PSF(legsAnim), 16 },            // Maximum number of animation sequences is 2048.  Top bit is reserved for the togglebit
        netField_t{ PSF(delta_angles[0]), 16 },
        netField_t{ PSF(torsoAnim), 16 },            // Maximum number of animation sequences is 2048.  Top bit is reserved for the togglebit
        netField_t{ PSF(groundEntityNum), GENTITYNUM_BITS },
        netField_t{ PSF(eFlags), 32 },
        netField_t{ PSF(fd.forcePower), 8 },
        netField_t{ PSF(eventSequence), 16 },
        netField_t{ PSF(torsoTimer), 16 },
        netField_t{ PSF(legsTimer), 16 },
        netField_t{ PSF(viewheight), -8 },
        netField_t{ PSF(fd.saberAnimLevel), 4 },
        netField_t{ PSF(rocketLockIndex), GENTITYNUM_BITS },
        netField_t{ PSF(fd.saberDrawAnimLevel), 4 },
        netField_t{ PSF(genericEnemyIndex), 32 }, //NOTE: This isn't just an index all the time, it's often used as a time value, and thus needs 32 bits
        netField_t{ PSF(events[0]), 10 },            // There is a maximum of 256 events (8 bits transmission, 2 high bits for uniqueness)
        netField_t{ PSF(events[1]), 10 },            // There is a maximum of 256 events (8 bits transmission, 2 high bits for uniqueness)
        netField_t{ PSF(customRGBA[0]), 8 }, //0-255
        netField_t{ PSF(movementDir), 4 },
        netField_t{ PSF(saberEntityNum), GENTITYNUM_BITS }, //Also used for channel tracker storage, but should never exceed entity number
        netField_t{ PSF(customRGBA[3]), 8 }, //0-255
        netField_t{ PSF(weaponstate), 4 },
        netField_t{ PSF(saberMove), 32 }, //This value sometimes exceeds the max LS_ value and gets set to a crazy amount, so it needs 32 bits
        netField_t{ PSF(standheight), 10 },
        netField_t{ PSF(crouchheight), 10 },
        netField_t{ PSF(basespeed), -16 },
        netField_t{ PSF(pm_flags), 16 },
        netField_t{ PSF(jetpackFuel), 8 },
        netField_t{ PSF(cloakFuel), 8 },
        netField_t{ PSF(pm_time), -16 },
        netField_t{ PSF(customRGBA[1]), 8 }, //0-255
        netField_t{ PSF(clientNum), GENTITYNUM_BITS },
        netField_t{ PSF(duelIndex), GENTITYNUM_BITS },
        netField_t{ PSF(customRGBA[2]), 8 }, //0-255
        netField_t{ PSF(gravity), 16 },
        netField_t{ PSF(weapon), 8 },
        netField_t{ PSF(delta_angles[2]), 16 },
        netField_t{ PSF(saberCanThrow), 1 },
        netField_t{ PSF(viewangles[2]), 0 },
        netField_t{ PSF(fd.forcePowersKnown), 32 },
        netField_t{ PSF(fd.forcePowerLevel[FP_LEVITATION]), 2 }, //unfortunately we need this for fall damage calculation (client needs to know the distance for the fall noise)
        netField_t{ PSF(fd.forcePowerDebounce[FP_LEVITATION]), 32 },
        netField_t{ PSF(fd.forcePowerSelected), 8 },
        netField_t{ PSF(torsoFlip), 1 },
        netField_t{ PSF(externalEvent), 10 },
        netField_t{ PSF(damageYaw), 8 },
        netField_t{ PSF(damageCount), 8 },
        netField_t{ PSF(inAirAnim), 1 }, //just transmit it for the sake of knowing right when on the client to play a land anim, it's only 1 bit
        netField_t{ PSF(eventParms[1]), 8 },
        netField_t{ PSF(fd.forceSide), 2 }, //so we know if we should apply greyed out shaders to dark/light force enlightenment
        netField_t{ PSF(saberAttackChainCount), 4 },
        netField_t{ PSF(pm_type), 8 },
        netField_t{ PSF(externalEventParm), 8 },
        netField_t{ PSF(eventParms[0]), -16 },
        netField_t{ PSF(lookTarget), GENTITYNUM_BITS },
        //netField_t{ PSF(vehOrientation[0]), 0 },
        netField_t{ PSF(weaponChargeSubtractTime), 32 }, //? really need 32 bits??
                                               //netField_t{ PSF(vehOrientation[1]), 0 },
                                               //netField_t{ PSF(moveDir[1]), 0 },
                                               //netField_t{ PSF(moveDir[0]), 0 },
        netField_t{ PSF(weaponChargeTime), 32 }, //? really need 32 bits??
                                       //netField_t{ PSF(vehOrientation[2]), 0 },
        netField_t{ PSF(legsFlip), 1 },
        netField_t{ PSF(damageEvent), 8 },
        netField_t{ PSF(rocketTargetTime), 32 },
        netField_t{ PSF(activeForcePass), 6 },
        netField_t{ PSF(electrifyTime), 32 },
        netField_t{ PSF(fd.forceJumpZStart), 0 },
        netField_t{ PSF(loopSound), 16 }, //rwwFIXMEFIXME: max sounds is 256, doesn't this only need to be 8?
        netField_t{ PSF(hasLookTarget), 1 },
        netField_t{ PSF(saberBlocked), 8 },
        netField_t{ PSF(damageType), 2 },
        netField_t{ PSF(rocketLockTime), 32 },
        netField_t{ PSF(forceHandExtend), 8 },
        netField_t{ PSF(saberHolstered), 2 },
        netField_t{ PSF(fd.forcePowersActive), 32 },
        netField_t{ PSF(damagePitch), 8 },
        netField_t{ PSF(m_iVehicleNum), GENTITYNUM_BITS }, // 10 bits fits all possible entity nums (2^10 = 1024). - AReis
                                                 //netField_t{ PSF(vehTurnaroundTime), 32 },//only used by vehicle?
        netField_t{ PSF(generic1), 8 },
        netField_t{ PSF(jumppad_ent), 10 },
        netField_t{ PSF(hasDetPackPlanted), 1 },
        netField_t{ PSF(saberInFlight), 1 },
        netField_t{ PSF(forceDodgeAnim), 16 },
        netField_t{ PSF(zoomMode), 2 }, // NOTENOTE Are all of these necessary?
        netField_t{ PSF(hackingTime), 32 },
        netField_t{ PSF(zoomTime), 32 },    // NOTENOTE Are all of these necessary?
        netField_t{ PSF(brokenLimbs), 8 }, //up to 8 limbs at once (not that that many are used)
        netField_t{ PSF(zoomLocked), 1 },    // NOTENOTE Are all of these necessary?
        netField_t{ PSF(zoomFov), 0 },    // NOTENOTE Are all of these necessary?
        netField_t{ PSF(fd.forceRageRecoveryTime), 32 },
        netField_t{ PSF(fallingToDeath), 32 },
        netField_t{ PSF(fd.forceMindtrickTargetIndex), 16 }, //NOTE: Not just an index, used as a (1 << val) bitflag for up to 16 clients
        netField_t{ PSF(fd.forceMindtrickTargetIndex2), 16 }, //NOTE: Not just an index, used as a (1 << val) bitflag for up to 16 clients
                                                    //netField_t{ PSF(vehWeaponsLinked), 1 },//only used by vehicle?
        netField_t{ PSF(lastHitLoc[2]), 0 },
        //netField_t{ PSF(hyperSpaceTime), 32 },//only used by vehicle?
        netField_t{ PSF(fd.forceMindtrickTargetIndex3), 16 }, //NOTE: Not just an index, used as a (1 << val) bitflag for up to 16 clients
        netField_t{ PSF(lastHitLoc[0]), 0 },
        netField_t{ PSF(eFlags2), 10 },
        netField_t{ PSF(fd.forceMindtrickTargetIndex4), 16 }, //NOTE: Not just an index, used as a (1 << val) bitflag for up to 16 clients
                                                    //netField_t{ PSF(hyperSpaceAngles[1]), 0 },//only used by vehicle?
        netField_t{ PSF(lastHitLoc[1]), 0 }, //currently only used so client knows to orient disruptor disintegration.. seems a bit much for just that though.
                                   //netField_t{ PSF(vehBoarding), 1 }, //only used by vehicle? not like the normal boarding value, this is a simple "1 or 0" value
        netField_t{ PSF(fd.sentryDeployed), 1 },
        netField_t{ PSF(saberLockTime), 32 },
        netField_t{ PSF(saberLockFrame), 16 },
        //netField_t{ PSF(vehTurnaroundIndex), GENTITYNUM_BITS },//only used by vehicle?
        //netField_t{ PSF(vehSurfaces), 16 }, //only used by vehicle? allow up to 16 surfaces in the flag I guess
        netField_t{ PSF(fd.forcePowerLevel[FP_SEE]), 2 }, //needed for knowing when to display players through walls
        netField_t{ PSF(saberLockEnemy), GENTITYNUM_BITS },
        netField_t{ PSF(fd.forceGripCripple), 1 }, //should only be 0 or 1 ever
        netField_t{ PSF(emplacedIndex), GENTITYNUM_BITS },
        netField_t{ PSF(holocronBits), 32 },
        netField_t{ PSF(isJediMaster), 1 },
        netField_t{ PSF(forceRestricted), 1 },
        netField_t{ PSF(trueJedi), 1 },
        netField_t{ PSF(trueNonJedi), 1 },
        netField_t{ PSF(duelTime), 32 },
        netField_t{ PSF(duelInProgress), 1 },
        netField_t{ PSF(saberLockAdvance), 1 },
        netField_t{ PSF(heldByClient), 6 },
        netField_t{ PSF(ragAttach), GENTITYNUM_BITS },
        netField_t{ PSF(iModelScale), 10 }, //0-1024 (guess it's gotta be increased if we want larger allowable scale.. but 1024% is pretty big)
        netField_t{ PSF(hackingBaseTime), 16 },
        netField_t{ PSF(userInt1), 1 },
        netField_t{ PSF(userInt2), 1 },
        netField_t{ PSF(userInt3), 1 },
        netField_t{ PSF(userFloat1), 1 },
        netField_t{ PSF(userFloat2), 1 },
        netField_t{ PSF(userFloat3), 1 },
        netField_t{ PSF(userVec1[0]), 1 },
        netField_t{ PSF(userVec1[1]), 1 },
        netField_t{ PSF(userVec1[2]), 1 },
        netField_t{ PSF(userVec2[0]), 1 },
        netField_t{ PSF(userVec2[1]), 1 },
        netField_t{ PSF(userVec2[2]), 1 }
    };

    inline constexpr std::array pilotPlayerStateFields
    {
        netField_t{ PSF(commandTime), 32 },
        netField_t{ PSF(origin[1]), 0 },
        netField_t{ PSF(origin[0]), 0 },
        netField_t{ PSF(viewangles[1]), 0 },
        netField_t{ PSF(viewangles[0]), 0 },
        netField_t{ PSF(origin[2]), 0 },
        netField_t{ PSF(weaponTime), -16 },
        netField_t{ PSF(delta_angles[1]), 16 },
        netField_t{ PSF(delta_angles[0]), 16 },
        netField_t{ PSF(eFlags), 32 },
        netField_t{ PSF(eventSequence), 16 },
        netField_t{ PSF(rocketLockIndex), GENTITYNUM_BITS },
        netField_t{ PSF(events[0]), 10 },            // There is a maximum of 256 events (8 bits transmission, 2 high bits for uniqueness)
        netField_t{ PSF(events[1]), 10 },            // There is a maximum of 256 events (8 bits transmission, 2 high bits for uniqueness)
        netField_t{ PSF(weaponstate), 4 },
        netField_t{ PSF(pm_flags), 16 },
        netField_t{ PSF(pm_time), -16 },
        netField_t{ PSF(clientNum), GENTITYNUM_BITS },
        netField_t{ PSF(weapon), 8 },
        netField_t{ PSF(delta_angles[2]), 16 },
        netField_t{ PSF(viewangles[2]), 0 },
        netField_t{ PSF(externalEvent), 10 },
        netField_t{ PSF(eventParms[1]), 8 },
        netField_t{ PSF(pm_type), 8 },
        netField_t{ PSF(externalEventParm), 8 },
        netField_t{ PSF(eventParms[0]), -16 },
        netField_t{ PSF(weaponChargeSubtractTime), 32 }, //? really need 32 bits??
        netField_t{ PSF(weaponChargeTime), 32 }, //? really need 32 bits??
        netField_t{ PSF(rocketTargetTime), 32 },
        netField_t{ PSF(fd.forceJumpZStart), 0 },
        netField_t{ PSF(rocketLockTime), 32 },
        netField_t{ PSF(m_iVehicleNum), GENTITYNUM_BITS }, // 10 bits fits all possible entity nums (2^10 = 1024). - AReis
        netField_t{ PSF(generic1), 8 },//used by passengers
        netField_t{ PSF(eFlags2), 10 },

        //===THESE SHOULD NOT BE CHANGING OFTEN====================================================================
        netField_t{ PSF(legsAnim), 16 },            // Maximum number of animation sequences is 2048.  Top bit is reserved for the togglebit
        netField_t{ PSF(torsoAnim), 16 },            // Maximum number of animation sequences is 2048.  Top bit is reserved for the togglebit
        netField_t{ PSF(torsoTimer), 16 },
        netField_t{ PSF(legsTimer), 16 },
        netField_t{ PSF(jetpackFuel), 8 },
        netField_t{ PSF(cloakFuel), 8 },
        netField_t{ PSF(saberCanThrow), 1 },
        netField_t{ PSF(fd.forcePowerDebounce[FP_LEVITATION]), 32 },
        netField_t{ PSF(torsoFlip), 1 },
        netField_t{ PSF(legsFlip), 1 },
        netField_t{ PSF(fd.forcePowersActive), 32 },
        netField_t{ PSF(hasDetPackPlanted), 1 },
        netField_t{ PSF(fd.forceRageRecoveryTime), 32 },
        netField_t{ PSF(saberInFlight), 1 },
        netField_t{ PSF(fd.forceMindtrickTargetIndex), 16 }, //NOTE: Not just an index, used as a (1 << val) bitflag for up to 16 clients
        netField_t{ PSF(fd.forceMindtrickTargetIndex2), 16 }, //NOTE: Not just an index, used as a (1 << val) bitflag for up to 16 clients
        netField_t{ PSF(fd.forceMindtrickTargetIndex3), 16 }, //NOTE: Not just an index, used as a (1 << val) bitflag for up to 16 clients
        netField_t{ PSF(fd.forceMindtrickTargetIndex4), 16 }, //NOTE: Not just an index, used as a (1 << val) bitflag for up to 16 clients
        netField_t{ PSF(fd.sentryDeployed), 1 },
        netField_t{ PSF(fd.forcePowerLevel[FP_SEE]), 2 }, //needed for knowing when to display players through walls
        netField_t{ PSF(holocronBits), 32 },
        netField_t{ PSF(fd.forcePower), 8 },

        //===THE REST OF THESE SHOULD NOT BE RELEVANT, BUT, FOR SAFETY, INCLUDE THEM ANYWAY, JUST AT THE BOTTOM===============================================================
        netField_t{ PSF(velocity[0]), 0 },
        netField_t{ PSF(velocity[1]), 0 },
        netField_t{ PSF(velocity[2]), 0 },
        netField_t{ PSF(bobCycle), 8 },
        netField_t{ PSF(speed), 0 }, //sadly, the vehicles require negative speed values, so..
        netField_t{ PSF(groundEntityNum), GENTITYNUM_BITS },
        netField_t{ PSF(viewheight), -8 },
        netField_t{ PSF(fd.saberAnimLevel), 4 },
        netField_t{ PSF(fd.saberDrawAnimLevel), 4 },
        netField_t{ PSF(genericEnemyIndex), 32 }, //NOTE: This isn't just an index all the time, it's often used as a time value, and thus needs 32 bits
        netField_t{ PSF(customRGBA[0]), 8 }, //0-255
        netField_t{ PSF(movementDir), 4 },
        netField_t{ PSF(saberEntityNum), GENTITYNUM_BITS }, //Also used for channel tracker storage, but should never exceed entity number
        netField_t{ PSF(customRGBA[3]), 8 }, //0-255
        netField_t{ PSF(saberMove), 32 }, //This value sometimes exceeds the max LS_ value and gets set to a crazy amount, so it needs 32 bits
        netField_t{ PSF(standheight), 10 },
        netField_t{ PSF(crouchheight), 10 },
        netField_t{ PSF(basespeed), -16 },
        netField_t{ PSF(customRGBA[1]), 8 }, //0-255
        netField_t{ PSF(duelIndex), GENTITYNUM_BITS },
        netField_t{ PSF(customRGBA[2]), 8 }, //0-255
        netField_t{ PSF(gravity), 16 },
        netField_t{ PSF(fd.forcePowersKnown), 32 },
        netField_t{ PSF(fd.forcePowerLevel[FP_LEVITATION]), 2 }, //unfortunately we need this for fall damage calculation (client needs to know the distance for the fall noise)
        netField_t{ PSF(fd.forcePowerSelected), 8 },
        netField_t{ PSF(damageYaw), 8 },
        netField_t{ PSF(damageCount), 8 },
        netField_t{ PSF(inAirAnim), 1 }, //just transmit it for the sake of knowing right when on the client to play a land anim, it's only 1 bit
        netField_t{ PSF(fd.forceSide), 2 }, //so we know if we should apply greyed out shaders to dark/light force enlightenment
        netField_t{ PSF(saberAttackChainCount), 4 },
        netField_t{ PSF(lookTarget), GENTITYNUM_BITS },
        netField_t{ PSF(moveDir[1]), 0 },
        netField_t{ PSF(moveDir[0]), 0 },
        netField_t{ PSF(damageEvent), 8 },
        netField_t{ PSF(moveDir[2]), 0 },
        netField_t{ PSF(activeForcePass), 6 },
        netField_t{ PSF(electrifyTime), 32 },
        netField_t{ PSF(damageType), 2 },
        netField_t{ PSF(loopSound), 16 }, //rwwFIXMEFIXME: max sounds is 256, doesn't this only need to be 8?
        netField_t{ PSF(hasLookTarget), 1 },
        netField_t{ PSF(saberBlocked), 8 },
        netField_t{ PSF(forceHandExtend), 8 },
        netField_t{ PSF(saberHolstered), 2 },
        netField_t{ PSF(damagePitch), 8 },
        netField_t{ PSF(jumppad_ent), 10 },
        netField_t{ PSF(forceDodgeAnim), 16 },
        netField_t{ PSF(zoomMode), 2 }, // NOTENOTE Are all of these necessary?
        netField_t{ PSF(hackingTime), 32 },
        netField_t{ PSF(zoomTime), 32 },    // NOTENOTE Are all of these necessary?
        netField_t{ PSF(brokenLimbs), 8 }, //up to 8 limbs at once (not that that many are used)
        netField_t{ PSF(zoomLocked), 1 },    // NOTENOTE Are all of these necessary?
        netField_t{ PSF(zoomFov), 0 },    // NOTENOTE Are all of these necessary?
        netField_t{ PSF(fallingToDeath), 32 },
        netField_t{ PSF(lastHitLoc[2]), 0 },
        netField_t{ PSF(lastHitLoc[0]), 0 },
        netField_t{ PSF(lastHitLoc[1]), 0 }, //currently only used so client knows to orient disruptor disintegration.. seems a bit much for just that though.
        netField_t{ PSF(saberLockTime), 32 },
        netField_t{ PSF(saberLockFrame), 16 },
        netField_t{ PSF(saberLockEnemy), GENTITYNUM_BITS },
        netField_t{ PSF(fd.forceGripCripple), 1 }, //should only be 0 or 1 ever
        netField_t{ PSF(emplacedIndex), GENTITYNUM_BITS },
        netField_t{ PSF(isJediMaster), 1 },
        netField_t{ PSF(forceRestricted), 1 },
        netField_t{ PSF(trueJedi), 1 },
        netField_t{ PSF(trueNonJedi), 1 },
        netField_t{ PSF(duelTime), 32 },
        netField_t{ PSF(duelInProgress), 1 },
        netField_t{ PSF(saberLockAdvance), 1 },
        netField_t{ PSF(heldByClient), 6 },
        netField_t{ PSF(ragAttach), GENTITYNUM_BITS },
        netField_t{ PSF(iModelScale), 10 }, //0-1024 (guess it's gotta be increased if we want larger allowable scale.. but 1024% is pretty big)
        netField_t{ PSF(hackingBaseTime), 16 },
        netField_t{ PSF(userInt1), 1 },
        netField_t{ PSF(userInt2), 1 },
        netField_t{ PSF(userInt3), 1 },
        netField_t{ PSF(userFloat1), 1 },
        netField_t{ PSF(userFloat2), 1 },
        netField_t{ PSF(userFloat3), 1 },
        netField_t{ PSF(userVec1[0]), 1 },
        netField_t{ PSF(userVec1[1]), 1 },
        netField_t{ PSF(userVec1[2]), 1 },
        netField_t{ PSF(userVec2[0]), 1 },
        netField_t{ PSF(userVec2[1]), 1 },
        netField_t{ PSF(userVec2[2]), 1 }
    };

    inline constexpr std::array vehPlayerStateFields
    {
        netField_t{ PSF(commandTime), 32 },
        netField_t{ PSF(origin[1]), 0 },
        netField_t{ PSF(origin[0]), 0 },
        netField_t{ PSF(viewangles[1]), 0 },
        netField_t{ PSF(viewangles[0]), 0 },
        netField_t{ PSF(origin[2]), 0 },
        netField_t{ PSF(velocity[0]), 0 },
        netField_t{ PSF(velocity[1]), 0 },
        netField_t{ PSF(velocity[2]), 0 },
        netField_t{ PSF(weaponTime), -16 },
        netField_t{ PSF(delta_angles[1]), 16 },
        netField_t{ PSF(speed), 0 }, //sadly, the vehicles require negative speed values, so..
        netField_t{ PSF(legsAnim), 16 },            // Maximum number of animation sequences is 2048.  Top bit is reserved for the togglebit
        netField_t{ PSF(delta_angles[0]), 16 },
        netField_t{ PSF(groundEntityNum), GENTITYNUM_BITS },
        netField_t{ PSF(eFlags), 32 },
        netField_t{ PSF(eventSequence), 16 },
        netField_t{ PSF(legsTimer), 16 },
        netField_t{ PSF(rocketLockIndex), GENTITYNUM_BITS },
        netField_t{ PSF(events[0]), 10 },            // There is a maximum of 256 events (8 bits transmission, 2 high bits for uniqueness)
        netField_t{ PSF(events[1]), 10 },
        netField_t{ PSF(weaponstate), 4 },
        netField_t{ PSF(pm_flags), 16 },
        netField_t{ PSF(pm_time), -16 },
        netField_t{ PSF(clientNum), GENTITYNUM_BITS },
        netField_t{ PSF(gravity), 16 },
        netField_t{ PSF(weapon), 8 },
        netField_t{ PSF(delta_angles[2]), 16 },
        netField_t{ PSF(viewangles[2]), 0 },
        netField_t{ PSF(externalEvent), 10 },
        netField_t{ PSF(eventParms[1]), 8 },
        netField_t{ PSF(pm_type), 8 },
        netField_t{ PSF(externalEventParm), 8 },
        netField_t{ PSF(eventParms[0]), -16 },
        netField_t{ PSF(vehOrientation[0]), 0 },
        netField_t{ PSF(vehOrientation[1]), 0 },
        netField_t{ PSF(moveDir[1]), 0 },
        netField_t{ PSF(moveDir[0]), 0 },
        netField_t{ PSF(vehOrientation[2]), 0 },
        netField_t{ PSF(moveDir[2]), 0 },
        netField_t{ PSF(rocketTargetTime), 32 },
        netField_t{ PSF(electrifyTime), 32 },
        netField_t{ PSF(loopSound), 16 }, //rwwFIXMEFIXME: max sounds is 256, doesn't this only need to be 8?
        netField_t{ PSF(rocketLockTime), 32 },
        netField_t{ PSF(m_iVehicleNum), GENTITYNUM_BITS }, // 10 bits fits all possible entity nums (2^10 = 1024). - AReis
        netField_t{ PSF(vehTurnaroundTime), 32 },
        netField_t{ PSF(hackingTime), 32 },
        netField_t{ PSF(brokenLimbs), 8 }, //up to 8 limbs at once (not that that many are used)
        netField_t{ PSF(vehWeaponsLinked), 1 },
        netField_t{ PSF(hyperSpaceTime), 32 },
        netField_t{ PSF(eFlags2), 10 },
        netField_t{ PSF(hyperSpaceAngles[1]), 0 },
        netField_t{ PSF(vehBoarding), 1 }, //not like the normal boarding value, this is a simple "1 or 0" value
        netField_t{ PSF(vehTurnaroundIndex), GENTITYNUM_BITS },
        netField_t{ PSF(vehSurfaces), 16 }, //allow up to 16 surfaces in the flag I guess
        netField_t{ PSF(hyperSpaceAngles[0]), 0 },
        netField_t{ PSF(hyperSpaceAngles[2]), 0 },

        netField_t{ PSF(userInt1), 1 },
        netField_t{ PSF(userInt2), 1 },
        netField_t{ PSF(userInt3), 1 },
        netField_t{ PSF(userFloat1), 1 },
        netField_t{ PSF(userFloat2), 1 },
        netField_t{ PSF(userFloat3), 1 },
        netField_t{ PSF(userVec1[0]), 1 },
        netField_t{ PSF(userVec1[1]), 1 },
        netField_t{ PSF(userVec1[2]), 1 },
        netField_t{ PSF(userVec2[0]), 1 },
        netField_t{ PSF(userVec2[1]), 1 },
        netField_t{ PSF(userVec2[2]), 1 }
    };

#undef NETF
#undef PSF
}
