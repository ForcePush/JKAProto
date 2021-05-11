#include <JKAProto/Huffman.h>

#include <climits>  // CHAR_BIT
#include <memory>

#include <JKAProto/jka/JKADefs.h>

namespace JKA {
    /* Get a symbol */
    int32_t Huffman::Huff_Receive(node_t *node, int32_t *ch, BitStream & fin) noexcept
    {
        while (node && node->symbol == INTERNAL_NODE) {
            if (fin.getBit()) {
                node = node->right;
            } else {
                node = node->left;
            }
        }
        if (!node) {
            return 0;
        }
        return (*ch = node->symbol);
    }

    Huffman::node_t **Huffman::get_ppnode(huff_t* huff) noexcept
    {
        node_t **tppnode = nullptr;
        if (!huff->freelist) {
            return &(huff->nodePtrs[huff->blocPtrs++]);
        } else {
            tppnode = huff->freelist;
            huff->freelist = reinterpret_cast<node_t **>(*tppnode);
            return tppnode;
        }
    }

    /* Swap the location of these two nodes in the tree */
    void Huffman::swap(huff_t* huff, node_t *node1, node_t *node2) noexcept
    {
        node_t *par1{}, *par2{};

        par1 = node1->parent;
        par2 = node2->parent;

        if (par1) {
            if (par1->left == node1) {
                par1->left = node2;
            } else {
                par1->right = node2;
            }
        } else {
            huff->tree = node2;
        }

        if (par2) {
            if (par2->left == node2) {
                par2->left = node1;
            } else {
                par2->right = node1;
            }
        }     else {
            huff->tree = node1;
        }

        node1->parent = par2;
        node2->parent = par1;
    }

    /* Swap these two nodes in the linked list (update ranks) */
    void Huffman::swaplist(node_t *node1, node_t *node2) noexcept
    {
        node_t *par1 = node1->next;
        node1->next = node2->next;
        node2->next = par1;

        par1 = node1->prev;
        node1->prev = node2->prev;
        node2->prev = par1;

        if (node1->next == node1) {
            node1->next = node2;
        }
        if (node2->next == node2) {
            node2->next = node1;
        }
        if (node1->next) {
            node1->next->prev = node1;
        }
        if (node2->next) {
            node2->next->prev = node2;
        }
        if (node1->prev) {
            node1->prev->next = node1;
        }
        if (node2->prev) {
            node2->prev->next = node2;
        }
    }

    void Huffman::free_ppnode(huff_t* huff, node_t **ppnode) noexcept
    {
        *ppnode = reinterpret_cast<node_t *>(huff->freelist);
        huff->freelist = ppnode;
    }

    /* Do the increments */
    void Huffman::increment(huff_t* huff, node_t *node) noexcept
    {
        node_t *lnode = nullptr;

        if (!node) {
            return;
        }

        if (node->next != NULL && node->next->weight == node->weight) {
            lnode = *node->head;
            if (lnode != node->parent) {
                swap(huff, lnode, node);
            }
            swaplist(lnode, node);
        }
        if (node->prev && node->prev->weight == node->weight) {
            *node->head = node->prev;
        } else {
            *node->head = NULL;
            free_ppnode(huff, node->head);
        }
        node->weight++;
        if (node->next && node->next->weight == node->weight) {
            node->head = node->next->head;
        } else {
            node->head = get_ppnode(huff);
            *node->head = node;
        }
        if (node->parent) {
            increment(huff, node->parent);
            if (node->prev == node->parent) {
                swaplist(node, node->parent);
                if (*node->head == node) {
                    *node->head = node->parent;
                }
            }
        }
    }

    void Huffman::Huff_addRef(huff_t* huff, uint8_t ch) noexcept
    {
        node_t *tnode = nullptr, *tnode2 = nullptr;
        if (huff->loc[ch] == NULL) { /* if this is the first transmission of this node */
            tnode = &(huff->nodeList[huff->blocNode++]);
            tnode2 = &(huff->nodeList[huff->blocNode++]);

            tnode2->symbol = INTERNAL_NODE;
            tnode2->weight = 1;
            tnode2->next = huff->lhead->next;
            if (huff->lhead->next) {
                huff->lhead->next->prev = tnode2;
                if (huff->lhead->next->weight == 1) {
                    tnode2->head = huff->lhead->next->head;
                } else {
                    tnode2->head = get_ppnode(huff);
                    *tnode2->head = tnode2;
                }
            } else {
                tnode2->head = get_ppnode(huff);
                *tnode2->head = tnode2;
            }
            huff->lhead->next = tnode2;
            tnode2->prev = huff->lhead;

            tnode->symbol = ch;
            tnode->weight = 1;
            tnode->next = huff->lhead->next;
            if (huff->lhead->next) {
                huff->lhead->next->prev = tnode;
                if (huff->lhead->next->weight == 1) {
                    tnode->head = huff->lhead->next->head;
                } else {
                    /* this should never happen */
                    tnode->head = get_ppnode(huff);
                    *tnode->head = tnode2;
                }
            } else {
                /* this should never happen */
                tnode->head = get_ppnode(huff);
                *tnode->head = tnode;
            }
            huff->lhead->next = tnode;
            tnode->prev = huff->lhead;
            tnode->left = tnode->right = NULL;

            if (huff->lhead->parent) {
                if (huff->lhead->parent->left == huff->lhead) { /* lhead is guaranteed to by the NYT */
                    huff->lhead->parent->left = tnode2;
                } else {
                    huff->lhead->parent->right = tnode2;
                }
            } else {
                huff->tree = tnode2;
            }

            tnode2->right = tnode;
            tnode2->left = huff->lhead;

            tnode2->parent = huff->lhead->parent;
            huff->lhead->parent = tnode->parent = tnode2;

            huff->loc[ch] = tnode;

            increment(huff, tnode2->parent);
        } else {
            increment(huff, huff->loc[ch]);
        }
    }

    size_t Huffman::getDecompressedSize(const char *input) noexcept
    {
        return static_cast<size_t>(input[0]) * 256 + static_cast<size_t>(input[1]);
    }

    size_t Huffman::decompress(const char *input, size_t inputSize, char *output, size_t outputSize)
    {
        size_t        cch = 0;
        int32_t       ch = 0;
        huff_t        huff{};
        BitStream     inputBuffer(input);

        if (inputSize <= 0) {
            return 0;
        }

        // Initialize the tree & list with the NYT node 
        huff.tree = huff.lhead = huff.ltail = huff.loc[NYT] = &(huff.nodeList[huff.blocNode++]);
        huff.tree->symbol = NYT;
        huff.tree->weight = 0;
        huff.lhead->next = huff.lhead->prev = NULL;
        huff.tree->parent = huff.tree->left = huff.tree->right = NULL;

        cch = getDecompressedSize(input);
        inputBuffer.addLocation(2 * CHAR_BIT);

        // don't overflow with bad messages
        if (cch > outputSize) {
            cch = outputSize;
        }

        for (size_t j = 0; j < cch; j++) {
            ch = 0;
            // don't overflow reading from the messages
            // FIXME: would it be better to have a overflow check in get_bit ?
            if ((inputBuffer.getLocation() >> 3) > inputSize) {
                output[j] = 0;
                break;
            }
            Huff_Receive(huff.tree, &ch, inputBuffer);  /* Get a character */
            if (ch == NYT) {  /* We got a NYT, get the symbol associated with it */
                ch = 0;
                for (size_t i = 0; i < 8; i++) {
                    ch = (ch << 1) + inputBuffer.getBit();
                }
            }

            output[j] = static_cast<char>(ch);  /* Write symbol */

            Huff_addRef(&huff, (uint8_t)ch);  /* Increment node */
        }

        return cch;
    }

    std::string Huffman::decompress(std::string_view input)
    {
        size_t decompressedSize = getDecompressedSize(input.data());
        std::string result(decompressedSize, '\x00');

        decompress(input.data(), input.size(), result.data(), result.size());

        return result;
    }

    /* Send the prefix code for this node */
    void Huffman::send(const node_t *node, const node_t *child, WriteableBitStream & fout) noexcept
    {
        if (node->parent) {
            send(node->parent, node, fout);
        }
        if (child) {
            if (node->right == child) {
                fout.addBit(1);
            } else {
                fout.addBit(0);
            }
        }
    }

    /* Send a symbol */
    void Huffman::Huff_transmit(huff_t *huff, int32_t ch, WriteableBitStream & fout) noexcept
    {
        if (huff->loc[ch] == NULL) {
            /* node_t hasn't been transmitted, send a NYT, then the symbol */
            Huff_transmit(huff, NYT, fout);
            for (int32_t i = 7; i >= 0; i--) {
                fout.addBit(static_cast<uint8_t>((ch >> i) & 0x1));
            }
        } else {
            send(huff->loc[ch], NULL, fout);
        }
    }

    // TODO: check for outSize overflow
    size_t Huffman::compress(const char *input, size_t inputSize, char *output, size_t outSize)
    {
        (void)outSize;  // Unused for now, see TODO above

        int32_t                 ch = 0;
        WriteableBitStream      seq(output);
        const uint8_t*          inputBytes = reinterpret_cast<const uint8_t *>(input);
        huff_t                  huff{};

        if (inputSize <= 0) {
            return 0;
        }

        // Add the NYT (not yet transmitted) node into the tree/list */
        huff.tree = huff.lhead = huff.loc[NYT] = &(huff.nodeList[huff.blocNode++]);
        huff.tree->symbol = NYT;
        huff.tree->weight = 0;
        huff.lhead->next = huff.lhead->prev = NULL;
        huff.tree->parent = huff.tree->left = huff.tree->right = NULL;
        huff.loc[NYT] = huff.tree;

        seq[0] = (inputSize >> CHAR_BIT) & 0xff;
        seq[1] = inputSize & 0xff;
        seq.setLocation(2 * CHAR_BIT);

        for (size_t i = 0; i < inputSize; i++) {
            ch = inputBytes[i];
            Huff_transmit(&huff, ch, seq);  /* Transmit symbol */
            Huff_addRef(&huff, static_cast<uint8_t>(ch));  /* Do update */
        }

        seq.addLocation(CHAR_BIT);  // next byte

        return (seq.getLocation() >> 3);
    }

    std::string Huffman::compress(std::string_view input, size_t maxCompressedSize)
    {
        auto buffer = std::string(maxCompressedSize, '\x00');
        size_t len = compress(input.data(), input.size(), buffer.data(), maxCompressedSize);
        buffer.erase(len);
        return buffer;
    }
}
