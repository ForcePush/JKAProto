#include <JKAProto/protocol/CompressedMessage.h>

#include <cassert>
#include <iostream>
#include <sstream>

#include <JKAProto/utility/BitCast.h>

namespace JKA::Protocol {
    using Utility::bit_cast;

    // ************************** WRITE **************************
    void CompressedMessage::writeString(std::string_view sv) noexcept
    {
        if (sv.empty()) {
            writeData(Utility::Span("\x00"));
        } else {
            writeData(Utility::Span(sv));
            writeData(Utility::Span("\x00"));  // string_view does not include the terminating null char
        }
    }

    void CompressedMessage::writeData(Utility::Span<const ByteType> data) noexcept
    {
        for (const auto & c : data) {
            writeByte(c);
        }
    }

    // ************************** READ **************************
    void CompressedMessage::readData(Utility::Span<ByteType> data) noexcept
    {
        for (auto & c : data) {
            c = readByte();
        }
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
        } while (l <= MAX_BIG_STRING - 1);

        return ss.str();
    }

    std::string CompressedMessage::readStringLine(bool translatePercent)
    {
        return readString(true, translatePercent);
    }

    // ************************** DELTA **************************
    // TODO: rww: get rid of aliasing
    void CompressedMessage::readDeltaEntity(const entityState_t *from, entityState_t *to, int number) noexcept
    {
        assert(from != nullptr);
        assert(to != nullptr);
        assert(number >= 0 && number < MAX_GENTITIES);

        constexpr size_t TOTAL_FIELDS = entityStateFields.size();

        // check for a remove
        if (readBit() == 1) {
            std::memset(to, 0, sizeof(*to));
            to->number = MAX_GENTITIES - 1;
            return;
        }

        to->number = number;

        // check for no delta
        if (readBit() == 0) {
            *to = *from;
            return;
        }

        const netField_t *field = &entityStateFields[0];
        size_t fieldsCount = readByte();

        for (size_t i = 0; i < fieldsCount; i++, field++) {
            const int32_t *fromF = reinterpret_cast<const int32_t *>(reinterpret_cast<const uint8_t *>(from) + field->offset);
            int32_t *toF = reinterpret_cast<int32_t *>(reinterpret_cast<uint8_t *>(to) + field->offset);

            if (!readBit()) {
                // no change
                *toF = *fromF;
            } else {
                if (field->bits == 0) {
                    // float
                    if (readBit() == 0) {
                        *(float *)toF = 0.0f;
                    } else {
                        if (readBit() == 0) {
                            // integral float
                            int32_t trunc = readBits<FLOAT_INT_BITS>();
                            // bias to allow equal parts positive and negative
                            trunc -= FLOAT_INT_BIAS;
                            *reinterpret_cast<float *>(toF) = static_cast<float>(trunc);
                        } else {
                            // full floating point value
                            *toF = readBits<32>();
                        }
                    }
                } else {
                    if (readBit() == 0) {
                        *toF = 0;
                    } else {
                        *toF = readBitsVariable(field->bits);
                    }
                }
            }
        }

        // other fields are not changed
        field = &entityStateFields[fieldsCount];
        for (size_t i = fieldsCount; i < TOTAL_FIELDS; i++, field++) {
            const int32_t *fromF = reinterpret_cast<const int32_t *>(reinterpret_cast<const uint8_t *>(from) + field->offset);
            int32_t *toF = reinterpret_cast<int32_t *>(reinterpret_cast<uint8_t *>(to) + field->offset);
            *toF = *fromF;
        }
    }

    // TODO: rww: get rid of aliasing
    void CompressedMessage::writeDeltaEntity(const entityState_t *from, const entityState_t *to, bool force) noexcept
    {
        int32_t trunc;
        float fullFloat;

        constexpr size_t numFields = entityStateFields.size();

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
            writeBits<GENTITYNUM_BITS>(from->number);
            writeBit(1);
            return;
        }

        if (to->number < 0 || to->number >= MAX_GENTITIES) {
            // TODO: replace with a logger
            std::cerr << "MSG_WriteDeltaEntity: Bad entity number: " << to->number << std::endl;
            return;
        }

        int32_t lc = 0;
        const int32_t *fromF = nullptr;
        int32_t *toF = nullptr;
        // build the change vector as bytes so it is endien independent
        for (int i = 0; i < static_cast<int>(numFields); i++) {
            fromF = (const int32_t *)((const uint8_t *)from + entityStateFields[i].offset);
            toF = (int32_t *)((uint8_t *)to + entityStateFields[i].offset);
            if (*fromF != *toF) {
                lc = i + 1;
            }
        }

        if (lc == 0) {
            // nothing at all changed
            if (!force) {
                return; // nothing at all
            }
            // write two bits for no change
            writeBits<GENTITYNUM_BITS>(to->number);
            writeBit(0);        // not removed
            writeBit(0);        // no delta
            return;
        }

        writeBits<GENTITYNUM_BITS>(to->number);
        writeBit(0);            // not removed
        writeBit(1);            // we have a delta

        writeByte(lc);    // # of changes

        for (int32_t i = 0; i < lc; i++) {
            fromF = (const int32_t *)((const uint8_t *)from + entityStateFields[i].offset);
            toF = (int32_t *)((uint8_t *)to + entityStateFields[i].offset);
            if (*fromF == *toF) {
                writeBit(0);    // no change
                continue;
            }

            writeBit(1);    // changed

            if (entityStateFields[i].bits == 0) {
                // float
                fullFloat = *(float *)toF;
                trunc = (int)fullFloat;

                if (fullFloat == 0.0f) {
                    writeBit(0);
                } else {
                    writeBit(1);
                    if (trunc == fullFloat && trunc + (int32_t)FLOAT_INT_BIAS >= 0 &&
                        trunc + (int)FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS)) {
                        // send as small integer
                        writeBit(0);
                        writeBits<FLOAT_INT_BITS>(trunc + FLOAT_INT_BIAS);
                    } else {
                        // send as full floating point value
                        writeBit(1);
                        writeBits<32>(*toF);
                    }
                }
            }         else {
                if (*toF == 0) {
                    writeBit(0);
                } else {
                    writeBit(1);
                    writeBitsVariable(*toF, entityStateFields[i].bits);
                }
            }
        }
    }

    void CompressedMessage::readDeltaPlayerstate(const playerState_t *from, playerState_t *to, bool isVehiclePS) noexcept
    {
        const netField_t *field = nullptr;
        const netField_t *PSFields = playerStateFields.data();
        playerState_t dummy = {};

        if (!from) {
            from = &dummy;
        }
        *to = *from;

        size_t numFields = 0;
        if (isVehiclePS) {  // a vehicle playerstate
            numFields = vehPlayerStateFields.size();
            PSFields = vehPlayerStateFields.data();
        } else {
            int32_t isPilot = readBit();
            if (isPilot) {  // pilot riding *inside* a vehicle!
                // Skinpack: I have absolutely no idea what 82 is.
                numFields = pilotPlayerStateFields.size() - 82;
                PSFields = pilotPlayerStateFields.data();
            } else {  // normal client
                numFields = playerStateFields.size();
            }
        }

        size_t lc = readByte();

        field = PSFields;
        for (size_t i = 0; i < lc; i++, field++) {
            const int32_t *fromF = reinterpret_cast<const int32_t *>(reinterpret_cast<const uint8_t *>(from) + field->offset);
            int32_t *toF = reinterpret_cast<int32_t *>(reinterpret_cast<uint8_t *>(to) + field->offset);

            if (!readBit()) {
                // no change
                *toF = *fromF;
            } else {
                if (field->bits == 0) {
                    // float
                    if (readBit() == 0) {
                        // integral float
                        int32_t trunc = readBits<FLOAT_INT_BITS>();
                        // bias to allow equal parts positive and negative
                        trunc -= FLOAT_INT_BIAS;
                        *reinterpret_cast<float *>(toF) = static_cast<float>(trunc);
                    } else {
                        // full floating point value
                        *toF = readBits<32>();
                    }
                } else {
                    // integer
                    *toF = readBitsVariable(field->bits);
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
        if (readBit()) {
            // parse stats
            if (readBit()) {
                int32_t bits = readShort();
                for (size_t i = 0; i < 16; i++) {
                    if (bits & (1 << i)) {
                        if (i == STAT_WEAPONS) { // ugly.. but we're gonna need it anyway -rww
                            to->stats[i] = readBits<MAX_WEAPONS>();
                        } else {
                            to->stats[i] = readShort();
                        }
                    }
                }
            }

            // parse persistant stats
            if (readBit()) {
                int32_t bits = readShort();
                for (size_t i = 0; i < 16; i++) {
                    if (bits & (1 << i)) {
                        to->persistant[i] = readShort();
                    }
                }
            }

            // parse ammo
            if (readBit()) {
                int32_t bits = readShort();
                for (size_t i = 0; i < 16; i++) {
                    if (bits & (1 << i)) {
                        to->ammo[i] = readShort();
                    }
                }
            }
            // parse powerups
            if (readBit()) {
                int32_t bits = readShort();
                for (size_t i = 0; i < 16; i++) {
                    if (bits & (1 << i)) {
                        to->powerups[i] = readLong();
                    }
                }
            }
        }
    }

    void CompressedMessage::writeDeltaPlayerstate(const playerState_t *from, const playerState_t *to, bool isVehiclePS) noexcept
    {
        int32_t                i;
        playerState_t     dummy;
        int32_t                statsbits;
        int32_t                persistantbits;
        int32_t                ammobits;
        int32_t                powerupbits;
        int32_t                numFields;
        const netField_t *PSFields = playerStateFields.data();
        float                  fullFloat;
        int32_t                trunc;

        if (!from) {
            from = &dummy;
            memset(&dummy, 0, sizeof(dummy));
        }

        if (isVehiclePS) {  //a vehicle playerstate
            numFields = static_cast<int>(vehPlayerStateFields.size());
            PSFields = vehPlayerStateFields.data();
        } else {  //regular client playerstate
            if (to->m_iVehicleNum
                && (to->eFlags & EF_NODRAW)) {  //pilot riding *inside* a vehicle!
                writeBit(1);    // Pilot player state
                numFields = static_cast<int>(pilotPlayerStateFields.size()) - 82;
                PSFields = pilotPlayerStateFields.data();
            } else {  //normal client
                writeBit(0);    // Normal player state
                numFields = static_cast<int>(playerStateFields.size());
            }
        }

        const netField_t *field;
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
                writeBit(0);    // no change
                continue;
            }

            writeBit(1);    // changed

            if (field->bits == 0) {
                // float
                fullFloat = *(float *)toF;
                trunc = (int32_t)fullFloat;

                if (trunc == fullFloat && trunc + (int32_t)FLOAT_INT_BIAS >= 0 &&
                    trunc + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS)) {
                    // send as small integer
                    writeBit(0);
                    writeBits<FLOAT_INT_BITS>(trunc + FLOAT_INT_BIAS);
                }             else {
                    // send as full floating point value
                    writeBit(1);
                    writeBits<32>(*toF);
                }
            } else {
                writeBitsVariable(*toF, field->bits);
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
            writeBit(0);    // no change
            return;
        }
        writeBit(1);    // changed

        if (statsbits) {
            writeBit(1);    // changed
            writeShort(statsbits);
            for (i = 0; i < 16; i++) {
                if (statsbits & (1 << i)) {
                    if (i == STAT_WEAPONS) { //ugly.. but we're gonna need it anyway -rww
                      //(just send this one in MAX_WEAPONS bits, so that we can add up to MAX_WEAPONS weaps without hassle)
                        writeBits<MAX_WEAPONS>(to->stats[i]);
                    } else {
                        writeShort(to->stats[i]);
                    }
                }
            }
        } else {
            writeBit(0);    // no change
        }

        if (persistantbits) {
            writeBit(1);    // changed
            writeShort(persistantbits);
            for (i = 0; i < static_cast<int32_t>(std::size(to->persistant)); i++) {
                if (persistantbits & (1 << i)) {
                    writeShort(to->persistant[i]);
                }
            }
        } else {
            writeBit(0);    // no change
        }

        if (ammobits) {
            writeBit(1);    // changed
            writeShort(ammobits);
            for (i = 0; i < static_cast<int32_t>(std::size(to->ammo)); i++) {
                if (ammobits & (1 << i)) {
                    writeShort(to->ammo[i]);
                }
            }
        } else {
            writeBit(0);    // no change
        }

        if (powerupbits) {
            writeBit(1);    // changed
            writeShort(powerupbits);
            for (i = 0; i < static_cast<int32_t>(std::size(to->powerups)); i++) {
                if (powerupbits & (1 << i)) {
                    writeLong(to->powerups[i]);
                }
            }
        } else {
            writeBit(0);    // no change
        }
    }
}
