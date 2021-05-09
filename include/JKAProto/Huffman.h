#pragma once

#include <cinttypes>
#include <cstring>   // memset, memcpy
#include <string>
#include <string_view>
#include <type_traits>

namespace JKA {
    template<typename InputBufferElementType>
    class BitStream {
    public:
        BitStream(InputBufferElementType *inputBuffer, size_t startBloc = 0) :
            bloc(startBloc),
            buffer(reinterpret_cast<decltype(buffer)>(inputBuffer))
        {
        }

        virtual ~BitStream() = default;

        int32_t peekBit() const
        {
            int32_t t;
            t = (buffer[(bloc >> 3)] >> (bloc & 7)) & 0x1;
            return t;
        }

        int32_t getBit()
        {
            int32_t bit = peekBit();
            bloc++;
            return bit;
        }

        int32_t getBitOffset(size_t *offset)
        {
            int32_t t;
            bloc = *offset;
            t = getBit();
            *offset = bloc;
            return t;
        }

        size_t getLocation() const
        {
            return bloc;
        }

        void setLocation(size_t newLoc)
        {
            bloc = newLoc;
        }

        size_t addLocation(size_t bits)
        {
            bloc += bits;
            return bloc;
        }

        const std::remove_const_t<InputBufferElementType> & operator[](size_t idx) const
        {
            return buffer[idx];
        }

        const std::remove_const_t<InputBufferElementType> *getBuffer() const
        {
            return reinterpret_cast<const std::remove_const_t<InputBufferElementType> *>(buffer);
        }

    protected:
        size_t bloc = 0;
        std::conditional_t<
            std::is_const_v<InputBufferElementType>,
            const uint8_t *,
            uint8_t *
        > buffer = nullptr;
    };
    using ByteBitStream = BitStream<const uint8_t>;

    template<typename InputBufferElementType>
    class WriteableBitStream : public BitStream<InputBufferElementType> {
    public:
        WriteableBitStream(InputBufferElementType *inputBuffer, size_t startBloc = 0) :
            BitStream<InputBufferElementType>(inputBuffer, startBloc)
        {
        }

        virtual ~WriteableBitStream() = default;

        void addBit(unsigned char bit)
        {
            if ((this->bloc & 0b111) == 0) {
                this->buffer[(this->bloc >> 3)] = 0;
            }
            this->buffer[(this->bloc >> 3)] |= bit << (this->bloc & 0b111);
            this->bloc++;
        }
        void putBit(unsigned char bit, size_t *offset)
        {
            this->bloc = *offset;
            if ((this->bloc & 7) == 0) {
                this->buffer[(this->bloc >> 3)] = 0;
            }
            this->buffer[(this->bloc >> 3)] |= bit << (this->bloc & 7);
            this->bloc++;
            *offset = this->bloc;
        }

        InputBufferElementType & operator[](size_t idx)
        {
            return reinterpret_cast<InputBufferElementType &>(this->buffer[idx]);
        }

        InputBufferElementType *getBuffer()
        {
            return reinterpret_cast<InputBufferElementType *>(this->buffer);
        }
    };
    using WriteableByteBitStream = WriteableBitStream<uint8_t>;

    class Huffman {
    protected:
        static constexpr size_t HMAX = 256;  /*Maximum symbol */
        static constexpr size_t NYT = HMAX;  /*NYT = Not Yet Transmitted */
        static constexpr size_t INTERNAL_NODE = (HMAX + 1);

        struct node_t {
            node_t *left, *right, *parent; /*tree structure */
            node_t *next, *prev; /*doubly-linked list */
            node_t **head; /*highest ranked node in block */
            int32_t weight;
            int32_t symbol;
        };

        struct huff_t {
            int32_t     blocNode;
            int32_t     blocPtrs;

            node_t*     tree;
            node_t*     lhead;
            node_t*     ltail;
            node_t*     loc[HMAX + 1];
            node_t**    freelist;

            node_t      nodeList[768];
            node_t*     nodePtrs[768];
        };

        struct huffman_t {
            huff_t        compressor;
            huff_t        decompressor;
        };

    public:
        Huffman() = default;
        virtual ~Huffman() = default;

        enum HuffType {
            HUFF_COMPRESS,
            HUFF_DECOMPRESS,
        };

        size_t decompress(const char *input, size_t inputSize, char *output, size_t outputSize);
        std::string decompress(std::string_view input);
        size_t compress(const char *input, size_t inputSize, char *output, size_t outSize);
        std::string compress(std::string_view input, size_t maxCompressedSize = 1400);

        size_t getDecompressedSize(const char *input);

    protected:
        int32_t Huff_Receive(node_t *node, int32_t *ch, ByteBitStream & fin);
        node_t **get_ppnode(huff_t *huff);
        void swap(huff_t *huff, node_t *node1, node_t *node2);
        void swaplist(node_t *node1, node_t *node2);
        void free_ppnode(huff_t *huff, node_t **ppnode);
        void increment(huff_t *huff, node_t *node);
        void Huff_addRef(huff_t *huff, uint8_t ch);
        void send(const node_t *node, const node_t *child, WriteableByteBitStream & fout);
        void Huff_transmit(huff_t *huff, int32_t ch, WriteableByteBitStream & fout);
    };

    class Q3Huffman : public Huffman {
    public:
        Q3Huffman();
        Q3Huffman(Q3Huffman && other) = default;
        Q3Huffman & operator=(Q3Huffman && other) = default;
        Q3Huffman & operator=(const Q3Huffman & other) = delete;
        virtual ~Q3Huffman() = default;

        void offsetTransmit(HuffType type, int32_t ch, WriteableByteBitStream & fout, size_t *offset);
        void offsetReceive(HuffType type, int32_t *ch, ByteBitStream & fin, size_t *offset);
        void offsetReceive(HuffType type, int32_t *ch, WriteableByteBitStream & fin, size_t *offset);

    protected:
        huffman_t msgHuff;
    };
}
