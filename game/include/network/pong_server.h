#pragma once
#include <memory>

#include "game/game_pong_manager.h"
#include "pong_packet_type.h"
#include "engine/system.h"
#include "game/game_pong_globals.h"

namespace game
{
    class Server : public PacketSenderInterface, public core::SystemInterface
    {
    protected:
        virtual void SpawnNewPlayer(ClientId clientId, PlayerNumber playerNumber) = 0;
        virtual void ReceivePacket(std::unique_ptr<Packet> packet);

        //Server game manager
        GameManager gameManager_;
        PlayerNumber lastPlayerNumber_ = 0;
        std::array<ClientId, maxPlayerNmb> clientMap_{};
    };
}
