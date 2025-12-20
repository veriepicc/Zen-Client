export module Global;

import ClientInstance;
import LocalPlayer;


export namespace Global
{
    inline ClientInstance* clientInstance = nullptr;

    void setClientInstance(ClientInstance* instance) { clientInstance = instance; }
    ClientInstance* getClientInstance() { return clientInstance; }

    LocalPlayer* getLocalPlayer()
    {
        if (!clientInstance) return nullptr;
        return clientInstance->getLocalPlayer();
    }
}