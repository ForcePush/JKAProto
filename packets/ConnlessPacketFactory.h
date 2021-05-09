#pragma once
#include <memory>
#include "ConnlessPacket.h"

namespace JKA::Packets::ConnlessPacketFactory {
    std::unique_ptr<ConnlessPacket> parsePacket(std::string_view rawPacket);
}
