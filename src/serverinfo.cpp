#include "StdAfx.h"

#include "RakBot.h"
#include "PlayerBase.h"
#include "Player.h"
#include "Bot.h"
#include "RakNet.h"
#include "Settings.h"
#include "Funcs.h"

SOCKET s;
sockaddr_in addr;

void SendPacket(char *data, int size) {
	RakNet::BitStream bsSend;
	bsSend.Write("SAMP", 4);
	bsSend.Write(addr.sin_addr.s_addr);
	bsSend.Write(RakBot::app()->getSettings()->getAddress()->getPort());
	bsSend.Write(data, size);

	sendto(s, (char *)bsSend.GetData(), bsSend.GetNumberOfBytesUsed(), NULL, (SOCKADDR *)&addr, sizeof(addr));
}

void ServerInfo() {
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	DWORD adapterAddress = inet_addr(vars.adapterAddress.c_str());
	if (adapterAddress != 0) {
		sockaddr_in adapter_addr;
		adapter_addr.sin_family = AF_INET;
		adapter_addr.sin_addr.s_addr = adapterAddress;
		adapter_addr.sin_port = 0;
		bind(s, (sockaddr *)&adapter_addr, sizeof(adapter_addr));
		RakBot::app()->log("[RAKBOT] Выбран адаптер с адресом: %s (ServerInfo)", vars.adapterAddress.c_str());
	}

	u_long arg = 1;
	ioctlsocket(s, FIONBIO, &arg);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(RakBot::app()->getSettings()->getAddress()->getIp().c_str());
	addr.sin_port = htons(RakBot::app()->getSettings()->getAddress()->getPort());

	while (!vars.botOff) {
		Sleep(1000);
		SendPacket("p4150", 5);
		SendPacket("i", 1);
		SendPacket("c", 1);
		SendPacket("r", 1);
	}

	closesocket(s);
}