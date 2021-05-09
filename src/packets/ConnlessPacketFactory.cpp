#include "../../include/packets/ConnlessPacketFactory.h"

#include "../../include/CTHash.h"
#include "../../include/jka/JKADefs.h"

#include "../../include/packets/AllConnlessPackets.h"

namespace JKA::Packets::ConnlessPacketFactory {
    std::unique_ptr<ConnlessPacket> parsePacket(std::string_view rawPacket)
    {
        // The connless packet must have a non-empty name
        if (rawPacket.size() <= CONNLESS_PREFIX_SIZE) {
            return nullptr;
        }

        if (!ConnlessPacket::isConnless(rawPacket)) {
            return nullptr;
        }

        std::string_view packet = rawPacket.substr(CONNLESS_PREFIX_SIZE);

        // Not a valid algorithm in general, but all JKA packet
        // names consist of alphabetical characters only, so to extract
        // them we need to find the first possible separator
        size_t possibleSeparatorIdx = packet.find_first_of(CONNLESS_SEPARATORS);
        std::string_view packetName = packet.substr(0, possibleSeparatorIdx);
        std::string_view data = "";

#define CONLESS_PACKETS_LIST_ENTRY(type_, cls_name, str, sep)                     \
                case ct_hash(CONNLESS_PACKETS[type_].name):                       \
                {                                                                 \
                    if (CONNLESS_PACKETS[type_].name != packetName) {             \
                        break;  /* Collision */                                   \
                    }                                                             \
                    if (!CONNLESS_PACKETS[type_].separator.empty()                \
                        && possibleSeparatorIdx < packet.size()) {                \
                        data = packet.substr(possibleSeparatorIdx + 1);           \
                    }                                                             \
                    return cls_name::parse(data);                                 \
                }

        switch (ct_hash(packetName)) {
            #include "../../include/data/ConnlessPacketsList.inc"
        }

#undef CONLESS_PACKETS_LIST_ENTRY

        // Packet name not found / collision
        return nullptr;
    }
}
