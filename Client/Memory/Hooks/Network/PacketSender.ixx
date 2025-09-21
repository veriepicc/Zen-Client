module;
#include <iostream>
#include <optional>

export module SendToServer;

import HookManager;
import SigManager;
import Packet;

using SendToServerFunction = void(*)(PacketSender* sender, Packet* packet);
SendToServerFunction originalFunction = nullptr;

void PacketSender_SendToServer(PacketSender* sender, Packet* packet) {
    static bool loggedOnce = false;
    if (originalFunction)
        originalFunction(sender, packet);

    if (sender)
    {
        if (!loggedOnce)
        {
			std::cout << static_cast<uint8_t>(packet->getID()) << std::endl;
            std::cout << "[SendToServer] first call ok ps=" << sender << std::endl;
            loggedOnce = true;
        }
    }
}

export namespace Hooks::Network::SendToServer {
    inline bool Install()
    {
        void* target = SigManager::PacketSender_SendToServer;
        if (!target)
        {
            std::cout << "[SendToServer] signature not resolved" << std::endl;
            return false;
        }

        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<SendToServerFunction>(
            target,
            PacketSender_SendToServer,
            &originalFunction
        );
        if (ok) std::cout << "[SendToServer] hook installed" << std::endl;
        return ok;
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Network::SendToServer::Install };