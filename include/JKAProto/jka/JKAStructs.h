#pragma once
#include <cinttypes>
#include <cstring>
#include <type_traits>

#include "JKAConstants.h"
#include "JKAEnums.h"

namespace JKA
{
    typedef int32_t qboolean;

    typedef float vec_t;
    typedef vec_t vec2_t[2];
    typedef vec_t vec3_t[3];
    typedef vec_t vec4_t[4];
    typedef vec_t vec5_t[5];

    // ********************************* netchan_t *********************************
    typedef struct {
        uint16_t qport;

        int32_t            dropped; // between last packet and previous
        int32_t            incomingSequence;
        int32_t            outgoingSequence;

        // incoming fragment assembly buffer
        int32_t            fragmentSequence;
        int32_t            fragmentLength;
        uint8_t            fragmentBuffer[MAX_MSGLEN];

        // outgoing fragment buffer
        // we need to space out the sending of large fragmented messages
        bool            unsentFragments;
        int32_t            unsentFragmentStart;
        int32_t            unsentLength;
        uint8_t            unsentBuffer[MAX_MSGLEN];
    } netchan_t;

    // ********************************* forcedata_t *********************************
    typedef struct forcedata_s {
        int32_t         forcePowerDebounce[NUM_FORCE_POWERS];    //for effects that must have an interval
        int32_t         forcePowersKnown;
        int32_t         forcePowersActive;
        int32_t         forcePowerSelected;
        int32_t         forceButtonNeedRelease;
        int32_t         forcePowerDuration[NUM_FORCE_POWERS];
        int32_t         forcePower;
        int32_t         forcePowerMax;
        int32_t         forcePowerRegenDebounceTime;
        int32_t         forcePowerLevel[NUM_FORCE_POWERS];        //so we know the max forceJump power you have
        int32_t         forcePowerBaseLevel[NUM_FORCE_POWERS];
        int32_t         forceUsingAdded;
        float        forceJumpZStart;                    //So when you land, you don't get hurt as much
        float        forceJumpCharge;                    //you're current forceJump charge-up level, increases the longer you hold the force jump button down
        int32_t         forceJumpSound;
        int32_t         forceJumpAddTime;
        int32_t         forceGripEntityNum;                    //what entity I'm gripping
        int32_t         forceGripDamageDebounceTime;        //debounce for grip damage
        float        forceGripBeingGripped;                //if > level.time then client is in someone's grip
        int32_t         forceGripCripple;                    //if != 0 then make it so this client can't move quickly (he's being gripped)
        int32_t         forceGripUseTime;                    //can't use if > level.time
        float        forceGripSoundTime;
        float        forceGripStarted;                    //level.time when the grip was activated
        int32_t         forceHealTime;
        int32_t         forceHealAmount;

        //This hurts me somewhat to do, but there's no other real way to allow completely "dynamic" mindtricking.
        int32_t         forceMindtrickTargetIndex; //0-15
        int32_t         forceMindtrickTargetIndex2; //16-32
        int32_t         forceMindtrickTargetIndex3; //33-48
        int32_t         forceMindtrickTargetIndex4; //49-64

        int32_t         forceRageRecoveryTime;
        int32_t         forceDrainEntNum;
        float        forceDrainTime;

        int32_t         forceDoInit;

        int32_t         forceSide;
        int32_t         forceRank;

        int32_t         forceDeactivateAll;

        int32_t         killSoundEntIndex[TRACK_CHANNEL_MAX]; //this goes here so it doesn't get wiped over respawn

        qboolean    sentryDeployed;

        int32_t         saberAnimLevelBase;//sigh...
        int32_t         saberAnimLevel;
        int32_t         saberDrawAnimLevel;

        int32_t         suicides;

        int32_t         privateDuelTime;
    } forcedata_t;

    // ********************************* playerState_s *********************************
    typedef struct playerState_s {
        int32_t         commandTime;    // cmd->serverTime of last executed command
        int32_t         pm_type;
        int32_t         bobCycle;        // for view bobbing and footstep generation
        int32_t         pm_flags;        // ducked, jump_held, etc
        int32_t         pm_time;

        vec3_t        origin;
        vec3_t        velocity;

        vec3_t        moveDir; //NOT sent over the net - nor should it be.

        int32_t         weaponTime;
        int32_t         weaponChargeTime;
        int32_t         weaponChargeSubtractTime;
        int32_t         gravity;
        float        speed;
        int32_t         basespeed; //used in prediction to know base server g_speed value when modifying speed between updates
        int32_t         delta_angles[3];    // add to command angles to get view direction
                                            // changed by spawns, rotating objects, and teleporters

        int32_t         slopeRecalcTime; //this is NOT sent across the net and is maintained seperately on game and cgame in pmove code.

        int32_t         useTime;

        int32_t         groundEntityNum;// ENTITYNUM_NONE = in air

        int32_t         legsTimer;        // don't change low priority animations until this runs out
        int32_t         legsAnim;

        int32_t         torsoTimer;        // don't change low priority animations until this runs out
        int32_t         torsoAnim;

        qboolean    legsFlip; //set to opposite when the same anim needs restarting, sent over in only 1 bit. Cleaner and makes porting easier than having that god forsaken ANIM_TOGGLEBIT.
        qboolean    torsoFlip;

        int32_t         movementDir;    // a number 0 to 7 that represents the reletive angle
                                        // of movement to the view angle (axial and diagonals)
                                        // when at rest, the value will remain unchanged
                                        // used to twist the legs during strafing

        int32_t         eFlags;            // copied to entityState_t->eFlags
        int32_t         eFlags2;        // copied to entityState_t->eFlags2, EF2_??? used much less frequently

        int32_t         eventSequence;    // pmove generated events
        int32_t         events[MAX_PS_EVENTS];
        int32_t         eventParms[MAX_PS_EVENTS];

        int32_t         externalEvent;    // events set on player from another source
        int32_t         externalEventParm;
        int32_t         externalEventTime;

        int32_t         clientNum;        // ranges from 0 to MAX_CLIENTS-1
        int32_t         weapon;            // copied to entityState_t->weapon
        int32_t         weaponstate;

        vec3_t        viewangles;        // for fixed views
        int32_t         viewheight;

        // damage feedback
        int32_t         damageEvent;    // when it changes, latch the other parms
        int32_t         damageYaw;
        int32_t         damagePitch;
        int32_t         damageCount;
        int32_t         damageType;

        int32_t         painTime;        // used for both game and client side to process the pain twitch - NOT sent across the network
        int32_t         painDirection;    // NOT sent across the network
        float        yawAngle;        // NOT sent across the network
        qboolean    yawing;            // NOT sent across the network
        float        pitchAngle;        // NOT sent across the network
        qboolean    pitching;        // NOT sent across the network

        int32_t         stats[MAX_STATS];
        int32_t         persistant[MAX_PERSISTANT];    // stats that aren't cleared on death
        int32_t         powerups[MAX_POWERUPS];    // level.time that the powerup runs out
        int32_t         ammo[MAX_WEAPONS];

        int32_t         generic1;
        int32_t         loopSound;
        int32_t         jumppad_ent;    // jumppad entity hit this frame

                                        // not communicated over the net at all
        int32_t         ping;            // server to game info for scoreboard
        int32_t         pmove_framecount;    // FIXME: don't transmit over the network
        int32_t         jumppad_frame;
        int32_t         entityEventSequence;

        int32_t         lastOnGround;    //last time you were on the ground

        qboolean    saberInFlight;

        int32_t         saberMove;
        int32_t         saberBlocking;
        int32_t         saberBlocked;

        int32_t         saberLockTime;
        int32_t         saberLockEnemy;
        int32_t         saberLockFrame; //since we don't actually have the ability to get the current anim frame
        int32_t         saberLockHits; //every x number of buttons hits, allow one push forward in a saber lock (server only)
        int32_t         saberLockHitCheckTime; //so we don't allow more than 1 push per server frame
        int32_t         saberLockHitIncrementTime; //so we don't add a hit per attack button press more than once per server frame
        qboolean    saberLockAdvance; //do an advance (sent across net as 1 bit)

        int32_t         saberEntityNum;
        float        saberEntityDist;
        int32_t         saberEntityState;
        int32_t         saberThrowDelay;
        qboolean    saberCanThrow;
        int32_t         saberDidThrowTime;
        int32_t         saberDamageDebounceTime;
        int32_t         saberHitWallSoundDebounceTime;
        int32_t         saberEventFlags;

        int32_t         rocketLockIndex;
        float        rocketLastValidTime;
        float        rocketLockTime;
        float        rocketTargetTime;

        int32_t         emplacedIndex;
        float        emplacedTime;

        qboolean    isJediMaster;
        qboolean    forceRestricted;
        qboolean    trueJedi;
        qboolean    trueNonJedi;
        int32_t         saberIndex;

        int32_t         genericEnemyIndex;
        float        droneFireTime;
        float        droneExistTime;

        int32_t         activeForcePass;

        qboolean    hasDetPackPlanted; //better than taking up an eFlag isn't it?

        float        holocronsCarried[NUM_FORCE_POWERS];
        int32_t         holocronCantTouch;
        float        holocronCantTouchTime; //for keeping track of the last holocron that just popped out of me (if any)
        int32_t         holocronBits;

        int32_t         electrifyTime;

        int32_t         saberAttackSequence;
        int32_t         saberIdleWound;
        int32_t         saberAttackWound;
        int32_t         saberBlockTime;

        int32_t         otherKiller;
        int32_t         otherKillerTime;
        int32_t         otherKillerDebounceTime;

        forcedata_t    fd;
        qboolean    forceJumpFlip;
        int32_t         forceHandExtend;
        int32_t         forceHandExtendTime;

        int32_t         forceRageDrainTime;

        int32_t         forceDodgeAnim;
        qboolean    quickerGetup;

        int32_t         groundTime;        // time when first left ground

        int32_t         footstepTime;

        int32_t         otherSoundTime;
        float        otherSoundLen;

        int32_t         forceGripMoveInterval;
        int32_t         forceGripChangeMovetype;

        int32_t         forceKickFlip;

        int32_t         duelIndex;
        int32_t         duelTime;
        qboolean    duelInProgress;

        int32_t         saberAttackChainCount;

        int32_t         saberHolstered;

        int32_t         forceAllowDeactivateTime;

        // zoom key
        int32_t         zoomMode;        // 0 - not zoomed, 1 - disruptor weapon
        int32_t         zoomTime;
        qboolean    zoomLocked;
        float        zoomFov;
        int32_t         zoomLockTime;

        int32_t         fallingToDeath;

        int32_t         useDelay;

        qboolean    inAirAnim;

        vec3_t        lastHitLoc;

        int32_t         heldByClient; //can only be a client index - this client should be holding onto my arm using IK stuff.

        int32_t         ragAttach; //attach to ent while ragging

        int32_t         iModelScale;

        int32_t         brokenLimbs;

        //for looking at an entity's origin (NPCs and players)
        qboolean    hasLookTarget;
        int32_t         lookTarget;

        int32_t         customRGBA[4];

        int32_t         standheight;
        int32_t         crouchheight;

        //If non-0, this is the index of the vehicle a player/NPC is riding.
        int32_t         m_iVehicleNum;

        //lovely hack for keeping vehicle orientation in sync with prediction
        vec3_t        vehOrientation;
        qboolean    vehBoarding;
        int32_t         vehSurfaces;

        //vehicle turnaround stuff (need this in ps so it doesn't jerk too much in prediction)
        int32_t         vehTurnaroundIndex;
        int32_t         vehTurnaroundTime;

        //vehicle has weapons linked
        qboolean    vehWeaponsLinked;

        //when hyperspacing, you just go forward really fast for HYPERSPACE_TIME
        int32_t         hyperSpaceTime;
        vec3_t        hyperSpaceAngles;

        //hacking when > time
        int32_t         hackingTime;
        //actual hack amount - only for the proper percentage display when
        //drawing progress bar (is there a less bandwidth-eating way to do
        //this without a lot of hassle?)
        int32_t         hackingBaseTime;

        //keeps track of jetpack fuel
        int32_t         jetpackFuel;

        //keeps track of cloak fuel
        int32_t         cloakFuel;

        //rww - spare values specifically for use by mod authors.
        //See psf_overrides.txt if you want to increase the send
        //amount of any of these above 1 bit.
        int32_t         userInt1;
        int32_t         userInt2;
        int32_t         userInt3;
        float        userFloat1;
        float        userFloat2;
        float        userFloat3;
        vec3_t        userVec1;
        vec3_t        userVec2;
    } playerState_t;

    // ********************************* gameState_t *********************************
    typedef struct {
        size_t stringOffsets[MAX_CONFIGSTRINGS];
        char stringData[MAX_GAMESTATE_CHARS];
        size_t dataCount;
    } gameState_t;

    // snapshots are a view of the server at a given time
    struct clSnapshot_t {
        void reset() noexcept
        {
            static_assert(
                std::is_trivial_v<std::remove_reference_t<decltype(*this)>>
            );
            std::memset(this, 0, sizeof(*this));
        }

        bool valid;                    // cleared if delta parsing was invalid
        int32_t snapFlags;             // rate delayed and dropped commands

        int32_t serverTime;            // server time the message is valid for (in msec)

        int32_t messageNum;            // copied from netchan->incoming_sequence
        int32_t deltaNum;              // messageNum the delta is from
        int32_t ping;                  // time from when cmdNum-1 was sent to time packet was reeceived
        uint8_t areamask[MAX_MAP_AREA_BYTES];  // portalarea visibility bits

        int32_t cmdNum;                // the next cmdNum the server is expecting
        playerState_t ps;              // complete information about the current player at this time
        playerState_t vps;             //vehicle I'm riding's playerstate (if applicable) -rww

        int32_t numEntities;           // all of the entities that need to be presented
        int32_t parseEntitiesNum;      // at the time of this snapshot

        int32_t serverCommandNum;      // execute all commands up to this before
                                       // making the snapshot current
    };

    // ********************************* clientSnapshot_t *********************************
    typedef struct {
        int32_t             areabytes;
        uint8_t            areabits[MAX_MAP_AREA_BYTES];        // portalarea visibility bits
        playerState_t    ps;
        playerState_t    vps; //vehicle I'm riding's playerstate (if applicable) -rww
        int32_t             num_entities;
        int32_t             first_entity;        // into the circular sv_packet_entities[]
                                                // the entities MUST be in increasing state number
                                                // order, otherwise the delta compression will fail
        int32_t             messageSent;        // time the message was transmitted
        int32_t             messageAcked;        // time the message was acked
        int32_t             messageSize;        // used to rate drop packets
    } clientSnapshot_t;

    // ********************************* trajectory_t *********************************
    typedef struct {
        trType_t    trType;
        int32_t     trTime;
        int32_t     trDuration;            // if non 0, trTime + trDuration = stop time
        vec3_t    trBase;
        vec3_t    trDelta;            // velocity, etc
    } trajectory_t;

    // ********************************* entityState_t *********************************
    typedef struct entityState_s {
        int32_t     number;            // entity index
        int32_t     eType;            // entityType_t
        int32_t     eFlags;
        int32_t     eFlags2;        // EF2_??? used much less frequently

        trajectory_t    pos;    // for calculating position
        trajectory_t    apos;    // for calculating angles

        int32_t     time;
        int32_t     time2;

        vec3_t    origin;
        vec3_t    origin2;

        vec3_t    angles;
        vec3_t    angles2;

        //rww - these were originally because we shared g2 info client and server side. Now they
        //just get used as generic values everywhere.
        int32_t     bolt1;
        int32_t     bolt2;

        //rww - this is necessary for determining player visibility during a jedi mindtrick
        int32_t     trickedentindex; //0-15
        int32_t     trickedentindex2; //16-32
        int32_t     trickedentindex3; //33-48
        int32_t     trickedentindex4; //49-64

        float    speed;

        int32_t     fireflag;

        int32_t     genericenemyindex;

        int32_t     activeForcePass;

        int32_t     emplacedOwner;

        int32_t     otherEntityNum;    // shotgun sources, etc
        int32_t     otherEntityNum2;

        int32_t     groundEntityNum;    // -1 = in air

        int32_t     constantLight;    // r + (g<<8) + (b<<16) + (intensity<<24)
        int32_t     loopSound;        // constantly loop this sound
        qboolean    loopIsSoundset; //qtrue if the loopSound index is actually a soundset index

        int32_t     soundSetIndex;

        int32_t     modelGhoul2;
        int32_t     g2radius;
        int32_t     modelindex;
        int32_t     modelindex2;
        int32_t     clientNum;        // 0 to (MAX_CLIENTS - 1), for players and corpses
        int32_t     frame;

        qboolean    saberInFlight;
        int32_t         saberEntityNum;
        int32_t         saberMove;
        int32_t         forcePowersActive;
        int32_t         saberHolstered;//sent in only only 2 bits - should be 0, 1 or 2

        qboolean    isJediMaster;

        qboolean    isPortalEnt; //this needs to be seperate for all entities I guess, which is why I couldn't reuse another value.

        int32_t     solid;            // for client side prediction, trap_linkentity sets this properly

        int32_t     event;            // impulse events -- muzzle flashes, footsteps, etc
        int32_t     eventParm;

        // so crosshair knows what it's looking at
        int32_t         owner;
        int32_t         teamowner;
        qboolean    shouldtarget;

        // for players
        int32_t     powerups;        // bit flags
        int32_t     weapon;            // determines weapon and flash model, etc
        int32_t     legsAnim;
        int32_t     torsoAnim;

        qboolean    legsFlip; //set to opposite when the same anim needs restarting, sent over in only 1 bit. Cleaner and makes porting easier than having that god forsaken ANIM_TOGGLEBIT.
        qboolean    torsoFlip;

        int32_t     forceFrame;        //if non-zero, force the anim frame

        int32_t     generic1;

        int32_t     heldByClient; //can only be a client index - this client should be holding onto my arm using IK stuff.

        int32_t     ragAttach; //attach to ent while ragging

        int32_t     iModelScale; //rww - transfer a percentage of the normal scale in a single int32_t instead of 3 x-y-z scale values

        int32_t     brokenLimbs;

        int32_t     boltToPlayer; //set to index of a real client+1 to bolt the ent to that client. Must be a real client, NOT an NPC.

                                  //for looking at an entity's origin (NPCs and players)
        qboolean    hasLookTarget;
        int32_t         lookTarget;

        int32_t         customRGBA[4];

        //I didn't want to do this, but I.. have no choice. However, we aren't setting this for all ents or anything,
        //only ones we want health knowledge about on cgame (like siege objective breakables) -rww
        int32_t         health;
        int32_t         maxhealth; //so I know how to draw the stupid health bar

                                   //NPC-SPECIFIC FIELDS
                                   //------------------------------------------------------------
        int32_t     npcSaber1;
        int32_t     npcSaber2;

        //index values for each type of sound, gets the folder the sounds
        //are in. I wish there were a better way to do this,
        int32_t     csSounds_Std;
        int32_t     csSounds_Combat;
        int32_t     csSounds_Extra;
        int32_t     csSounds_Jedi;

        int32_t     surfacesOn; //a bitflag of corresponding surfaces from a lookup table. These surfaces will be forced on.
        int32_t     surfacesOff; //same as above, but forced off instead.

                                 //Allow up to 4 PCJ lookup values to be stored here.
                                 //The resolve to configstrings which contain the name of the
                                 //desired bone.
        int32_t     boneIndex1;
        int32_t     boneIndex2;
        int32_t     boneIndex3;
        int32_t     boneIndex4;

        //packed with x, y, z orientations for bone angles
        int32_t     boneOrient;

        //I.. feel bad for doing this, but NPCs really just need to
        //be able to control this sort of thing from the server sometimes.
        //At least it's at the end so this stuff is never going to get sent
        //over for anything that isn't an NPC.
        vec3_t    boneAngles1; //angles of boneIndex1
        vec3_t    boneAngles2; //angles of boneIndex2
        vec3_t    boneAngles3; //angles of boneIndex3
        vec3_t    boneAngles4; //angles of boneIndex4

        int32_t     NPC_class; //we need to see what it is on the client for a few effects.

                               //If non-0, this is the index of the vehicle a player/NPC is riding.
        int32_t     m_iVehicleNum;

        //rww - spare values specifically for use by mod authors.
        //See netf_overrides.txt if you want to increase the send
        //amount of any of these above 1 bit.
        int32_t         userInt1;
        int32_t         userInt2;
        int32_t         userInt3;
        float        userFloat1;
        float        userFloat2;
        float        userFloat3;
        vec3_t        userVec1;
        vec3_t        userVec2;
    } entityState_t;

    // ********************************* outPacket_t *********************************
    typedef struct {
        int32_t  p_cmdNumber;    // cl.cmdNumber when packet was sent
        int32_t  p_serverTime;   // usercmd->serverTime when packet was sent
        int32_t  p_realtime;     // cls.realtime when packet was sent
    } outPacket_t;

    // ********************************* minimalPlayerState_t *********************************
    typedef struct {
        qboolean isPilot : 1;
        int8_t lc;
        qboolean statsSent : 1;
    } minimalPlayerState_t;

    // ********************************* minimalPacketEntities *********************************
    typedef struct {
        uint32_t newnum : 10;
    } minimalPacketEntities;

    // ********************************* minimalSnapshot_t *********************************
    typedef struct {
        int32_t svTime;
        int8_t deltaNum;
        int8_t snapFlags;
        int8_t areaMaskLen;
        int8_t areaMask;
        minimalPlayerState_t ps;
        minimalPacketEntities entities;
    } minimalSnapshot_t;

    // ********************************* score_t *********************************
    typedef struct {
        int32_t client;
        int32_t score;
        int32_t ping;
        int32_t time;
        int32_t scoreFlags;
        int32_t powerUps;
        int32_t accuracy;
        int32_t impressiveCount;
        int32_t excellentCount;
        int32_t guantletCount;
        int32_t defendCount;
        int32_t assistCount;
        int32_t captures;
        bool perfect;
        int32_t team;
    } score_t;
}
