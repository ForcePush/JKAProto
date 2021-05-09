#pragma once
#include <string_view>
#include "JKAConstants.h"

namespace JKA
{
    // server to client
    enum svc_ops_e {
        svc_bad,
        svc_nop,
        svc_gamestate,
        svc_configstring,            // [short] [string] only in gamestate messages
        svc_baseline,                // only in gamestate messages
        svc_serverCommand,            // [string] to be executed by client game module
        svc_download,                // [short] size [size bytes]
        svc_snapshot,
        svc_setgame,
        svc_mapchange,
        svc_EOF
    };

    // client to server
    enum clc_ops_e {
        clc_bad,
        clc_nop,
        clc_move,               // [[usercmd_t]
        clc_moveNoDelta,        // [[usercmd_t]
        clc_clientCommand,      // [string] message
        clc_EOF
    };

    typedef enum {
        CS_FREE,        // can be reused for a new connection
        CS_ZOMBIE,      // client has been disconnected, but don't reuse
                        // connection for a couple seconds
        CS_CONNECTED,   // has been assigned to a client_t, but no gamestate yet
        CS_PRIMED,      // gamestate has been sent, but client hasn't sent a usercmd
        CS_ACTIVE       // client is fully in game
    } clientState_t;

    typedef enum {
        CA_UNINITIALIZED,
        CA_DISCONNECTED, 	// not talking to a server
        CA_AUTHORIZING,		// not used any more, was checking cd key 
        CA_CONNECTING,		// sending request packets to the server
        CA_CHALLENGING,		// sending challenge packets to the server
        CA_CONNECTED,		// netchan_t established, getting gamestate
        CA_LOADING,			// only during cgame initialization, never during main loop
        CA_PRIMED,			// got gamestate, waiting for first frame
        CA_ACTIVE,			// game views should be displayed
        CA_CINEMATIC		// playing a cinematic or a static pic, not connected to a server
    } connstate_t;

    enum ConnlessType {
        CLS__BEGIN = 0,

        CLS_GETINFO = CLS__BEGIN,
        CLS_GETSTATUS,
        CLS_GETCHALLENGE,
        CLS_CONNECT,

        CLS_GETSERVERS,

        CLS_GETINFO_RESPONSE,
        CLS_GETSTATUS_RESPONSE,
        CLS_GETCHALLENGE_RESPONSE,
        CLS_CONNECT_RESPONSE,

        CLS_GETSERVERS_RESPONSE,

        CLS_RCON,
        CLS_PRINT,

        CLS_DISCONNECT,

        CLS__MAX,
        CLS__END = CLS__MAX,
        CLS__BAD = CLS__MAX
    };

    struct ConnlessPacketDef {
        ConnlessType type;
        std::string_view className;  // NOTE: not zero-terminated.
        std::string_view name;       // NOTE: not zero-terminated.
        std::string_view separator;  // NOTE: not zero-terminated.
    };

    enum
    {
        FP_FIRST = 0,//marker
        FP_HEAL = 0,//instant
        FP_LEVITATION,//hold/duration
        FP_SPEED,//duration
        FP_PUSH,//hold/duration
        FP_PULL,//hold/duration
        FP_TELEPATHY,//instant
        FP_GRIP,//hold/duration
        FP_LIGHTNING,//hold/duration
        FP_RAGE,//duration
        FP_PROTECT,
        FP_ABSORB,
        FP_TEAM_HEAL,
        FP_TEAM_FORCE,
        FP_DRAIN,
        FP_SEE,
        FP_SABER_OFFENSE,
        FP_SABER_DEFENSE,
        FP_SABERTHROW,
        NUM_FORCE_POWERS
    };
    typedef int forcePowers_t;

    typedef enum {
        TR_STATIONARY,
        TR_INTERPOLATE,                // non-parametric, but interpolate between snapshots
        TR_LINEAR,
        TR_LINEAR_STOP,
        TR_NONLINEAR_STOP,
        TR_SINE,                    // value = base + sin( time / duration ) * delta
        TR_GRAVITY
    } trType_t;

    // player_state->persistant[] indexes
    typedef enum {
        PERS_SCORE,                        // !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
        PERS_HITS,                        // total points damage inflicted so damage beeps can sound on change
        PERS_RANK,                        // player rank or team rank
        PERS_TEAM,                        // player team
        PERS_SPAWN_COUNT,                // incremented every respawn
        PERS_PLAYEREVENTS,                // 16 bits that can be flipped for events
        PERS_ATTACKER,                    // clientnum of last damage inflicter
        PERS_ATTACKEE_ARMOR,            // health/armor of last person we attacked
        PERS_KILLED,
        PERS_IMPRESSIVE_COUNT,            // two railgun hits in a row
        PERS_EXCELLENT_COUNT,            // two successive kills in a short amount of time
        PERS_DEFEND_COUNT,                // defend awards
        PERS_ASSIST_COUNT,                // assist awards
        PERS_GAUNTLET_FRAG_COUNT,        // kills with the guantlet
        PERS_CAPTURES                    // captures
    } persEnum_t;

    enum {
        TEAM_FREE,
        TEAM_RED,
        TEAM_BLUE,
        TEAM_SPECTATOR,

        TEAM_NUM_TEAMS
    };
    typedef int32_t team_t;

    // player_state->stats[] indexes
    typedef enum {
        STAT_HEALTH,
        STAT_HOLDABLE_ITEM,
        STAT_HOLDABLE_ITEMS,
        STAT_PERSISTANT_POWERUP,
        STAT_WEAPONS = 4,                    // 16 bit fields
        STAT_ARMOR,
        STAT_DEAD_YAW,                    // look this direction when dead (FIXME: get rid of?)
        STAT_CLIENTS_READY,                // bit mask of clients wishing to exit the intermission (FIXME: configstring?)
        STAT_MAX_HEALTH                    // health / armor limit, changable by handicap
    } statIndex_t;

    typedef enum {
        PM_NORMAL,        // can accelerate and turn
        PM_JETPACK,        // special jetpack movement
        PM_FLOAT,        // float with no gravity in general direction of velocity (intended for gripping)
        PM_NOCLIP,        // noclip movement
        PM_SPECTATOR,    // still run into walls
        PM_DEAD,        // no acceleration or turning, but free falling
        PM_FREEZE,        // stuck in place with no control
        PM_INTERMISSION,    // no movement or status bar
        PM_SPINTERMISSION    // no movement or status bar
    } pmtype_t;

    typedef enum {
        ET_GENERAL,
        ET_PLAYER,
        ET_ITEM,
        ET_MISSILE,
        ET_SPECIAL,                // rww - force fields
        ET_HOLOCRON,            // rww - holocron icon displays
        ET_MOVER,
        ET_BEAM,
        ET_PORTAL,
        ET_SPEAKER,
        ET_PUSH_TRIGGER,
        ET_TELEPORT_TRIGGER,
        ET_INVISIBLE,
        ET_NPC,                    // ghoul2 player-like entity
        ET_TEAM,
        ET_BODY,
        ET_TERRAIN,
        ET_FX,

        ET_EVENTS                // any of the EV_* events can be added freestanding
                                // by setting eType to ET_EVENTS + eventNum
                                // this avoids having to set eFlags and eventNum
    } entityType_t;
}
