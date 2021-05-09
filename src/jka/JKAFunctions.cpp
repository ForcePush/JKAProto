#include <algorithm>
#include <cstdio>
#include <cstdarg>

#include <JKAProto/jka/JKAFunctions.h>
#include <JKAProto/jka/JKAConstants.h>
#include <JKAProto/jka/JKAStructs.h>
#include <JKAProto/jka/JKAEnums.h>

namespace JKA
{
    int32_t Com_HashKey(std::string_view string, size_t maxLen)
    {
        maxLen = std::min(maxLen, string.size());

        // The upper bounding of this sum (with maxLen = 32) is
        // 255 * (119 + 31) * 32 = 1'224'000, so no signed integer
        // overflow may occur
        int32_t hash = 0;
        for (size_t i = 0; i < maxLen && string[i] != '\0'; i++) {
            hash += string[i] * (119 + static_cast<int32_t>(i));
        }
        hash = (hash ^ (hash >> 10) ^ (hash >> 20));

        return hash;
    }

    void BG_PlayerStateToEntityState(playerState_t & ps, entityState_t & s)
    {
        int        i;

        if (ps.pm_type == PM_INTERMISSION || ps.pm_type == PM_SPECTATOR) {
            s.eType = ET_INVISIBLE;
        } else if (ps.stats[STAT_HEALTH] <= -40) {
            s.eType = ET_INVISIBLE;
        } else {
            s.eType = ET_PLAYER;
        }

        s.number = ps.clientNum;

        s.pos.trType = TR_INTERPOLATE;
        VectorCopy(ps.origin, s.pos.trBase);
        // set the trDelta for flag direction
        VectorCopy(ps.velocity, s.pos.trDelta);

        s.apos.trType = TR_INTERPOLATE;
        VectorCopy(ps.viewangles, s.apos.trBase);

        s.trickedentindex = ps.fd.forceMindtrickTargetIndex;
        s.trickedentindex2 = ps.fd.forceMindtrickTargetIndex2;
        s.trickedentindex3 = ps.fd.forceMindtrickTargetIndex3;
        s.trickedentindex4 = ps.fd.forceMindtrickTargetIndex4;

        s.forceFrame = ps.saberLockFrame;

        s.emplacedOwner = ps.electrifyTime;

        s.speed = ps.speed;

        s.genericenemyindex = ps.genericEnemyIndex;

        s.activeForcePass = ps.activeForcePass;

        s.angles2[YAW] = static_cast<vec_t>(ps.movementDir);
        s.legsAnim = ps.legsAnim;
        s.torsoAnim = ps.torsoAnim;

        s.legsFlip = ps.legsFlip;
        s.torsoFlip = ps.torsoFlip;

        s.clientNum = ps.clientNum;        // ET_PLAYER looks here instead of at number
                                        // so corpses can also reference the proper config
        s.eFlags = ps.eFlags;
        s.eFlags2 = ps.eFlags2;

        s.saberInFlight = ps.saberInFlight;
        s.saberEntityNum = ps.saberEntityNum;
        s.saberMove = ps.saberMove;
        s.forcePowersActive = ps.fd.forcePowersActive;

        if (ps.duelInProgress)
        {
            s.bolt1 = 1;
        } else
        {
            s.bolt1 = 0;
        }

        s.otherEntityNum2 = ps.emplacedIndex;

        s.saberHolstered = ps.saberHolstered;

        if (ps.genericEnemyIndex != -1)
        {
            s.eFlags |= EF_SEEKERDRONE;
        }

        if (ps.stats[STAT_HEALTH] <= 0) {
            s.eFlags |= EF_DEAD;
        } else {
            s.eFlags &= ~EF_DEAD;
        }

        if (ps.externalEvent) {
            s.event = ps.externalEvent;
            s.eventParm = ps.externalEventParm;
        } else if (ps.entityEventSequence < ps.eventSequence) {
            int        seq;

            if (ps.entityEventSequence < ps.eventSequence - static_cast<int32_t>(MAX_PS_EVENTS)) {
                ps.entityEventSequence = ps.eventSequence - static_cast<int32_t>(MAX_PS_EVENTS);
            }
            seq = ps.entityEventSequence & (MAX_PS_EVENTS - 1);
            s.event = ps.events[seq] | ((ps.entityEventSequence & 3) << 8);
            s.eventParm = ps.eventParms[seq];
            ps.entityEventSequence++;
        }


        s.weapon = ps.weapon;
        s.groundEntityNum = ps.groundEntityNum;

        s.powerups = 0;
        for (i = 0; i < MAX_POWERUPS; i++) {
            if (ps.powerups[i]) {
                s.powerups |= 1 << i;
            }
        }

        s.loopSound = ps.loopSound;
        s.generic1 = ps.generic1;

        //NOT INCLUDED IN ENTITYSTATETOPLAYERSTATE:
        s.modelindex2 = ps.weaponstate;
        s.constantLight = ps.weaponChargeTime;

        VectorCopy(ps.lastHitLoc, s.origin2);

        s.isJediMaster = ps.isJediMaster;

        s.time2 = ps.holocronBits;

        s.fireflag = ps.fd.saberAnimLevel;

        s.heldByClient = ps.heldByClient;
        s.ragAttach = ps.ragAttach;

        s.iModelScale = ps.iModelScale;

        s.brokenLimbs = ps.brokenLimbs;

        s.hasLookTarget = ps.hasLookTarget;
        s.lookTarget = ps.lookTarget;

        s.customRGBA[0] = ps.customRGBA[0];
        s.customRGBA[1] = ps.customRGBA[1];
        s.customRGBA[2] = ps.customRGBA[2];
        s.customRGBA[3] = ps.customRGBA[3];

        s.m_iVehicleNum = ps.m_iVehicleNum;
    }
}
