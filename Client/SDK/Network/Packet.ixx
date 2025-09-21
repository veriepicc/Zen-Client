module;
#include <string>
#include <stdint.h>

export module Packet;

namespace Packets {
	enum class PacketID : uint8_t {
		NONE = 0,
		LOGIN = 0x1,
		TEXT = 0x9,
		ACTOR_EVENT = 0x1B,
		CHANGE_DIMENSION = 0x3D,
		TRANSFER = 0x55,
		SET_TITLE = 0x58,
		COMMAND_REQUEST = 0x4D,
		TOAST_REQUEST = 0xBA,
		MODAL_FORM_REQUEST = 0x64,
		SET_SCORE = 0x6c,
		COUNT,
	};
}

export class Packet {
public:
	int32_t priority = 2;
	int32_t reliability = 1;
	uint8_t subClientId = 0;
	bool isHandled = false;
	void* unknown = nullptr;
	void*** handler = nullptr;
	int32_t compressibility = 0;

public:
	virtual ~Packet() {};
	virtual Packets::PacketID getID() { return Packets::PacketID::NONE; };
	virtual std::string getName() { return ""; };
	virtual void write(void* stream) {};
	virtual void readExtended(void* stream) {};
	virtual bool allowBatch() { return false; };
	virtual void _read(void* stream) {};
};

export class PacketSender {
	virtual ~PacketSender() = 0;
public:
	virtual bool isInitialized() = 0;
	virtual void send(Packet*) = 0;
	virtual void sendTo(void* networkIdentifier, uint8_t subClientId, Packet* pkt);
	virtual void sendToServer(Packet* pkt) = 0;
};