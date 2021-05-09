#pragma once
#include <string>

#include "Huffman.h"
#include "jka/JKADefs.h"
#include "jka/JKAStructs.h"
#include "jka/Usercmd.h"

namespace JKA {
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

        CompressedMessage(Q3Huffman & huffman);
        CompressedMessage(Q3Huffman & huffman, void *buf, size_t bufSize, size_t maxSize);
        CompressedMessage(CompressedMessage && other) noexcept;
        CompressedMessage & operator=(CompressedMessage && other) noexcept;
        ~CompressedMessage();

        union {
            void *buf;
            char *charBuf;
            unsigned char *ucharBuf;
        } data = { nullptr };
        WriteableByteBitStream dataStream{ nullptr };

        size_t readcount = 0;
        size_t bit = 0;
        size_t cursize = 0;

        size_t maxSize = 0;
        bool overflowed = false;

        template<typename T>
        T *castDataPtr()
        {
            return reinterpret_cast<T *>(data.buf);
        }

        template<typename T>
        const T *castDataPtr() const
        {
            return reinterpret_cast<const T *>(data.buf);
        }

        template<typename T>
        T *castDataPtr(size_t byteOffset)
        {
            return reinterpret_cast<T *>(data.ucharBuf + byteOffset);
        }

        template<typename T>
        const T *castDataPtr(size_t byteOffset) const
        {
            return reinterpret_cast<const T *>(data.ucharBuf + byteOffset);
        }

        void writeBit(int32_t val);
        void writeBits(int32_t val, int32_t bits);
        void writeLong(int32_t val);
        void writeShort(int32_t val);
        void writeByte(int32_t val);
        void writeFloat(float val);
        void writeString(const char *str);
        void writeData(const void *data, size_t dataSize);

        int32_t readBits(int32_t bits);
        int32_t readLong();
        int16_t readShort();
        uint16_t readUShort();
        uint8_t readByte(bool readSigned = false);
        void readData(void *data, size_t len);
        float readFloat();
        std::string readString(bool breakOnNewline = false, bool translatePercent = false);
        std::string readStringLine(bool translatePercent = false);

        int32_t peekLong();
        int32_t peekLongOOB();

        int32_t readOOB(int32_t bits);
        void writeOOB(int32_t value, int32_t bits);

        void writeDeltaKey(int32_t key, int32_t oldV, int32_t newV, int32_t bits);
        int32_t readDeltaKey(int32_t key, int32_t oldV, int32_t bits);
        void writeDeltaKeyFloat(int32_t key, float oldV, float newV);
        float readDeltaKeyFloat(int32_t key, float oldV);

        // ************************** DELTA **************************
        void writeDelta(int32_t oldV, int32_t newV, int32_t bits);
        int32_t readDelta(int32_t oldV, int32_t bits);
        void writeDeltaFloat(float oldV, float newV);
        float readDeltaFloat(float oldV);

        void readDeltaUsercmd(const usercmd_t *from, usercmd_t *to);
        void readDeltaUsercmdKey(int32_t key, const usercmd_t *from, usercmd_t *to);
        void writeDeltaUsercmd(const usercmd_t *from, const usercmd_t *to);
        void writeDeltaUsercmdKey(int32_t key, const usercmd_t *from, const usercmd_t *to);

        void readDeltaEntity(const entityState_t *from, entityState_t *to, int number);
        void writeDeltaEntity(const entityState_t *from, const entityState_t *to, bool force);

        void readDeltaPlayerstate(const playerState_t *from, playerState_t *to, bool isVehiclePS = false);
        void writeDeltaPlayerstate(const playerState_t *from, const playerState_t *to, bool isVehiclePS);


        InternalState saveState() const;
        void resetReadState();
        void restoreState(const InternalState & state);

    private:
        CompressedMessage(const CompressedMessage &) = delete;
        CompressedMessage & operator= (const CompressedMessage &) = delete;

        int32_t float_to_int(float val) const noexcept;
        float int_to_float(int32_t val) const noexcept;

        Q3Huffman & huff;
    };
}
