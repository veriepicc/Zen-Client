export module Global;

import ClientInstance;

export namespace Global {
	inline ClientInstance* clientInstance = nullptr;

	void setClientInstance(ClientInstance* ci) { clientInstance = ci; }
	ClientInstance* getClientInstance() { return clientInstance; }
}