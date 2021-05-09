#pragma once
#include <cinttypes>
#include <cstdlib>
#include <string_view>

namespace JKA {
    static constexpr uint8_t CONNLESS_PREFIX[] = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };
    static constexpr char    CONNLESS_PREFIX_C[] = { '\xFF', '\xFF', '\xFF', '\xFF' };
    static constexpr auto    CONNLESS_PREFIX_SIZE = sizeof(CONNLESS_PREFIX);
    static constexpr auto    CONNLESS_PREFIX_S = std::string_view(CONNLESS_PREFIX_C, CONNLESS_PREFIX_SIZE);

    static constexpr auto    MAX_PLAYERS = 32;
    static constexpr auto    MAX_CLIENTS = MAX_PLAYERS;
    static constexpr auto    MAX_QPATH = 64;

    static constexpr auto    PACKET_BACKUP = 32;
    static constexpr auto    PACKET_MASK = (PACKET_BACKUP - 1);
    static constexpr auto    CMD_BACKUP = 64;
    static constexpr auto    CMD_MASK = (CMD_BACKUP - 1);
    static constexpr auto    MAX_PACKET_USERCMDS = 32;

    static constexpr auto    MAX_PACKETLEN = 1400;
    static constexpr auto    FRAGMENT_SIZE = (MAX_PACKETLEN - 100);
    static constexpr auto    FRAGMENT_BIT = (1u << 31u);
    static constexpr auto    MAX_CONNECT_LEN = FRAGMENT_SIZE;

    static constexpr auto    MAX_INFO_LEN = 4096;
    static constexpr auto    MAX_BIG_STRING = 8192;
    static constexpr auto    MAX_STRING_CHARS = 1024;
    static constexpr auto    MAX_RELIABLE_COMMANDS = 128;

    static constexpr auto    SV_ENCODE_START = 8;
    static constexpr auto    SV_DECODE_START = 12;
    static constexpr auto    CL_ENCODE_START = 12;
    static constexpr auto    CL_DECODE_START = 4;

    static constexpr auto    MAX_MSGLEN = 49152;

    static constexpr auto    MAX_HEIGHTMAP_SIZE = 16000;

    // for forcedata_t
    static constexpr auto    TRACK_CHANNEL_MAX = 6;

    // for playerState_t
    static constexpr auto    MAX_STATS = 16;
    static constexpr auto    MAX_PERSISTANT = 16;
    static constexpr auto    MAX_POWERUPS = 16;
    static constexpr auto    MAX_WEAPONS = 19;
    static constexpr auto    MAX_MAP_AREA_BYTES = 32;
    static constexpr auto    MAX_PS_EVENTS = 2;
    // angle indexes
    static constexpr auto    PITCH = 0;    // up / down
    static constexpr auto    YAW = 1;      // left / right
    static constexpr auto    ROLL = 2;     // fall over

    // for gameState_t
    static constexpr auto    MAX_GAMESTATE_CHARS = 16000;
    static constexpr auto    MAX_CONFIGSTRINGS = 1700;
    static constexpr auto    CS_SERVERINFO = 0;  // an info string with all the serverinfo cvars
    static constexpr auto    CS_SYSTEMINFO = 1;  // an info string for server system to client system configuration (timescale, etc)

    // for snapshots
    static constexpr auto    SNAPFLAG_RATE_DELAYED = 1;
    static constexpr auto    SNAPFLAG_NOT_ACTIVE = 2;     // snapshot used during connection and for zombies
    static constexpr auto    SNAPFLAG_SERVERCOUNT = 4;    // toggled every map_restart so transitions can be detected
    static constexpr auto    MAX_PARSE_ENTITIES = 2048;

    // for net
    static constexpr auto    MAX_PARMS = 16;
    static constexpr auto    MAX_PARM_STRING_LENGTH = MAX_QPATH;

    static constexpr auto    GENTITYNUM_BITS = 10;
    static constexpr auto    MAX_GENTITIES = (1 << GENTITYNUM_BITS);
    static constexpr auto    ENTITYNUM_NONE = MAX_GENTITIES - 1;

    static constexpr auto    FLOAT_INT_BITS = 13;
    static constexpr auto    FLOAT_INT_BIAS = (1 << (FLOAT_INT_BITS - 1));

    static constexpr auto    CM_ANGLE1 = (1 << 0);
    static constexpr auto    CM_ANGLE2 = (1 << 1);
    static constexpr auto    CM_ANGLE3 = (1 << 2);
    static constexpr auto    CM_FORWARD = (1 << 3);
    static constexpr auto    CM_SIDE = (1 << 4);
    static constexpr auto    CM_UP = (1 << 5);
    static constexpr auto    CM_BUTTONS = (1 << 6);
    static constexpr auto    CM_WEAPON = (1 << 7);
    static constexpr auto    CM_FORCE = (1 << 8);
    static constexpr auto    CM_INVEN = (1 << 9);

    // entityState_t->eFlags;
    static constexpr auto    EF_G2ANIMATING = (1u << 0);
    static constexpr auto    EF_DEAD = (1u << 1);
    static constexpr auto    EF_RADAROBJECT = (1u << 2);
    static constexpr auto    EF_TELEPORT_BIT = (1u << 3);
    static constexpr auto    EF_SHADER_ANIM = (1u << 4);
    static constexpr auto    EF_PLAYER_EVENT = (1u << 5);
    static constexpr auto    EF_RAG = (1u << 6);
    static constexpr auto    EF_PERMANENT = (1u << 7);
    static constexpr auto    EF_NODRAW = (1u << 8);
    static constexpr auto    EF_FIRING = (1u << 9);
    static constexpr auto    EF_ALT_FIRING = (1u << 10);
    static constexpr auto    EF_JETPACK_ACTIVE = (1u << 11);
    static constexpr auto    EF_NOT_USED_1 = (1u << 12);
    static constexpr auto    EF_TALK = (1u << 13);
    static constexpr auto    EF_CONNECTION = (1u << 14);
    static constexpr auto    EF_NOT_USED_6 = (1u << 15);
    static constexpr auto    EF_NOT_USED_2 = (1u << 16);
    static constexpr auto    EF_NOT_USED_3 = (1u << 17);
    static constexpr auto    EF_NOT_USED_4 = (1u << 18);
    static constexpr auto    EF_BODYPUSH = (1u << 19);
    static constexpr auto    EF_DOUBLE_AMMO = (1u << 20);
    static constexpr auto    EF_SEEKERDRONE = (1u << 21);
    static constexpr auto    EF_MISSILE_STICK = (1u << 22);
    static constexpr auto    EF_ITEMPLACEHOLDER = (1u << 23);
    static constexpr auto    EF_SOUNDTRACKER = (1u << 24);
    static constexpr auto    EF_DROPPEDWEAPON = (1u << 25);
    static constexpr auto    EF_DISINTEGRATION = (1u << 26);
    static constexpr auto    EF_INVULNERABLE = (1u << 27);
    static constexpr auto    EF_CLIENTSMOOTH = (1u << 28);
    static constexpr auto    EF_JETPACK = (1u << 29);
    static constexpr auto    EF_JETPACK_FLAMING = (1u << 30);
    static constexpr auto    EF_NOT_USED_5 = (1u << 31);

    static constexpr auto    EF2_HELD_BY_MONSTER = (1u << 0);
    static constexpr auto    EF2_USE_ALT_ANIM = (1u << 1);
    static constexpr auto    EF2_ALERTED = (1u << 2);
    static constexpr auto    EF2_GENERIC_NPC_FLAG = (1u << 3);
    static constexpr auto    EF2_FLYING = (1u << 4);
    static constexpr auto    EF2_HYPERSPACE = (1u << 5);
    static constexpr auto    EF2_BRACKET_ENTITY = (1u << 6);
    static constexpr auto    EF2_SHIP_DEATH = (1u << 7);
    static constexpr auto    EF2_NOT_USED_1 = (1u << 8);

    // For RMG
    static constexpr auto    MAX_AUTOMAP_SYMBOLS = 512;

    // For configstrings
    static constexpr auto    MAX_AMBIENT_SETS = 256;
    static constexpr auto	 MAX_MODELS = 512;
    static constexpr auto	 MAX_SOUNDS = 256;
    static constexpr auto    MAX_ICONS = 64;
    static constexpr auto    MAX_FX = 64;
    static constexpr auto    MAX_G2BONES = 64;
    static constexpr auto    MAX_LOCATIONS = 64;
    static constexpr auto    MAX_LIGHT_STYLES = 64;
    static constexpr auto    MAX_TERRAINS = 1;

    static constexpr auto    MAX_SUB_BSP = 32;

    // Configstrings
    static constexpr auto    CS_MUSIC = 2;
    static constexpr auto    CS_MESSAGE = 3;
    static constexpr auto    CS_MOTD = 4;
    static constexpr auto    CS_WARMUP = 5;
    static constexpr auto    CS_SCORES1 = 6;
    static constexpr auto    CS_SCORES2 = 7;
    static constexpr auto    CS_VOTE_TIME = 8;
    static constexpr auto    CS_VOTE_STRING = 9;
    static constexpr auto    CS_VOTE_YES = 10;
    static constexpr auto    CS_VOTE_NO = 11;

    static constexpr auto    CS_TEAMVOTE_TIME = 12;
    static constexpr auto    CS_TEAMVOTE_STRING = 14;
    static constexpr auto    CS_TEAMVOTE_YES = 16;
    static constexpr auto    CS_TEAMVOTE_NO = 18;

    static constexpr auto    CS_GAME_VERSION = 20;
    static constexpr auto    CS_LEVEL_START_TIME = 21;
    static constexpr auto    CS_INTERMISSION = 22;
    static constexpr auto    CS_FLAGSTATUS = 23;
    static constexpr auto    CS_SHADERSTATE = 24;
    static constexpr auto    CS_BOTINFO = 25;

    static constexpr auto    CS_ITEMS = 27;

    static constexpr auto    CS_CLIENT_JEDIMASTER = 28;
    static constexpr auto    CS_CLIENT_DUELWINNER = 29;
    static constexpr auto    CS_CLIENT_DUELISTS = 30;
    static constexpr auto    CS_CLIENT_DUELHEALTHS = 31;
    static constexpr auto    CS_GLOBAL_AMBIENT_SET = 32;

    static constexpr auto    CS_AMBIENT_SET = 37;

    static constexpr auto    CS_SIEGE_STATE = (CS_AMBIENT_SET + MAX_AMBIENT_SETS);
    static constexpr auto    CS_SIEGE_OBJECTIVES = (CS_SIEGE_STATE + 1);
    static constexpr auto    CS_SIEGE_TIMEOVERRIDE = (CS_SIEGE_OBJECTIVES + 1);
    static constexpr auto    CS_SIEGE_WINTEAM = (CS_SIEGE_TIMEOVERRIDE + 1);
    static constexpr auto    CS_SIEGE_ICONS = (CS_SIEGE_WINTEAM + 1);

    static constexpr auto    CS_MODELS = (CS_SIEGE_ICONS + 1);
    static constexpr auto    CS_SKYBOXORG = (CS_MODELS + MAX_MODELS);
    static constexpr auto    CS_SOUNDS = (CS_SKYBOXORG + 1);
    static constexpr auto    CS_ICONS = (CS_SOUNDS + MAX_SOUNDS);
    static constexpr auto    CS_PLAYERS = (CS_ICONS + MAX_ICONS);

    static constexpr auto    CS_G2BONES = (CS_PLAYERS + MAX_PLAYERS);

    static constexpr auto    CS_LOCATIONS = (CS_G2BONES + MAX_G2BONES);
    static constexpr auto    CS_PARTICLES = (CS_LOCATIONS + MAX_LOCATIONS);
    static constexpr auto    CS_EFFECTS = (CS_PARTICLES + MAX_LOCATIONS);
    static constexpr auto    CS_LIGHT_STYLES = (CS_EFFECTS + MAX_FX);

    static constexpr auto    CS_TERRAINS = (CS_LIGHT_STYLES + (MAX_LIGHT_STYLES * 3));
    static constexpr auto    CS_BSP_MODELS = (CS_TERRAINS + MAX_TERRAINS);

    static constexpr auto    CS_MAX = (CS_BSP_MODELS + MAX_SUB_BSP);
}
