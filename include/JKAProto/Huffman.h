#pragma once
#include <cassert>
#include <cinttypes>
#include <cstring>   // memset, memcpy
#include <string>
#include <string_view>
#include <type_traits>

#include "SharedDefs.h"
#include "utility/Traits.h"

#include "_HuffmanTable.h"

namespace JKA {
    class BitStream {
    public:
        explicit constexpr BitStream(const ByteType *inputBuffer, size_t startBloc = 0) noexcept :
            bloc(startBloc),
            buffer(inputBuffer)
        {
        }

        template<typename CharT, std::enable_if_t<Utility::can_alias_v<CharT, ByteType>, int> = 0>
        explicit BitStream(const CharT *inputBuffer, size_t startBloc = 0) noexcept :
            bloc(startBloc),
            buffer(reinterpret_cast<const ByteType *>(inputBuffer))
        {
            Utility::CheckAliasing<CharT, ByteType>();
        }

        constexpr BitStream(const BitStream &) noexcept = default;
        constexpr BitStream(BitStream &&) noexcept = default;
        constexpr BitStream & operator=(const BitStream &) noexcept = default;
        constexpr BitStream & operator=(BitStream &&) noexcept = default;

        constexpr int32_t peekBit() const noexcept
        {
            return (buffer[(bloc >> 3)] >> (bloc & 7)) & 0x1;
        }

        constexpr int32_t getBit() noexcept
        {
            int32_t bit = peekBit();
            bloc++;
            return bit;
        }

        constexpr int32_t getBitOffset(size_t *offset) noexcept
        {
            bloc = *offset;
            int32_t t = getBit();
            *offset = bloc;
            return t;
        }

        constexpr size_t getLocation() const noexcept
        {
            return bloc;
        }

        constexpr void setLocation(size_t newLoc) noexcept
        {
            bloc = newLoc;
        }

        constexpr size_t addLocation(size_t bits) noexcept
        {
            bloc += bits;
            return bloc;
        }

        constexpr const ByteType & operator[](size_t idx) const noexcept
        {
            return buffer[idx];
        }

        constexpr const ByteType *getBuffer() const noexcept
        {
            return buffer;
        }

    protected:
        size_t bloc = 0;
        const ByteType *buffer = nullptr;
    };

    class WriteableBitStream : public BitStream {
    public:
        explicit constexpr WriteableBitStream(ByteType *inputBuffer, size_t startBloc = 0) noexcept :
            BitStream(inputBuffer, startBloc)
        {
        }

        template<typename CharT, std::enable_if_t<Utility::can_alias_v<CharT, ByteType>, int> = 0>
        explicit WriteableBitStream(CharT *inputBuffer, size_t startBloc = 0) noexcept :
            BitStream(inputBuffer, startBloc)
        {
            Utility::CheckAliasing<CharT, ByteType>();
        }

        constexpr WriteableBitStream(const WriteableBitStream &) noexcept = default;
        constexpr WriteableBitStream(WriteableBitStream &&) noexcept = default;
        constexpr WriteableBitStream & operator=(const WriteableBitStream &) noexcept = default;
        constexpr WriteableBitStream & operator=(WriteableBitStream &&) noexcept = default;

        constexpr void addBit(unsigned char bit)
        {
            if ((bloc & 0b111) == 0) JKA_UNLIKELY {
                bufferMutable()[(bloc >> 3)] = 0;
            }
            bufferMutable()[(bloc >> 3)] |= bit << (bloc & 0b111);
            bloc++;
        }

        constexpr void putBit(unsigned char bit, size_t *offset) noexcept
        {
            bloc = *offset;
            if ((bloc & 0b111) == 0) JKA_UNLIKELY {
                bufferMutable()[(bloc >> 3)] = 0;
            }
            bufferMutable()[(bloc >> 3)] |= bit << (bloc & 7);
            bloc++;
            *offset = bloc;
        }

        constexpr ByteType & operator[](size_t idx) noexcept
        {
            return bufferMutable()[idx];
        }

        constexpr ByteType *getBuffer() noexcept
        {
            return bufferMutable();
        }

    protected:
        constexpr ByteType *bufferMutable() noexcept
        {
            return const_cast<ByteType *>(buffer);
        }
    };

    class Huffman {
    protected:
        static constexpr size_t HMAX = 256;  /*Maximum symbol */
        static constexpr size_t NYT = HMAX;  /*NYT = Not Yet Transmitted */
        static constexpr size_t INTERNAL_NODE = (HMAX + 1);

        struct node_t {
            node_t *left{}, *right{}, *parent{}; /*tree structure */
            node_t *next{}, *prev{}; /*doubly-linked list */
            node_t **head{}; /*highest ranked node in block */
            int32_t weight{};
            int32_t symbol{};
        };

        struct huff_t {
            int32_t     blocNode{};
            int32_t     blocPtrs{};

            node_t*     tree{};
            node_t*     lhead{};
            node_t*     ltail{};
            node_t*     loc[HMAX + 1]{};
            node_t**    freelist{};

            node_t      nodeList[768]{};
            node_t*     nodePtrs[768]{};
        };

        struct huffman_t {
            huff_t compressor{};
            huff_t decompressor{};
        };

    public:
        constexpr Huffman() noexcept = default;
        constexpr Huffman(const Huffman &) noexcept = default;
        constexpr Huffman(Huffman &&) noexcept = default;
        constexpr Huffman & operator=(const Huffman &) noexcept = default;
        constexpr Huffman & operator=(Huffman &&) noexcept = default;

        enum HuffType {
            HUFF_COMPRESS,
            HUFF_DECOMPRESS,
        };

        size_t decompress(const char *input, size_t inputSize, char *output, size_t outputSize);
        std::string decompress(std::string_view input);
        size_t compress(const char *input, size_t inputSize, char *output, size_t outSize);
        std::string compress(std::string_view input, size_t maxCompressedSize = 1400);

        size_t getDecompressedSize(const char *input) noexcept;

    protected:
        int32_t Huff_Receive(node_t *node, int32_t *ch, BitStream & fin) noexcept;
        node_t **get_ppnode(huff_t *huff) noexcept;
        void swap(huff_t *huff, node_t *node1, node_t *node2) noexcept;
        void swaplist(node_t *node1, node_t *node2) noexcept;
        void free_ppnode(huff_t *huff, node_t **ppnode) noexcept;
        void increment(huff_t *huff, node_t *node) noexcept;
        void Huff_addRef(huff_t *huff, uint8_t ch) noexcept;
        void send(const node_t *node, const node_t *child, WriteableBitStream & fout) noexcept;
        void Huff_transmit(huff_t *huff, int32_t ch, WriteableBitStream & fout) noexcept;
    };

    class Q3Huffman : public Huffman {
    public:
        constexpr Q3Huffman() noexcept
        {
            // Initialize the tree & list with the NYT node 
            msgHuff.decompressor.tree = msgHuff.decompressor.lhead
                = msgHuff.decompressor.ltail
                = msgHuff.decompressor.loc[NYT]
                = &(msgHuff.decompressor.nodeList[msgHuff.decompressor.blocNode++]);
            msgHuff.decompressor.tree->symbol = NYT;
            msgHuff.decompressor.tree->weight = 0;
            msgHuff.decompressor.lhead->next = msgHuff.decompressor.lhead->prev = NULL;
            msgHuff.decompressor.tree->parent = msgHuff.decompressor.tree->left = msgHuff.decompressor.tree->right = NULL;

            // Add the NYT (not yet transmitted) node into the tree/list */
            msgHuff.compressor.tree = msgHuff.compressor.lhead = msgHuff.compressor.loc[NYT] = &(msgHuff.compressor.nodeList[msgHuff.compressor.blocNode++]);
            msgHuff.compressor.tree->symbol = NYT;
            msgHuff.compressor.tree->weight = 0;
            msgHuff.compressor.lhead->next = msgHuff.compressor.lhead->prev = NULL;
            msgHuff.compressor.tree->parent = msgHuff.compressor.tree->left = msgHuff.compressor.tree->right = NULL;
            msgHuff.compressor.loc[NYT] = msgHuff.compressor.tree;

            for (size_t i = 0; i < std::size(msg_hData); i++) {
                for (int32_t j = 0; j < msg_hData[i]; j++) {
                    Huff_addRef(&msgHuff.compressor, static_cast<uint8_t>(i));  // Do update
                    Huff_addRef(&msgHuff.decompressor, static_cast<uint8_t>(i));  // Do update
                }
            }
        }

        constexpr Q3Huffman(const Q3Huffman & other) noexcept = delete;
        constexpr Q3Huffman(Q3Huffman && other) noexcept = default;
        constexpr Q3Huffman & operator=(const Q3Huffman & other) noexcept = delete;
        constexpr Q3Huffman & operator=(Q3Huffman && other) noexcept = default;

        template<HuffType Type>
        constexpr void offsetTransmit(int32_t ch, WriteableBitStream & fout, size_t *offset) noexcept
        {
            fout.setLocation(*offset);

            static_assert(Type == Huffman::HUFF_COMPRESS || Huffman::HUFF_DECOMPRESS);
            if constexpr (Type == Huffman::HUFF_COMPRESS) {
                send(msgHuff.compressor.loc[ch], NULL, fout);
            } else if constexpr (Type == Huffman::HUFF_DECOMPRESS) {
                send(msgHuff.decompressor.loc[ch], NULL, fout);
            }

            *offset = fout.getLocation();
        }

        template<HuffType Type>
        constexpr void offsetReceive(int32_t *ch, BitStream & fin, size_t *offset) noexcept
        {
            fin.setLocation(*offset);
            const node_t *node = nullptr;

            static_assert(Type == Huffman::HUFF_COMPRESS || Huffman::HUFF_DECOMPRESS);
            if constexpr (Type == Huffman::HUFF_COMPRESS) {
                node = msgHuff.compressor.tree;
            } else if constexpr (Type == Huffman::HUFF_DECOMPRESS) {
                node = msgHuff.decompressor.tree;
            }

            while (node && node->symbol == INTERNAL_NODE) {
                if (fin.getBit()) {
                    node = node->right;
                } else {
                    node = node->left;
                }
            }

            if (node == nullptr) JKA_UNLIKELY {
                assert(node != nullptr);
                *ch = 0;
                return;
            }

            *ch = node->symbol;
            *offset = fin.getLocation();
        }

        template<HuffType Type>
        constexpr void offsetReceive(int32_t *ch, WriteableBitStream & fin, size_t *offset) noexcept
        {
            BitStream inStream(fin.getBuffer(), fin.getLocation());
            offsetReceive<Type>(ch, inStream, offset);
        }

    protected:
        huffman_t msgHuff{};
    };
}
