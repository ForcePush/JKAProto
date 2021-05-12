#pragma once
#include <climits>
#include <string>
#include <string_view>

#include "../Huffman.h"
#include "../jka/JKADefs.h"
#include "../jka/JKAStructs.h"
#include "../jka/Usercmd.h"
#include "../utility/BitCast.h"
#include "../utility/Span.h"
#include "../SharedDefs.h"

namespace JKA::Protocol {
    class CompressedMessage {
    public:
        struct InternalState {
            size_t streamLoc = 0;

            size_t readcount = 0;
            size_t bit = 0;
            size_t cursize = 0;

            size_t maxSize = 0;
            bool overflowed = false;
        };

        constexpr CompressedMessage(Q3Huffman & huffman) noexcept :
            huff(std::addressof(huffman))
        {
        }

        constexpr CompressedMessage(Q3Huffman & huffman, ByteType *buf, size_t bufBytesPresent, size_t maxSize) noexcept :
            cursize(bufBytesPresent),
            bit(0),
            readcount(0),
            maxSize(maxSize),
            overflowed(false),
            dataBuf(buf),
            huff(std::addressof(huffman))
        {
            dataStream = WriteableBitStream(buf);
        }

        constexpr CompressedMessage(Q3Huffman & huffman, Utility::Span<ByteType> buffer) noexcept :
            CompressedMessage(huffman, buffer.data(), buffer.size(), buffer.size())
        {
        }

        constexpr CompressedMessage(const CompressedMessage & other) noexcept = default;
        constexpr CompressedMessage(CompressedMessage && other) noexcept = default;
        constexpr CompressedMessage & operator=(const CompressedMessage & other) noexcept = default;
        constexpr CompressedMessage & operator=(CompressedMessage && other) noexcept = default;

        [[nodiscard]] constexpr Utility::Span<ByteType> to_span() & noexcept
        {
            return Utility::Span<ByteType>(dataBuf, cursize);
        }

        [[nodiscard]] constexpr Utility::Span<const ByteType> to_span() const & noexcept
        {
            return Utility::Span<const ByteType>(dataBuf, cursize);
        }

        WriteableBitStream dataStream{ nullptr };

        size_t cursize = 0;
        size_t bit = 0;
        size_t readcount = 0;

        size_t maxSize = 0;
        bool overflowed = false;

        // ************************** WRITE **************************
        constexpr void writeBitsVariable(int32_t val, int32_t bits) noexcept
        {
            // this isn't an exact overflow check, but close enough
            if (maxSize - cursize < 4) JKA_UNLIKELY{
                overflowed = true;
                return;
            }

            val &= (0xffffffff >> (32 - bits));
            if (bits & 7) {
                int32_t nbits = bits & 0b111;
                for (int32_t i = 0; i < nbits; i++) {
                    dataStream.putBit((val & 1), &bit);
                    val >>= 1;
                }
                bits -= nbits;
            }

            if (bits) {
                for (int32_t i = 0; i < bits; i += 8) {
                    huff->offsetTransmit<Huffman::HUFF_COMPRESS>((val & 0xff), dataStream, &bit);
                    val = (val >> 8);
                }
            }
            cursize = (bit >> 3) + 1;
        }

        template<int32_t Bits>
        constexpr void writeBits(int32_t val) noexcept
        {
            static_assert(Bits > 0 && Bits <= 32);

            // this isn't an exact overflow check, but close enough
            if (maxSize - cursize < 4) JKA_UNLIKELY {
                overflowed = true;
                return;
            }

            int32_t bits = Bits;
            constexpr int32_t nbits = Bits & 0b111;
            val &= (0xffffffff >> (32 - Bits));
            if constexpr (nbits != 0) {
                for (int32_t i = 0; i < nbits; i++) {
                    dataStream.putBit((val & 1), &bit);
                    val >>= 1;
                }
                bits -= nbits;
            }

            if (bits) {
                for (int32_t i = 0; i < bits; i += 8) {
                    huff->offsetTransmit<Huffman::HUFF_COMPRESS>((val & 0xff), dataStream, &bit);
                    val = (val >> 8);
                }
            }
            cursize = (bit >> 3) + 1;
        }

        constexpr void writeBit(int32_t val) noexcept
        {
            writeBits<1>(val & 1);
        }

        constexpr void writeLong(int32_t val) noexcept
        {
            writeBits<32>(val);
        }

        constexpr void writeShort(int32_t val) noexcept
        {
            writeBits<16>(val);
        }

        constexpr void writeByte(int32_t val) noexcept
        {
            writeBits<8>(val);
        }

        void writeFloat(float val) noexcept
        {
            writeBits<32>(Utility::bit_cast<int32_t>(val));
        }

        void writeString(std::string_view sv) noexcept;
        void writeData(Utility::Span<const ByteType> data) noexcept;

        // ************************** READ **************************
        constexpr int32_t readBitsVariable(int32_t bits) noexcept
        {
            int32_t value = 0;
            int32_t get = 0;

            bool sgn = false;
            if (bits < 0) {
                sgn = true;
                bits = -bits;
            }

            int32_t nbits = bits & 0b111;
            if (nbits) {
                for (int32_t i = 0; i < nbits; i++) {
                    value |= (dataStream.getBitOffset(&bit) << i);
                }
                bits -= nbits;
            }

            if (bits) JKA_LIKELY {
                for (int32_t i = 0; i < bits; i += 8) {
                    huff->offsetReceive<Huffman::HUFF_DECOMPRESS>(&get, dataStream, &bit);
                    value |= (get << (i + nbits));
                }
            }

            if (sgn) {
                if (value & (1 << (bits - 1))) {
                    // TODO: replace with a more adequate expression
                    value |= -1 ^ ((1 << bits) - 1);
                }
            }

            readcount = (bit >> 3) + 1;
            return value;
        }

        template<int32_t Bits>
        constexpr int32_t readBits() noexcept
        {
            static_assert(Bits > 0);

            int32_t value = 0;
            int32_t get = 0;

            int32_t bits = Bits;

            constexpr int32_t nbits = Bits & 0b111;
            if constexpr (nbits != 0) {
                for (size_t i = 0; i < nbits; i++) {
                    value |= (dataStream.getBitOffset(&bit) << i);
                }
                bits -= nbits;
            }

            if (bits) JKA_LIKELY {
                for (int32_t i = 0; i < bits; i += 8) {
                    huff->offsetReceive<Huffman::HUFF_DECOMPRESS>(&get, dataStream, &bit);
                    value |= (get << (i + nbits));
                }
            }

            readcount = (bit >> 3) + 1;
            return value;
        }

        constexpr int32_t readBit() noexcept
        {
            return readBits<1>();
        }

        constexpr int32_t readLong() noexcept
        {
            int32_t c = readBits<32>();
            if (readcount > cursize) JKA_UNLIKELY {
                c = -1;
            }

            return c;
        }

        constexpr int16_t readShort() noexcept
        {
            int16_t c = static_cast<int16_t>(readBits<16>());
            if (readcount > cursize) JKA_UNLIKELY {
                c = -1;
            }
            return c;
        }

        constexpr uint16_t readUShort() noexcept
        {
            return static_cast<uint16_t>(readShort());
        }

        constexpr uint8_t readByte(bool readSigned = false) noexcept
        {
            int32_t c = 0;

            if (readSigned) {
                c = static_cast<int8_t>(readBits<8>());
            } else {
                c = static_cast<uint8_t>(readBits<8>());
            }
            if (readcount > cursize) JKA_UNLIKELY {
                c = -1;
            }
            return static_cast<uint8_t>(c);
        }

        float readFloat() noexcept
        {
            int32_t val = readBits<32>();

            if (readcount > cursize) JKA_UNLIKELY {
                return -1;
            }

            return Utility::bit_cast<float>(val);
        }

        void readData(Utility::Span<ByteType> data) noexcept;
        std::string readString(bool breakOnNewline = false, bool translatePercent = false);
        std::string readStringLine(bool translatePercent = false);

        constexpr int32_t peekLong() noexcept
        {
            auto state = saveState();
            int32_t result = readLong();
            restoreState(state);
            return result;
        }

        // ************************** OOB **************************
        int32_t peekLongOOB() noexcept
        {
            return Utility::bit_reinterpret<int32_t>(dataBuf + readcount);
        }

        template<int32_t Bits>
        int32_t readOOB() noexcept
        {
            static_assert(Bits == 32 ||
                          Bits == 16 ||
                          Bits == 8);

            int32_t res = -1;
            if constexpr (Bits == 32) {
                res = Utility::bit_reinterpret<int32_t>(dataBuf + readcount);
                readcount += 4;
                bit += 32;
            } else if constexpr (Bits == 16) {
                res = Utility::bit_reinterpret<int16_t>(dataBuf + readcount);
                readcount += 2;
                bit += 16;
            } else if constexpr (Bits == 8) {
                res = Utility::bit_reinterpret<int8_t>(dataBuf + readcount);
                readcount += 1;
                bit += 8;
            }

            dataStream.setLocation(dataStream.getLocation() + Bits);
            return res;
        }

        template<int32_t Bits>
        constexpr void writeOOB(int32_t value) noexcept
        {
            static_assert(Bits == 32 ||
                          Bits == 16 ||
                          Bits == 8);
            for (size_t i = 0; i < Bits / CHAR_BIT; i++) {
                dataBuf[cursize] = (value >> (i * CHAR_BIT)) & 0xFF;
                cursize += 1;
                bit += CHAR_BIT;
            }

            dataStream.setLocation(dataStream.getLocation() + Bits);
        }

        // ************************** READ/WRITE KEY **************************
    private:
        static constexpr std::array<uint32_t, 32> kbitmask {
            0x00000001, 0x00000003, 0x00000007, 0x0000000F,
            0x0000001F,    0x0000003F,    0x0000007F,    0x000000FF,
            0x000001FF,    0x000003FF,    0x000007FF,    0x00000FFF,
            0x00001FFF,    0x00003FFF,    0x00007FFF,    0x0000FFFF,
            0x0001FFFF,    0x0003FFFF,    0x0007FFFF,    0x000FFFFF,
            0x001FFFFf,    0x003FFFFF,    0x007FFFFF,    0x00FFFFFF,
            0x01FFFFFF,    0x03FFFFFF,    0x07FFFFFF,    0x0FFFFFFF,
            0x1FFFFFFF,    0x3FFFFFFF,    0x7FFFFFFF,    0xFFFFFFFF,
        };

    public:
        template<int32_t Bits>
        constexpr void writeDeltaKey(int32_t key, int32_t oldV, int32_t newV) noexcept
        {
            static_assert(Bits > 0 && Bits <= 32);

            if (oldV == newV) {
                writeBit(0);
                return;
            }
            writeBit(1);
            writeBits<Bits>((newV ^ key) & ((1LL << Bits) - 1));
        }

        template<int32_t Bits>
        constexpr int32_t readDeltaKey(int32_t key, int32_t oldV) noexcept
        {
            static_assert(Bits > 0 && Bits <= 32);

            if (readBits<1>()) {
                return readBits<Bits>() ^ (key & kbitmask[Bits]);
            }
            return oldV;
        }

        void writeDeltaKeyFloat(int32_t key, float oldV, float newV) noexcept
        {
            if (oldV == newV) {
                writeBit(0);
                return;
            }
            writeBit(1);
            writeLong(Utility::bit_cast<int32_t>(newV) ^ key);
        }

        float readDeltaKeyFloat(int32_t key, float oldV) noexcept
        {
            if (readBits<1>()) {
                return Utility::bit_cast<float>(readLong() ^ key);
            }
            return oldV;
        }

        // ************************** DELTA **************************
        template<int32_t Bits>
        constexpr void writeDelta(int32_t oldV, int32_t newV) noexcept
        {
            if (oldV == newV) {
                writeBit(0);
                return;
            }
            writeBit(1);
            writeBits<Bits>(newV);
        }

        template<int32_t Bits>
        constexpr int32_t readDelta(int32_t oldV) noexcept
        {
            if (readBits<1>()) {
                return readBits<Bits>();
            }
            return oldV;
        }

        void writeDeltaFloat(float oldV, float newV) noexcept
        {
            if (oldV == newV) {
                writeBit(0);
                return;
            }
            writeBit(1);
            writeLong(Utility::bit_cast<int32_t>(newV));
        }

        float readDeltaFloat(float oldV) noexcept
        {
            if (readBits<1>()) {
                return Utility::bit_cast<float>(readLong());
            }
            return oldV;
        }

        // ************************** DELTA **************************
        constexpr void readDeltaUsercmd(const usercmd_t *from, usercmd_t *to) noexcept
        {
            if (readBit()) JKA_LIKELY{
                to->serverTime = from->serverTime + readByte();
            } else {
                to->serverTime = readLong();
            }

            to->angles[0] = readDelta<16>(from->angles[0]);
            to->angles[1] = readDelta<16>(from->angles[1]);
            to->angles[2] = readDelta<16>(from->angles[2]);
            to->forwardmove = static_cast<int8_t>(readDelta<8>(from->forwardmove));
            to->rightmove = static_cast<int8_t>(readDelta<8>(from->rightmove));
            to->upmove = static_cast<int8_t>(readDelta<8>(from->upmove));
            to->buttons = readDelta<16>(from->buttons);
            to->weapon = static_cast<uint8_t>(readDelta<8>(from->weapon));

            to->forcesel = static_cast<uint8_t>(readDelta<8>(from->forcesel));
            to->invensel = static_cast<uint8_t>(readDelta<8>(from->invensel));

            to->generic_cmd = static_cast<uint8_t>(readDelta<8>(from->generic_cmd));
        }

        constexpr void writeDeltaUsercmd(const usercmd_t *from, const usercmd_t *to) noexcept
        {
            int32_t timeDelta = to->serverTime - from->serverTime;
            if (timeDelta < 256) JKA_LIKELY{
                writeBit(1);
                writeByte(timeDelta);
            } else {
                writeBit(0);
                writeLong(to->serverTime);
            }

            writeDelta<16>(from->angles[0], to->angles[0]);
            writeDelta<16>(from->angles[1], to->angles[1]);
            writeDelta<16>(from->angles[2], to->angles[2]);
            writeDelta<8>(from->forwardmove, to->forwardmove);
            writeDelta<8>(from->rightmove, to->rightmove);
            writeDelta<8>(from->upmove, to->upmove);
            writeDelta<16>(from->buttons, to->buttons);
            writeDelta<8>(from->weapon, to->weapon);

            writeDelta<8>(from->forcesel, to->forcesel);
            writeDelta<8>(from->invensel, to->invensel);

            writeDelta<8>(from->generic_cmd, to->generic_cmd);
        }

        constexpr void readDeltaUsercmdKey(int32_t key, const usercmd_t *from, usercmd_t *to) noexcept
        {
            if (readBit()) JKA_UNLIKELY{
                to->serverTime = from->serverTime + readByte();
            } else {
                to->serverTime = readLong();
            }
            if (readBit()) {
                key ^= to->serverTime;
                to->angles[0] = readDeltaKey<16>(key, from->angles[0]);
                to->angles[1] = readDeltaKey<16>(key, from->angles[1]);
                to->angles[2] = readDeltaKey<16>(key, from->angles[2]);
                to->forwardmove = static_cast<int8_t>(readDeltaKey<8>(key, from->forwardmove));
                to->rightmove = static_cast<int8_t>(readDeltaKey<8>(key, from->rightmove));
                to->upmove = static_cast<int8_t>(readDeltaKey<8>(key, from->upmove));
                to->buttons = readDeltaKey<16>(key, from->buttons);
                to->weapon = static_cast<uint8_t>(readDeltaKey<8>(key, from->weapon));

                to->forcesel = static_cast<uint8_t>(readDeltaKey<8>(key, from->forcesel));
                to->invensel = static_cast<uint8_t>(readDeltaKey<8>(key, from->invensel));

                to->generic_cmd = static_cast<uint8_t>(readDeltaKey<8>(key, from->generic_cmd));
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

        constexpr void writeDeltaUsercmdKey(int32_t key, const usercmd_t *from, const usercmd_t *to) noexcept
        {
            int32_t timeDelta = to->serverTime - from->serverTime;
            if (timeDelta < 256) {
                writeBit(1);
                writeByte(timeDelta);
            } else {
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
                writeBit(0);
                return;
            }
            key ^= to->serverTime;
            writeBit(1);

            writeDeltaKey<16>(key, from->angles[0], to->angles[0]);
            writeDeltaKey<16>(key, from->angles[1], to->angles[1]);
            writeDeltaKey<16>(key, from->angles[2], to->angles[2]);
            writeDeltaKey<8>(key, from->forwardmove, to->forwardmove);
            writeDeltaKey<8>(key, from->rightmove, to->rightmove);
            writeDeltaKey<8>(key, from->upmove, to->upmove);
            writeDeltaKey<16>(key, from->buttons, to->buttons);
            writeDeltaKey<8>(key, from->weapon, to->weapon);

            writeDeltaKey<8>(key, from->forcesel, to->forcesel);
            writeDeltaKey<8>(key, from->invensel, to->invensel);

            writeDeltaKey<8>(key, from->generic_cmd, to->generic_cmd);
        }

        void readDeltaEntity(const entityState_t *from, entityState_t *to, int number) noexcept;
        void writeDeltaEntity(const entityState_t *from, const entityState_t *to, bool force) noexcept;

        void readDeltaPlayerstate(const playerState_t *from, playerState_t *to, bool isVehiclePS = false) noexcept;
        void writeDeltaPlayerstate(const playerState_t *from, const playerState_t *to, bool isVehiclePS) noexcept;

        constexpr InternalState saveState() const noexcept
        {
            return { dataStream.getLocation(), readcount, bit, cursize, maxSize, overflowed };
        }

        constexpr void resetReadState() noexcept
        {
            dataStream.setLocation(0);
            readcount = 0;
            bit = 0;
        }

        constexpr void restoreState(const InternalState & state) noexcept
        {
            dataStream.setLocation(state.streamLoc);
            readcount = state.readcount;
            bit = state.bit;
            cursize = state.cursize;
            maxSize = state.maxSize;
            overflowed = state.overflowed;
        }

    private:
        ByteType *dataBuf = nullptr;
        Q3Huffman *huff;
    };
}
