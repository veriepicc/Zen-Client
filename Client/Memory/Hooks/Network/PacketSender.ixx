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
    if (originalFunction)
        originalFunction(sender, packet);
}

export namespace Hooks::Network::SendToServer {
    inline bool Install()
    {
        void* target = SigManager::PacketSender_SendToServer;
        if (!target)
        {
            return false;
        }

        auto& hookManager = GetHookManager();
        bool ok = hookManager.hook<SendToServerFunction>(
            target,
            PacketSender_SendToServer,
            &originalFunction
        );
        return ok;
    }
}

static HookRegistry::Registration RegisterHook{ &Hooks::Network::SendToServer::Install };