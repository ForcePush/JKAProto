#include "../include/CompressedMessage.h"

#include <cassert>
#include <iostream>
#include <sstream>

CompressedMessage::CompressedMessage(Q3Huffman & huffman) :
    huff(huffman)
{
}

CompressedMessage::CompressedMessage(Q3Huffman & huffman, void *buf, size_t bufSize, size_t maxSize) :
    cursize(bufSize),
    maxSize(maxSize),
    overflowed(false),
    huff(huffman)
{
    data.buf = buf;
    dataStream = WriteableByteBitStream(data.ucharBuf);
}

CompressedMessage::CompressedMessage(CompressedMessage && other) noexcept :
    readcount(std::move(other.readcount)),
    bit(std::move(other.bit)),
    cursize(std::move(other.cursize)),
    maxSize(std::move(other.maxSize)),
    overflowed(std::move(other.overflowed)),
    huff(other.huff)
{
    data.buf = other.data.buf;
    other.data.buf = nullptr;
    other.cursize = other.maxSize = 0;
}

CompressedMessage & CompressedMessage::operator=(CompressedMessage && other) noexcept
{
    dataStream = other.dataStream;

    readcount = other.readcount;
    bit = other.bit;
    cursize = other.cursize;
    maxSize = other.maxSize;
    overflowed = other.overflowed;

    huff = std::move(other.huff);  // TODO: FIXME: prevent copying of other's huffman!!!

    data.buf = other.data.buf;

    other.data.buf = nullptr;
    other.cursize = other.maxSize = 0;

    return *this;
}

CompressedMessage::~CompressedMessage()
{
}

void CompressedMessage::writeBit(int32_t val)
{
    writeBits(val & 1, 1);
}

// ************************** WRITE **************************
void CompressedMessage::writeBits(int32_t val, int32_t bits)
{
    int32_t i;

    // this isn't an exact overflow check, but close enough
    if (maxSize - cursize < 4) {
        overflowed = true;
        return;
    }

    if (bits == 0 || bits < -31 || bits > 32) {
        return;
    }

    if (bits < 0) {
        bits = -bits;
    }
    val &= (0xffffffff >> (32 - bits));
    if (bits & 7) {
        int nbits;
        nbits = bits & 7;
        for (i = 0; i < nbits; i++) {
            dataStream.putBit((val & 1), &bit);
            val = (val >> 1);
        }
        bits = bits - nbits;
    }
    if (bits) {
        for (i = 0; i < bits; i += 8) {
            huff.offsetTransmit(Huffman::HUFF_COMPRESS, (val & 0xff), dataStream, &bit);
            val = (val >> 8);
        }
    }
    cursize = (bit >> 3) + 1;
}

void CompressedMessage::writeLong(int32_t val)
{
    writeBits(val, 32);
}

void CompressedMessage::writeShort(int32_t val)
{
    writeBits(val, 16);
}

void CompressedMessage::writeByte(int32_t val)
{
    writeBits(val, 8);
}

void CompressedMessage::writeFloat(float val)
{
    static_assert(sizeof(float) == sizeof(int32_t), "sizeof(float) must be == 4 bytes");
    union {
        float f;
        int32_t l;
    } dat;

    dat.f = val;
    writeBits(dat.l, 32);
}

void CompressedMessage::writeString(const char *str)
{
    if (!str) {
        writeData("\x00", 1);
    }
    else {
        // TODO: eliminate '%' somehow?
        writeData(str, strlen(str) + 1);
    }
}

void CompressedMessage::writeData(const void *data_, size_t dataSize)
{
    // TODO: get rid of reinterpret_cast?
    for (size_t i = 0; i < dataSize; i++) {
        writeByte((reinterpret_cast<const uint8_t*>(data_))[i]);
    }
}

// ************************** READ **************************
int32_t CompressedMessage::readBits(int32_t bits)
{
    int            value;
    int            get;
    bool        sgn;
    int            i, nbits;
    value = 0;

    if (bits < 0) {
        bits = -bits;
        sgn = true;
    }
    else {
        sgn = false;
    }

    nbits = 0;
    if (bits & 7) {
        nbits = bits & 7;
        for (i = 0; i < nbits; i++) {
            value |= (dataStream.getBitOffset(&bit) << i);
        }
        bits = bits - nbits;
    }
    if (bits) {
        for (i = 0; i < bits; i += 8) {
            huff.offsetReceive(Huffman::HUFF_DECOMPRESS, &get, dataStream, &bit);
            value |= (get << (i + nbits));
        }
    }
    readcount = (bit >> 3) + 1;
    if (sgn) {
        if (value & (1 << (bits - 1))) {
            value |= -1 ^ ((1 << bits) - 1);
        }
    }

    return value;
}

int32_t CompressedMessage::readLong()
{
    int    c;

    c = readBits(32);
    if (readcount > cursize) {
        c = -1;
    }

    return c;
}

int16_t CompressedMessage::readShort()
{
    int32_t    c;

    c = (int16_t)readBits(16);
    if (readcount > cursize) {
        c = -1;
    }

    return static_cast<int16_t>(c);
}

uint16_t CompressedMessage::readUShort()
{
    return static_cast<uint16_t>(readShort());
}

uint8_t CompressedMessage::readByte(bool readSigned)
{
    int32_t    c;

    if (readSigned) {
        c = static_cast<int8_t>(readBits(8));
    }
    else {
        c = static_cast<uint8_t>(readBits(8));
    }
    if (readcount > cursize) {
        c = -1;
    }
    return static_cast<uint8_t>(c);
}

void CompressedMessage::readData(void *data_, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        *static_cast<uint8_t *>(data_) = readByte();
    }
}

float CompressedMessage::readFloat()
{
    union {
        uint8_t    b[4];
        float    f;
        int    l;
    } dat;

    dat.l = readBits(32);
    if (readcount > cursize) {
        dat.f = -1;
    }

    return dat.f;
}

std::string CompressedMessage::readString(bool breakOnNewline, bool translatePercent)
{
    std::ostringstream ss{};

    int32_t l = 0, c = 0;
    do {
        c = readByte(true);  // use unsigned read so -1 is out of bounds
        if (c == -1 || c == 0 || (breakOnNewline && c == '\n')) {
            break;
        }

        if (c == '%' && translatePercent) {
            c = '.';
        }

        ss << static_cast<char>(c);
        l++;
    } while (l <= JKA::MAX_BIG_STRING - 1);

    return ss.str();
}

std::string CompressedMessage::readStringLine(bool translatePercent)
{
    return readString(true, translatePercent);
}

int32_t CompressedMessage::peekLong()
{
    auto state = saveState();
    int32_t result = readLong();
    restoreState(state);
    return result;
}

int32_t CompressedMessage::peekLongOOB()
{
    return *reinterpret_cast<const int32_t*>(data.ucharBuf + readcount);
}

// ************************** OOB **************************
int32_t CompressedMessage::readOOB(int32_t bits)
{
    int32_t res = -1;

    switch (bits) {
    case 32:
        res = *reinterpret_cast<const int32_t*>(data.ucharBuf + readcount);
        readcount += 4;
        bit += 32;
        break;

    case 16:
        res = *reinterpret_cast<const int16_t*>(data.ucharBuf + readcount);
        readcount += 2;
        bit += 16;
        break;

    case 8:
        res = *reinterpret_cast<const int8_t*>(data.ucharBuf + readcount);
        readcount += 1;
        bit += 8;
        break;

    default: assert(!"Invalid bits count"); return -1;
    }

    dataStream.setLocation(dataStream.getLocation() + bits);

    return res;
}

void CompressedMessage::writeOOB(int32_t value, int32_t bits)
{
    switch (bits) {
    case 32:
        *reinterpret_cast<int32_t*>(data.ucharBuf + cursize) = value;
        cursize += 4;
        bit += 32;  // TODO: FIXME: original: 8. Just as planned?
        break;

    case 16:
        *reinterpret_cast<int16_t*>(data.ucharBuf + cursize) = value & 0xFFFF;
        cursize += 2;
        bit += 16;
        break;

    case 8:
        *reinterpret_cast<int8_t*>(data.ucharBuf + cursize) = value & 0xFF;
        cursize += 1;
        bit += 8;
        break;

    default:
        assert(false);
        return;
    }

    dataStream.setLocation(dataStream.getLocation() + bits);
}

// ************************** READ/WRITE KEY **************************
static constexpr uint32_t kbitmask[32] = {
    0x00000001, 0x00000003, 0x00000007, 0x0000000F,
    0x0000001F,    0x0000003F,    0x0000007F,    0x000000FF,
    0x000001FF,    0x000003FF,    0x000007FF,    0x00000FFF,
    0x00001FFF,    0x00003FFF,    0x00007FFF,    0x0000FFFF,
    0x0001FFFF,    0x0003FFFF,    0x0007FFFF,    0x000FFFFF,
    0x001FFFFf,    0x003FFFFF,    0x007FFFFF,    0x00FFFFFF,
    0x01FFFFFF,    0x03FFFFFF,    0x07FFFFFF,    0x0FFFFFFF,
    0x1FFFFFFF,    0x3FFFFFFF,    0x7FFFFFFF,    0xFFFFFFFF,
};

void CompressedMessage::writeDeltaKey(int32_t key, int32_t oldV, int32_t newV, int32_t bits)
{
    if (oldV == newV) {
        writeBit(0);
        return;
    }
    writeBit(1);
    writeBits((newV ^ key) & ((1ll << bits) - 1), bits);
}

int32_t CompressedMessage::readDeltaKey(int32_t key, int32_t oldV, int32_t bits)
{
    if (readBits(1)) {
        return readBits(bits) ^ (key & kbitmask[bits]);
    }
    return oldV;
}

void CompressedMessage::writeDeltaKeyFloat(int32_t key, float oldV, float newV)
{
    if (oldV == newV) {
        writeBits(0, 1);
        return;
    }
    writeBits(1, 1);
    writeBits(float_to_int(newV) ^ key, 32);
}

float CompressedMessage::readDeltaKeyFloat(int32_t key, float oldV)
{
    if (readBits(1)) {
        return int_to_float(readBits(32) ^ key);
    }
    return oldV;
}

// ************************** DELTA **************************
void CompressedMessage::writeDelta(int32_t oldV, int32_t newV, int32_t bits)
{
    if (oldV == newV) {
        writeBits(0, 1);
        return;
    }
    writeBits(1, 1);
    writeBits(newV, bits);
}

int32_t CompressedMessage::readDelta(int32_t oldV, int32_t bits)
{
    if (readBits(1)) {
        return readBits(bits);
    }
    return oldV;
}

void CompressedMessage::writeDeltaFloat(float oldV, float newV)
{
    if (oldV == newV) {
        writeBits(0, 1);
        return;
    }
    writeBits(1, 1);
    writeBits(float_to_int(newV), 32);
}

float CompressedMessage::readDeltaFloat(float oldV)
{
    if (readBits(1)) {
        return int_to_float(readBits(32));
    }
    return oldV;
}

void CompressedMessage::writeDeltaUsercmd(const JKA::usercmd_t *from, const JKA::usercmd_t *to)
{
    int32_t timeDelta = to->serverTime - from->serverTime;
    if (timeDelta < 256) {
        writeBits(1, 1);
        writeByte(timeDelta);
    } else {
        writeBits(0, 1);
        writeLong(to->serverTime);
    }
    writeDelta(from->angles[0], to->angles[0], 16);
    writeDelta(from->angles[1], to->angles[1], 16);
    writeDelta(from->angles[2], to->angles[2], 16);
    writeDelta(from->forwardmove, to->forwardmove, 8);
    writeDelta(from->rightmove, to->rightmove, 8);
    writeDelta(from->upmove, to->upmove, 8);
    writeDelta(from->buttons, to->buttons, 16);
    writeDelta(from->weapon, to->weapon, 8);

    writeDelta(from->forcesel, to->forcesel, 8);
    writeDelta(from->invensel, to->invensel, 8);

    writeDelta(from->generic_cmd, to->generic_cmd, 8);
}

void CompressedMessage::readDeltaUsercmd(const JKA::usercmd_t *from, JKA::usercmd_t *to)
{
    if (readBits(1)) {
        to->serverTime = from->serverTime + readByte();
    }
    else {
        to->serverTime = readLong();
    }
    to->angles[0] = readDelta(from->angles[0], 16);
    to->angles[1] = readDelta(from->angles[1], 16);
    to->angles[2] = readDelta(from->angles[2], 16);
    to->forwardmove = static_cast<int8_t>(readDelta(from->forwardmove, 8));
    to->rightmove = static_cast<int8_t>(readDelta(from->rightmove, 8));
    to->upmove = static_cast<int8_t>(readDelta(from->upmove, 8));
    to->buttons = readDelta(from->buttons, 16);
    to->weapon = static_cast<uint8_t>(readDelta(from->weapon, 8));

    to->forcesel = static_cast<uint8_t>(readDelta(from->forcesel, 8));
    to->invensel = static_cast<uint8_t>(readDelta(from->invensel, 8));

    to->generic_cmd = static_cast<uint8_t>(readDelta(from->generic_cmd, 8));
}

void CompressedMessage::readDeltaUsercmdKey(int32_t key, const JKA::usercmd_t *from, JKA::usercmd_t *to)
{
    if (readBits(1)) {
        to->serverTime = from->serverTime + readByte();
    } else {
        to->serverTime = readLong();
    }
    if (readBits(1)) {
        key ^= to->serverTime;
        to->angles[0] = readDeltaKey(key, from->angles[0], 16);
        to->angles[1] = readDeltaKey(key, from->angles[1], 16);
        to->angles[2] = readDeltaKey(key, from->angles[2], 16);
        to->forwardmove = static_cast<int8_t>(readDeltaKey(key, from->forwardmove, 8));
        to->rightmove = static_cast<int8_t>(readDeltaKey(key, from->rightmove, 8));
        to->upmove = static_cast<int8_t>(readDeltaKey(key, from->upmove, 8));
        to->buttons = readDeltaKey(key, from->buttons, 16);
        to->weapon = static_cast<uint8_t>(readDeltaKey(key, from->weapon, 8));

        to->forcesel = static_cast<uint8_t>(readDeltaKey(key, from->forcesel, 8));
        to->invensel = static_cast<uint8_t>(readDeltaKey(key, from->invensel, 8));

        to->generic_cmd = static_cast<uint8_t>(readDeltaKey(key, from->generic_cmd, 8));
    } else {
        to->angles[0] = from->angles[0];
        to->angles[1] = from->angles[1];
        to->angles[2] = from->angles[2];
        to->forwardmove = from->forwardmove;
        to->rightmove = from->rightmove;
        to->upmove = from->upmove;
        to->buttons = from->buttons;
        to->weapon = from->weapon;

        to->forcesel = from->forcesel;
        to->invensel = from->invensel;

        to->generic_cmd = from->generic_cmd;
    }
}

void CompressedMessage::writeDeltaUsercmdKey(int32_t key, const JKA::usercmd_t *from, const JKA::usercmd_t *to)
{
    int32_t timeDelta = to->serverTime - from->serverTime;
    if (timeDelta < 256) {
        writeBit(1);
        writeByte(timeDelta);
    }  else {
        writeBit(0);
        writeLong(to->serverTime);
    }
    if (from->angles[0] == to->angles[0] &&
        from->angles[1] == to->angles[1] &&
        from->angles[2] == to->angles[2] &&
        from->forwardmove == to->forwardmove &&
        from->rightmove == to->rightmove &&
        from->upmove == to->upmove &&
        from->buttons == to->buttons &&
        from->weapon == to->weapon &&
        from->forcesel == to->forcesel &&
        from->invensel == to->invensel &&
        from->generic_cmd == to->generic_cmd) {
        writeBit(0);  // no change
        return;
    }
    key ^= to->serverTime;
    writeBit(1);
    writeDeltaKey(key, from->angles[0], to->angles[0], 16);
    writeDeltaKey(key, from->angles[1], to->angles[1], 16);
    writeDeltaKey(key, from->angles[2], to->angles[2], 16);
    writeDeltaKey(key, from->forwardmove, to->forwardmove, 8);
    writeDeltaKey(key, from->rightmove, to->rightmove, 8);
    writeDeltaKey(key, from->upmove, to->upmove, 8);
    writeDeltaKey(key, from->buttons, to->buttons, 16);
    writeDeltaKey(key, from->weapon, to->weapon, 8);

    writeDeltaKey(key, from->forcesel, to->forcesel, 8);
    writeDeltaKey(key, from->invensel, to->invensel, 8);

    writeDeltaKey(key, from->generic_cmd, to->generic_cmd, 8);
}

void CompressedMessage::readDeltaEntity(const JKA::entityState_t *from, JKA::entityState_t *to, int number)
{
    assert(from != nullptr);
    assert(to != nullptr);
    assert(number >= 0 && number < JKA::MAX_GENTITIES);

    constexpr size_t TOTAL_FIELDS = JKA::entityStateFields.size();

    // check for a remove
    if (readBits(1) == 1) {
        std::memset(to, 0, sizeof(*to));
        to->number = JKA::MAX_GENTITIES - 1;
        return;
    }

    to->number = number;

    // check for no delta
    if (readBits(1) == 0) {
        *to = *from;
        return;
    }

    const JKA::netField_t *field = &JKA::entityStateFields[0];
    size_t fieldsCount = readByte();

    for (size_t i = 0; i < fieldsCount; i++, field++) {
        const int32_t *fromF = reinterpret_cast<const int32_t *>(reinterpret_cast<const uint8_t *>(from) + field->offset);
        int32_t *toF = reinterpret_cast<int32_t *>(reinterpret_cast<uint8_t *>(to) + field->offset);

        if (!readBits(1)) {
            // no change
            *toF = *fromF;
        } else {
            if (field->bits == 0) {
                // float
                if (readBits(1) == 0) {
                    *(float *)toF = 0.0f;
                } else {
                    if (readBits(1) == 0) {
                        // integral float
                        int32_t trunc = readBits(JKA::FLOAT_INT_BITS);
                        // bias to allow equal parts positive and negative
                        trunc -= JKA::FLOAT_INT_BIAS;
                        *reinterpret_cast<float *>(toF) = static_cast<float>(trunc);  // TODO: check if this is correct
                    } else {
                        // full floating point value
                        *toF = readBits(32);
                    }
                }
            } else {
                if (readBits(1) == 0) {
                    *toF = 0;
                } else {
                    // integer
                    *toF = readBits(field->bits);
                }
            }
        }
    }

    // other fields are not changed
    field = &JKA::entityStateFields[fieldsCount];
    for (size_t i = fieldsCount; i < TOTAL_FIELDS; i++, field++) {
        const int32_t *fromF = reinterpret_cast<const int32_t *>(reinterpret_cast<const uint8_t *>(from) + field->offset);
        int32_t *toF = reinterpret_cast<int32_t *>(reinterpret_cast<uint8_t *>(to) + field->offset);
        *toF = *fromF;
    }
}

void CompressedMessage::writeDeltaEntity(const JKA::entityState_t *from, const JKA::entityState_t *to, bool force)
{
    int            trunc;
    float        fullFloat;

    constexpr size_t numFields = sizeof(JKA::entityStateFields) / sizeof(JKA::entityStateFields[0]);

    // all fields should be 32 bits to avoid any compiler packing issues
    // the "number" field is not part of the field list
    // if this assert fails, someone added a field to the entityState_t
    // struct without updating the message fields
    static_assert(numFields + 1 == sizeof(*from) / 4, "Bad alignment");

    // a NULL to is a delta remove message
    if (to == nullptr) {
        if (from == nullptr) {
            return;
        }
        writeBits(from->number, JKA::GENTITYNUM_BITS);
        writeBits(1, 1);
        return;
    }

    if (to->number < 0 || to->number >= JKA::MAX_GENTITIES) {
        // TODO: replace with a logger
        std::cerr << "MSG_WriteDeltaEntity: Bad entity number: " << to->number << std::endl;
        return;
    }

    int32_t lc = 0;
    const int32_t *fromF = nullptr;
    int32_t *toF = nullptr;
    // build the change vector as bytes so it is endien independent
    // TODO: OPTIMIZE: How about we do this in reverse order so we can
    // just break out at the first changed field we find?
    for (int i = 0; i < static_cast<int>(numFields); i++) {
        fromF = (const int32_t *)((const uint8_t *)from + JKA::entityStateFields[i].offset);
        toF = (int32_t *)((uint8_t *)to + JKA::entityStateFields[i].offset);
        if (*fromF != *toF) {
            lc = i + 1;
        }
    }

    if (lc == 0) {
        // nothing at all changed
        if (!force) {
            return;        // nothing at all
        }
        // write two bits for no change
        writeBits(to->number, JKA::GENTITYNUM_BITS);
        writeBits(0, 1);        // not removed
        writeBits(0, 1);        // no delta
        return;
    }

    writeBits(to->number, JKA::GENTITYNUM_BITS);
    writeBits(0, 1);            // not removed
    writeBits(1, 1);            // we have a delta

    writeByte(lc);    // # of changes

    for (int i = 0; i < lc; i++) {
        fromF = (const int32_t *)((const uint8_t *)from + JKA::entityStateFields[i].offset);
        toF = (int32_t *)((uint8_t *)to + JKA::entityStateFields[i].offset);
        if (*fromF == *toF) {
            writeBits(0, 1);    // no change
            continue;
        }

        writeBits(1, 1);    // changed

        if (JKA::entityStateFields[i].bits == 0) {
            // float
            fullFloat = *(float *)toF;
            trunc = (int)fullFloat;

            if (fullFloat == 0.0f) {
                writeBits(0, 1);
            }
            else {
                writeBits(1, 1);
                if (trunc == fullFloat && trunc + (int32_t)JKA::FLOAT_INT_BIAS >= 0 &&
                    trunc + (int)JKA::FLOAT_INT_BIAS < (1 << JKA::FLOAT_INT_BITS)) {
                    // send as small integer
                    writeBits(0, 1);
                    writeBits(trunc + JKA::FLOAT_INT_BIAS, JKA::FLOAT_INT_BITS);
                }
                else {
                    // send as full floating point value
                    writeBits(1, 1);
                    writeBits(*toF, 32);
                }
            }
        }
        else {
            if (*toF == 0) {
                writeBits(0, 1);
            }
            else {
                writeBits(1, 1);
                // integer
                writeBits(*toF, JKA::entityStateFields[i].bits);
            }
        }
    }
}

void CompressedMessage::readDeltaPlayerstate(const JKA::playerState_t *from, JKA::playerState_t *to, bool isVehiclePS)
{
    const JKA::netField_t *field = nullptr;
    const JKA::netField_t *PSFields = JKA::playerStateFields.data();
    JKA::playerState_t dummy = {};

    if (!from) {
        from = &dummy;
    }
    *to = *from;

    size_t numFields = 0;
    if (isVehiclePS) {  // a vehicle playerstate
        numFields = JKA::vehPlayerStateFields.size();
        PSFields = JKA::vehPlayerStateFields.data();
    } else {
        int32_t isPilot = readBits(1);
        if (isPilot) {  // pilot riding *inside* a vehicle!
            // Skinpack: I have absolutely no idea what 82 is.
            numFields = JKA::pilotPlayerStateFields.size() - 82;
            PSFields = JKA::pilotPlayerStateFields.data();
        } else {  // normal client
            numFields = JKA::playerStateFields.size();
        }
    }

    size_t lc = readByte();

    field = PSFields;
    for (size_t i = 0; i < lc; i++, field++) {
        const int32_t *fromF = reinterpret_cast<const int32_t *>(reinterpret_cast<const uint8_t *>(from) + field->offset);
        int32_t *toF = reinterpret_cast<int32_t *>(reinterpret_cast<uint8_t *>(to) + field->offset);

        if (!readBits(1)) {
            // no change
            *toF = *fromF;
        } else {
            if (field->bits == 0) {
                // float
                if (readBits(1) == 0) {
                    // integral float
                    int32_t trunc = readBits(JKA::FLOAT_INT_BITS);
                    // bias to allow equal parts positive and negative
                    trunc -= JKA::FLOAT_INT_BIAS;
                    *reinterpret_cast<float *>(toF) = static_cast<float>(trunc);
                } else {
                    // full floating point value
                    *toF = readBits(32);
                }
            } else {
                // integer
                *toF = readBits(field->bits);
            }
        }
    }

    field = &PSFields[lc];
    for (size_t i = lc; i < numFields; i++, field++) {
        const int32_t *fromF = reinterpret_cast<const int32_t *>(reinterpret_cast<const uint8_t *>(from) + field->offset);
        int32_t *toF = reinterpret_cast<int32_t *>(reinterpret_cast<uint8_t *>(to) + field->offset);
        // no change
        *toF = *fromF;
    }

    // read the arrays
    if (readBits(1)) {
        // parse stats
        if (readBits(1)) {
            int32_t bits = readShort();
            for (size_t i = 0; i < 16; i++) {
                if (bits & (1 << i)) {
                    if (i == JKA::STAT_WEAPONS) { // ugly.. but we're gonna need it anyway -rww
                        to->stats[i] = readBits(JKA::MAX_WEAPONS);
                    } else {
                        to->stats[i] = readShort();
                    }
                }
            }
        }

        // parse persistant stats
        if (readBits(1)) {
            int32_t bits = readShort();
            for (size_t i = 0; i < 16; i++) {
                if (bits & (1 << i)) {
                    to->persistant[i] = readShort();
                }
            }
        }

        // parse ammo
        if (readBits(1)) {
            int32_t bits = readShort();
            for (size_t i = 0; i < 16; i++) {
                if (bits & (1 << i)) {
                    to->ammo[i] = readShort();
                }
            }
        }
        // parse powerups
        if (readBits(1)) {
            int32_t bits = readShort();
            for (size_t i = 0; i < 16; i++) {
                if (bits & (1 << i)) {
                    to->powerups[i] = readLong();
                }
            }
        }
    }
}

void CompressedMessage::writeDeltaPlayerstate(const JKA::playerState_t *from, const JKA::playerState_t *to, bool isVehiclePS)
{
    int32_t                i;
    JKA::playerState_t     dummy;
    int32_t                statsbits;
    int32_t                persistantbits;
    int32_t                ammobits;
    int32_t                powerupbits;
    int32_t                numFields;
    const JKA::netField_t *PSFields = JKA::playerStateFields.data();
    float                  fullFloat;
    int32_t                trunc;

    if (!from) {
        from = &dummy;
        memset(&dummy, 0, sizeof(dummy));
    }

    if (isVehiclePS) {//a vehicle playerstate
        numFields = static_cast<int>(JKA::vehPlayerStateFields.size());
        PSFields = JKA::vehPlayerStateFields.data();
    }
    else {//regular client playerstate
        if (to->m_iVehicleNum
            && (to->eFlags & JKA::EF_NODRAW)) {//pilot riding *inside* a vehicle!
            writeBits(1, 1);    // Pilot player state
            numFields = static_cast<int>(JKA::pilotPlayerStateFields.size()) - 82;
            PSFields = JKA::pilotPlayerStateFields.data();
        }
        else {//normal client
            writeBits(0, 1);    // Normal player state
            numFields = static_cast<int>(JKA::playerStateFields.size());
        }
    }

    const JKA::netField_t *field;
    int32_t lc = 0;
    const int32_t *fromF;
    int32_t *toF;
    for (i = 0, field = PSFields; i < numFields; i++, field++) {
        fromF = (const int32_t *)((const uint8_t *)from + field->offset);
        toF = (int32_t *)((uint8_t *)to + field->offset);
        if (*fromF != *toF) {
            lc = i + 1;
        }
    }

    writeByte(lc);    // # of changes

    for (i = 0, field = PSFields; i < lc; i++, field++) {
        fromF = (const int32_t *)((const uint8_t *)from + field->offset);
        toF = (int32_t *)((uint8_t *)to + field->offset);

        if (*fromF == *toF) {
            writeBits(0, 1);    // no change
            continue;
        }

        writeBits(1, 1);    // changed

        if (field->bits == 0) {
            // float
            fullFloat = *(float *)toF;
            trunc = (int32_t)fullFloat;

            if (trunc == fullFloat && trunc + (int32_t)JKA::FLOAT_INT_BIAS >= 0 &&
                trunc + JKA::FLOAT_INT_BIAS < (1 << JKA::FLOAT_INT_BITS)) {
                // send as small integer
                writeBits(0, 1);
                writeBits(trunc + JKA::FLOAT_INT_BIAS, JKA::FLOAT_INT_BITS);
            }
            else {
                // send as full floating point value
                writeBits(1, 1);
                writeBits(*toF, 32);
            }
        }
        else {
            // integer
            writeBits(*toF, field->bits);
        }
    }

    //
    // send the arrays
    //
    statsbits = 0;
    for (i = 0; i < 16; i++) {
        if (to->stats[i] != from->stats[i]) {
            statsbits |= 1 << i;
        }
    }
    persistantbits = 0;
    for (i = 0; i < 16; i++) {
        if (to->persistant[i] != from->persistant[i]) {
            persistantbits |= 1 << i;
        }
    }
    ammobits = 0;
    for (i = 0; i < 16; i++) {
        if (to->ammo[i] != from->ammo[i]) {
            ammobits |= 1 << i;
        }
    }
    powerupbits = 0;
    for (i = 0; i < 16; i++) {
        if (to->powerups[i] != from->powerups[i]) {
            powerupbits |= 1 << i;
        }
    }

    if (!statsbits && !persistantbits && !ammobits && !powerupbits) {
        writeBits(0, 1);    // no change
        return;
    }
    writeBits(1, 1);    // changed

    if (statsbits) {
        writeBits(1, 1);    // changed
        writeShort(statsbits);
        for (i = 0; i < 16; i++) {
            if (statsbits & (1 << i)) {
                if (i == JKA::STAT_WEAPONS) { //ugly.. but we're gonna need it anyway -rww
                  //(just send this one in MAX_WEAPONS bits, so that we can add up to MAX_WEAPONS weaps without hassle)
                    writeBits(to->stats[i], JKA::MAX_WEAPONS);
                }
                else {
                    writeShort(to->stats[i]);
                }
            }
        }
    }
    else {
        writeBits(0, 1);    // no change
    }

    if (persistantbits) {
        writeBits(1, 1);    // changed
        writeShort(persistantbits);
        for (i = 0; i < 16; i++)
            if (persistantbits & (1 << i))
                writeShort(to->persistant[i]);
    }
    else {
        writeBits(0, 1);    // no change
    }

    if (ammobits) {
        writeBits(1, 1);    // changed
        writeShort(ammobits);
        for (i = 0; i < 16; i++)
            if (ammobits & (1 << i))
                writeShort(to->ammo[i]);
    }
    else {
        writeBits(0, 1);    // no change
    }

    if (powerupbits) {
        writeBits(1, 1);    // changed
        writeShort(powerupbits);
        for (i = 0; i < 16; i++)
            if (powerupbits & (1 << i))
                writeLong(to->powerups[i]);
    }
    else {
        writeBits(0, 1);    // no change
    }
}

CompressedMessage::InternalState CompressedMessage::saveState() const
{
    return { dataStream.getLocation(), readcount, bit, cursize, maxSize, overflowed };
}

void CompressedMessage::resetReadState()
{
    dataStream.setLocation(0);
    readcount = 0;
    bit = 0;
}

void CompressedMessage::restoreState(const InternalState & state)
{
    dataStream.setLocation(state.streamLoc);
    readcount = state.readcount;
    bit = state.bit;
    cursize = state.cursize;
    maxSize = state.maxSize;
    overflowed = state.overflowed;
}

int32_t CompressedMessage::float_to_int(float val) const noexcept
{
    static_assert(sizeof(float) == sizeof(int32_t));

    alignas(float) int32_t res{};
    std::memcpy(&res, &val, sizeof(float));
    return res;
}

float CompressedMessage::int_to_float(int32_t val) const noexcept
{
    static_assert(sizeof(float) == sizeof(int32_t));

    alignas(int32_t) float res{};
    std::memcpy(&res, &val, sizeof(int32_t));
    return res;
}
