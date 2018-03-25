#pragma once

class SampRpFuncs {
private:
	static bool _botSuspended;

public:
	SampRpFuncs();
	~SampRpFuncs();

	static void pickUpPickup(Pickup *pickupId);
	static void takeCheckpoint();
	static bool isBotSuspended() { return _botSuspended; }
	static bool isSampRpServer();
	static bool onServerMessage(std::string msg);
};

