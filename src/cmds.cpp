// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "StdAfx.h"

#include "RakBot.h"
#include "PlayerBase.h"
#include "Player.h"
#include "Bot.h"
#include "RakNet.h"
#include "Script.h"
#include "Settings.h"
#include "Pickup.h"
#include "Vehicle.h"
#include "Events.h"

#include "Funcs.h"
#include "MathStuff.h"
#include "VehicleStuff.h"
#include "MiscFuncs.h"
#include "SampRpFuncs.h"

#include "main.h"
#include "ini.h"
#include "netrpc.h"
#include "mapwnd.h"
#include "netgame.h"
#include "window.h"

#define cmdcmp(command) !strnicmp(cmd, command, strlen(command))

void RunCommand(const char *cmdstr) {
	static std::mutex mutex;
	std::lock_guard<std::mutex> lock(mutex);

	std::string command = cmdstr;

	if (command.empty())
		return;

	if (RakBot::app()->getEvents()->onRunCommand(command))
		return;

	RakClientInterface *rakClient = RakBot::app()->getRakClient();
	Bot *bot = RakBot::app()->getBot();

	if (command[0] != '!') {
		bot->sendInput(command);
		return;
	}

	std::string s = command.substr(1, command.length() - 1);
	const char *cmd = s.c_str();

	// ����� �� �������
	if (cmdcmp("quit")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ �� ����, �� ������� ����������",
				"������",
				MB_ICONASTERISK);
			return;
		}

		RakBot::app()->exit();
		return;
	}

	if (cmdcmp("keeponline")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� �������� ������� ���� (������� ������� � ���� ��� �������� � �����).\n"
				"������� 2 ���������: ������ ������ � ����� ������ � �������. �� ����, ���� ������ \n"
				"\"!keeponline 10 30\", ��� ����� �������� ������ ��� � 10-� ������ � �������� 30 ����� (�� 40-� ������).",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int keepOnlineTime;

		if (sscanf(cmd, "%*s%d%d", &vars.keepOnlineBegin, &keepOnlineTime) < 2) {
			vars.keepOnlineEnabled = false;
			RakBot::app()->log("[RAKBOT] �������: !keeponline [������ ������] [����� ������ � �������]");
			return;
		}

		if (keepOnlineTime < 0 || keepOnlineTime > 60) {
			vars.keepOnlineEnabled = false;
			RakBot::app()->log("[RAKBOT] ����� ������ ����� ���� �������� 60 �����!");
			return;
		}

		vars.keepOnlineEnd = vars.keepOnlineBegin + keepOnlineTime;

		if (vars.keepOnlineEnd > 59) {
			vars.keepOnlineEnd %= 60;
			vars.keepOnlineBeginAfterEnd = true;
		}

		vars.keepOnlineEnabled ^= true;

		if (vars.keepOnlineEnabled) {
			char msg[512];
			snprintf(msg, sizeof(msg), "[RAKBOT] ��� ����� �������� � %d �� %d ������%s",
				vars.keepOnlineBegin, vars.keepOnlineEnd, vars.keepOnlineBeginAfterEnd ? " ���������� ����" : "");
			RakBot::app()->log(msg);
		} else {
			RakBot::app()->log("[RAKBOT] �������� ������� ��������");
		}

		return;
	}

	// ANTIAFK
	if (cmdcmp("aafk")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� �������, �� ������� ����������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.antiAfkEnabled ^= 1;

		if (vars.antiAfkEnabled)
			RakBot::app()->log("[RAKBOT] AntiAFK �������");
		else
			RakBot::app()->log("[RAKBOT] AntiAFK ��������");
		return;
	}

	// NOAFK
	if (cmdcmp("noafk")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� NoAFK, �� ������� ����������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.noAfk ^= 1;
		if (vars.noAfk)
			RakBot::app()->log("[RAKBOT] NoAFK �������");
		else
			RakBot::app()->log("[RAKBOT] NoAFK ��������");
		return;
	}

	// SPAWN

	if (cmdcmp("reqspawn")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������� ������ ����, �� ������� ����������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		bot->requestSpawn();
		vars.waitForRequestSpawnReply = true;
		return;
	}

	if (cmdcmp("spawn")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ����, �� ������� ����������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		bot->spawn();
		return;
	}

	if (cmdcmp("reqclass")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ������, ������� 1 ��������: ID ������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int classId;
		if (sscanf(cmd, "%*s%d", &classId) < 1) {
			RakBot::app()->log("[RAKBOT] ������� ID ������");
			return;
		}

		bot->requestClass(classId);
		RakBot::app()->log("[RAKBOT] ������ ����� %d", classId);
		return;
	}

	if (cmdcmp("sendpick")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� �������� ������, ������� 1 ��������: ID ������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int pickupId;
		if (sscanf(cmd, "%*s%d", &pickupId) < 1) {
			RakBot::app()->log("[RAKBOT] �������� ������: ������� ID ������");
			return;
		}

		RakBot::app()->log("[RAKBOT] �������� ������ � ID %d", pickupId);
		Pickup *pickup = RakBot::app()->getPickup(pickupId);
		bot->pickUpPickup(pickup, false);
		return;
	}

	if (cmdcmp("reloadscripts")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������������ Lua ��������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		UnloadScripts();
		LoadScripts();
		return;
	}

	if (cmdcmp("sleep")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� �������� ���, �������� ����� ������ � ���������������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		Bot *bot = RakBot::app()->getBot();
		PlayerAnimation *botAnim = bot->getAnimation();

		vars.sleepEnabled ^= true;

		if (vars.sleepEnabled) {
			botAnim->setAnimId(390);
			botAnim->setAnimFlags(4356);
			RakBot::app()->log("[RAKBOT] ��� �������");
		} else {
			botAnim->setAnimId(1189);
			botAnim->setAnimFlags(33000);
			RakBot::app()->log("[RAKBOT] ��� ��������");
		}

		return;
	}

	// SPAWN CAR
	if (cmdcmp("spawncar")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ���������� (������� �������, �� � �� ���� ����������, �� ����� �����).\n"
				"������� 1 ��������: ID ������������� ��������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int carid = std::strtoul(&cmd[5], nullptr, 10);

		RakNet::BitStream bsData;
		bsData.Write(carid);
		rakClient->RPC(&RPC_VehicleDestroyed, &bsData, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

		RakBot::app()->log("[RAKBOT] ������ ������ ���������� � ID %d", carid);

		return;
	}

	// TELEPORT
	if (cmdcmp("tp")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������������ ����, ��������� 3 ���������, ������� �������� X, Y � Z ������������.\n"
				"��� ������� ����� ������������ �������!",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!bot->isSpawned())
			return;

		float position[3];

		if (sscanf(cmd, "%*s%f%f%f", &position[0], &position[1], &position[2]) < 3) {
			RakBot::app()->log("[RAKBOT] ��������: ������� ���������� ����� ������");
			return;
		}

		bot->teleport(position[0], position[1], position[2]);

		char buf[512];
		sprintf(buf, "[RAKBOT] ��� �������������� �� ���������� (%0.2f; %0.2f; %0.2f)", position[0], position[1], position[2]);
		RakBot::app()->log(buf);
		return;
	}

	// SAVE COORDS
	if (cmdcmp("scoords")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������� ��������� ��� ��������� ����� ������,\n"
				"��������� 3 ���������, ������� �������� X, Y � Z ������������.\n"
				"��� ������� ����� ������������ �������!",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!bot->isSpawned())
			return;

		float pos[3];

		if (sscanf(cmd, "%*s%f%f%f", &pos[0], &pos[1], &pos[2]) < 3) {
			RakBot::app()->log("[RAKBOT] ���������� �������: ������� ���������� ����� ������");
			return;
		}

		vect3_copy(pos, vars.savedCoords);
		vars.savedTeleportEnabled ^= true;

		if (vars.savedTeleportEnabled) {
			char szBuf[512];
			sprintf(szBuf, "[RAKBOT] ���������� (%0.2f; %0.2f; %0.2f) ���������",
				vars.savedCoords[0], vars.savedCoords[1], vars.savedCoords[2]);
			RakBot::app()->log(szBuf);
			RakBot::app()->log("[RAKBOT] ����� ��������������� ��� ������������� �������� �� ������ ����������");
		} else {
			RakBot::app()->log("[RAKBOT] �������� �� ����������� ���������� ��������");
		}
		return;
	}

	// ROUTE

	if (cmdcmp("routedelay")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� �������� �������� ������� ��� ��������������� ��������"
				"��������� 1 ��������: �������� ���������� � �������������",
				"������",
				MB_ICONASTERISK);
			return;
		}

		int delay;
		if (sscanf(cmd, "%*s%d", &delay) < 1) {
			RakBot::app()->log("[RAKBOT] �������� ��������: ������� �������� ��������������� ��������");
			return;
		}
		vars.routeUpdateDelay = delay;
		RakBot::app()->log("[RAKBOT] �������� ��������: ����������� �������� %d", vars.routeUpdateDelay);
		if (vars.routeUpdateDelay < vars.mainDelay) {
			RakBot::app()->log("[WARNING] �������� ��������������� �������� (%d) �� ����� ���� ������ �������� �������� ���������� (%d)!", vars.routeUpdateDelay, vars.mainDelay);
		}
		return;
	}

	if (cmdcmp("routecount")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� ���������� �������� �������� ������� ��� ��������������� ��������"
				"��������� 1 ��������: ���������� �������� �������� �������",
				"������",
				MB_ICONASTERISK);
			return;
		}

		int count;
		if (sscanf(cmd, "%*s%d", &count) < 1) {
			RakBot::app()->log("[RAKBOT] ������ ������� ��������: ������� ���������� �������� �������� �������");
			return;
		}
		vars.routeUpdateCount = count;
		RakBot::app()->log("[RAKBOT] ������ ������� ��������: �������� ������� ����� ����������� %d ���", vars.routeUpdateCount);
		return;
	}

	if (cmdcmp("routeloop")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ������� ��� ��������������� ��������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.routeLoop ^= 1;
		if (vars.routeLoop) {
			RakBot::app()->log("[RAKBOT] ������ �������� �������");
		} else {
			RakBot::app()->log("[RAKBOT] ������ �������� ��������");
		}
		return;
	}

	if (cmdcmp("route")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������� ��������, ��������� 2 ���������: ��� ����� �������� (��� ����������) � ��������.\n"
				"�������� ���������� ������������ ��� ������ �� ��������, �������������� �������� ���!\n\n"
				"����� - \"!route +pause\"\n������������� - \"!route +resume\"",
				"������",
				MB_ICONASTERISK);

			return;
		} else if (strstr(cmd, "+pause")) {
			if (!vars.routeEnabled) {
				RakBot::app()->log("[WARNING] ����������� ������� ��� ����������");
				return;
			}

			vars.routeEnabled = false;
			bot->getKeys()->reset();
			bot->getAnimation()->reset();
			for (int i = 0; i < 3; i++)
				bot->setSpeed(i, 0.f);
			bot->sync();
			vars.syncAllowed = true;
			return;
		} else if (strstr(cmd, "+resume")) {
			if (vars.routeEnabled) {
				RakBot::app()->log("[WARNING] ����������� ������� ��� �������");
				return;
			}

			if (vars.routeData.size() < 1) {
				RakBot::app()->log("[ERROR] ������� ��������� �������!");
				return;
			}

			vars.routeEnabled = true;
			vars.syncAllowed = false;
			RakBot::app()->log("[RAKBOT] ����������� ������� ��������� ������");
			return;
		}

		vars.routeEnabled = false;
		bot->getKeys()->reset();
		bot->getAnimation()->reset();
		for (int i = 0; i < 3; i++)
			bot->setSpeed(i, 0.f);
		bot->sync();
		vars.syncAllowed = true;

		char routeFile[64];
		if (sscanf(cmd, "%*s%s", routeFile) < 1) {
			RakBot::app()->log("[RAKBOT] ����������� �������: ������ �������� !route <����> <��������>");
			RakBot::app()->log("[RAKBOT] ����������� �������: ����������");
			return;
		}

		LoadRoute(routeFile);

		if (vars.routeData.size() > 0) {
			vars.routeEnabled = true;
			vars.syncAllowed = false;
			vars.routeIndex = 0;

			RakBot::app()->log("[RAKBOT] ����������� ������� �������");
		} else {
			RakBot::app()->log("[RAKBOT] ����������� �������: ���� ��������� ���� ��� ����������");
		}
		return;
	}

	if (cmdcmp("buswork")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ������ ��������� ��������.\n"
				"������� \"!route\" ��� ��������� ����������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!SampRpFuncs::isSampRpServer()) {
			RakBot::app()->log("[ERROR] ������ ������� ������������� ������ ��� �������� Samp-Rp.Ru");
			return;
		}

		vars.busWorkerRoute = std::strtoul(&cmd[8], nullptr, 10);

		switch (vars.busWorkerRoute) {
			case 0:
				RakBot::app()->log("[RAKBOT] ============[�������������� ��� ��������]=============");
				RakBot::app()->log("[RAKBOT] �������: !buswork [����� ��������]. ������ ���������:");
				RakBot::app()->log("[RAKBOT] 1 - ��������� ��");
				RakBot::app()->log("[RAKBOT] 2 - ��������� ��");
				RakBot::app()->log("[RAKBOT] 3 - ��������� ��");
				RakBot::app()->log("[RAKBOT] 4 - ������������� ��-��");
				RakBot::app()->log("[RAKBOT] 5 - ������������� ��-��");
				RakBot::app()->log("[RAKBOT] 6 - ������������� ��-��");
				RakBot::app()->log("[RAKBOT] 7 - ����������� ��-��");
				RakBot::app()->log("[RAKBOT] 8 - ����������� ��-������");
				RakBot::app()->log("[RAKBOT] ============[�������������� ��� ��������]=============");
				RakBot::app()->log("[RAKBOT] *�������������� ��� ������������ ������ ��� �������� Samp-Rp.Ru");
				return;

			case 1:
				DoCoordMaster(true, 1258.83f, 10.07f, 10.07f);
				vars.busWorkerBusModel = 437;
				RakBot::app()->log("[RAKBOT] ������ ������� ��������: ��������� ��");
				break;

			case 2:
				DoCoordMaster(true, 1985.03f, 96.93f, 23.82f);
				vars.busWorkerBusModel = 437;
				RakBot::app()->log("[RAKBOT] ������ ������� ��������: ��������� ��");
				break;

			case 3:
				DoCoordMaster(true, 2778.17f, 1290.91f, 6.73f);
				vars.busWorkerBusModel = 437;
				RakBot::app()->log("[RAKBOT] ������ ������� ��������: ��������� ��");
				break;

			case 4:
				vars.busWorkerRouteItem = 0;
				DoCoordMaster(true, 1654.91f, -1050.12f, 21.13f);
				vars.busWorkerBusModel = 431;
				RakBot::app()->log("[RAKBOT] ������ ������� ��������: ������������� ��-��");
				break;

			case 5:
				vars.busWorkerRouteItem = 1;
				DoCoordMaster(true, 1654.91f, -1050.12f, 21.13f);
				vars.busWorkerBusModel = 431;
				RakBot::app()->log("[RAKBOT] ������ ������� ��������: ������������� ��-��");
				break;

			case 6:
				vars.busWorkerRouteItem = 2;
				DoCoordMaster(true, 1654.91f, -1050.12f, 21.13f);
				vars.busWorkerBusModel = 431;
				RakBot::app()->log("[RAKBOT] ������ ������� ��������: ������������� ��-��");
				break;

			case 7:
				vars.busWorkerRouteItem = 3;
				DoCoordMaster(true, 1654.91f, -1050.12f, 21.13f);
				vars.busWorkerBusModel = 431;
				RakBot::app()->log("[RAKBOT] ������ ������� ��������: ����������� ��-��");
				break;

			case 8:
				vars.busWorkerRouteItem = 4;
				DoCoordMaster(true, 1654.91f, -1050.12f, 21.13f);
				vars.busWorkerBusModel = 431;
				RakBot::app()->log("[RAKBOT] ������ ������� ��������: ����������� ��-�����");
				break;
		}
		return;
	}

	if (cmdcmp("setwork")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������� �� ������.\n"
				"������� \"!setwork\" ��� ��������� ����������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.iSetWorkIndex = std::strtoul(&cmd[8], nullptr, 10);

		if (!vars.iSetWorkIndex) {
			RakBot::app()->log("[RAKBOT] !setwork 1 - �������� ��������");
			RakBot::app()->log("[RAKBOT] !setwork 2 - �������");
			RakBot::app()->log("[RAKBOT] !setwork 3 - �������� ���-�����");
			RakBot::app()->log("[RAKBOT] !setwork 4 - ��������� ���������");
			RakBot::app()->log("[RAKBOT] !setwork 5 - �������");
			RakBot::app()->log("[RAKBOT] !setwork 6 - ����������");
			RakBot::app()->log("[RAKBOT] !setwork 7 - ������");
			RakBot::app()->log("[RAKBOT] !setwork 8 - ������");
			RakBot::app()->log("[RAKBOT] !setwork 9 - ������������");
			return;
		}

		RakBot::app()->log("[RAKBOT] ���������� �� ������ %d", vars.iSetWorkIndex);
		DoCoordMaster(true, 1480.00f, -1771.00f, 18.00f);
		return;
	}

	if (cmdcmp("flood")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� �������.\n"
				"������� \"!flood\" ��� ��������� ����������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		std::string str = cmd;
		std::smatch matches;
		std::regex_search(str, matches, std::regex("\\S+\\s+(\\d+)\\s+(\\d+)\\s+(.+)"));

		if (matches.size() != 4) {
			if (!vars.floodEnabled) {
				RakBot::app()->log("[RAKBOT] !flood 1 <��������> <�����> - ���� � ��� � ��������� ���������");
				RakBot::app()->log("[RAKBOT] !flood 2 <��������> <�����> - ���� � SMS � ��������� ���������");
				RakBot::app()->log("[RAKBOT] !flood - ��������� ������");
			} else {
				RakBot::app()->log("[RAKBOT] ������ ��������");
				vars.floodEnabled = 0;
			}
			return;
		}

		vars.floodMode = std::stoul(matches[1]);
		vars.floodDelay = std::stoul(matches[2]);
		vars.floodText = matches[3];
		Trim(vars.floodText);

		switch (vars.floodMode) {
			case 0:
				RakBot::app()->log("[RAKBOT] ������ ��������");
				vars.floodEnabled = 0;
				break;

			case 1:
				RakBot::app()->log("[RAKBOT] ������� ������ � ���");
				vars.floodEnabled = 1;
				break;

			case 2:
				RakBot::app()->log("[RAKBOT] ������� ������ � SMS");
				vars.floodEnabled = 1;
				break;
		}
		return;
	}

	if (cmdcmp("atm")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� ������� � ���������.\n"
				"������� \"!atm\" ��� ��������� ����������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!SampRpFuncs::isSampRpServer()) {
			RakBot::app()->log("[ERROR] ������ ������� ������������� ������ ��� �������� Samp-Rp.Ru");
			return;
		}

		int atmCity;

		if (sscanf(cmd, "%*s%d", &atmCity) < 1) {
			RakBot::app()->log("[RAKBOT] !atm 1 - ��������� ������� � ��������� ��");
			RakBot::app()->log("[RAKBOT] !atm 2 - ��������� ������� � ��������� ��");
			RakBot::app()->log("[RAKBOT] !atm 3 - ��������� ������� � ��������� ��");
			vars.getBalanceEnabled = false;
			return;
		}

		int sel = atmCity - 1;

		if (sel == -1) {
			vars.getBalanceEnabled = false;
			return;
		}

		float atm[3][3] = {
			{ 1919.98f, -1765.62f, 13.55f },
			{ -2032.73f, 159.74f, 29.04f },
			{ 2853.26f, 1286.76f, 11.39f }
		};

		DoCoordMaster(true, atm[sel][0], atm[sel][1], atm[sel][2]);
		vars.getBalanceEnabled = true;
		RakBot::app()->log("[RAKBOT] �������� � ���������...");
		return;
	}

	if (cmdcmp("coord")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������������ ���� �������������, ��������� 3 ���������, ������� �������� X, Y � Z ������������.\n"
				"��� ������� ����� ������������ �������!",
				"������",
				MB_ICONASTERISK);

			return;
		}

		float pos[3];
		if (sscanf(cmd, "%*s%f%f%f", &pos[0], &pos[1], &pos[2]) < 3) {
			RakBot::app()->log("[RAKBOT] CoordMaster: ������� ���������� ����� ������");
			return;
		}

		bool coordMasterToggle = (vars.coordMasterEnabled != true);
		DoCoordMaster(coordMasterToggle, pos[0], pos[1], pos[2]);
		return;
	}

	// ����������������
	if (cmdcmp("rejoin")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������������� � �������, �� ��������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		bot->disconnect(false);
		bot->reconnect(vars.reconnectDelay);
		return;
	}

	// ���������� ����� ����������
	if (cmdcmp("setrjtime")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� ������� �������� ����� ���������������� � �������.\n"
				"� �������� ��������� ��������� ����� �������� � ��������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int reconnectTime;

		if (sscanf(cmd, "%*s%d", &reconnectTime) < 1) {
			RakBot::app()->log("[RAKBOT] ����� ���������������: ������� ����� ���������������");
			return;
		}

		if (reconnectTime < 0) {
			return;
		}

		vars.reconnectDelay = reconnectTime * 1000;
		RakBot::app()->log("[RAKBOT] ����� ���������������: ����������� �������� %d ������", reconnectTime);
		return;
	}

	// ��������� ��������
	if (cmdcmp("sethp")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� ���������� �������� ����.\n"
				"� �������� ��������� ��������� ����� �����, ������ ���������� ���������� ��������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int health;
		if (sscanf(cmd, "%*s%d", &health) < 1) {
			RakBot::app()->log("[RAKBOT] ��������� ��������: ������� ������� ��������");
			return;
		}

		if (health < 0 || health > 100) {
			RakBot::app()->log("[RAKBOT] ������� �������� ������ ���� � ��������� �� 0 �� 100");
			return;
		}

		if (!bot->isSpawned())
			return;

		bot->setHealth(static_cast<uint8_t>(health));
		bot->sync();
		return;
	}

	// ������ �������
	if (cmdcmp("players")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ������ �������, �� ��������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!bot->isConnected()) {
			RakBot::app()->log("[RAKBOT] ������ �������: ��� �� ��������� � �������!");
			return;
		}

		RakBot::app()->log("[RAKBOT] === ������ ������� ===");
		RakBot::app()->log("[RAKBOT] ID * ��� * ���� * ���");

		if (bot->isConnected())
			RakBot::app()->log("[RAKBOT] %d * %s * %d * %d", bot->getPlayerId(), bot->getName().c_str(), bot->getInfo()->getPing(), bot->getInfo()->getScore());

		for (int i = 0; i < MAX_PLAYERS; i++) {
			Player *player = RakBot::app()->getPlayer(i);
			if (player == nullptr)
				continue;

			RakBot::app()->log("[RAKBOT] %d * %s * %d * %d", i, player->getName().c_str(), player->getInfo()->getPing(), player->getInfo()->getScore());
		}

		RakBot::app()->log("[RAKBOT] === [�����: %d] ===", RakBot::app()->getPlayersCount());

		return;
	}

	if (cmdcmp("quest")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ����������� ������, �� ��������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.bQuestEnabled ^= 1;
		if (vars.bQuestEnabled)
			RakBot::app()->log("[RAKBOT] ����������� ������ ��������. ��� ������ ������� �����");
		else
			RakBot::app()->log("[RAKBOT] ����������� ������ ���������");
		return;
	}

	if (cmdcmp("loadcars")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� �������� ����� ����� ��������, �� ��������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.botLoaderCheckVans ^= 1;
		if (vars.botLoaderCheckVans) {
			RakBot::app()->log("[RAKBOT] �������� �������� ����� ����� ��������");
		} else {
			RakBot::app()->log("[RAKBOT] ��������� �������� ����� ����� ��������");
		}
		return;
	}

	if (cmdcmp("loadcount")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� ������ ������ ���� �������� (����� ���������� �������� �� � �������� ������).\n"
				"� �������� ��������� ��������� ����� �����: ���������� ������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int limit = std::strtoul(&cmd[10], nullptr, 10);
		vars.botLoaderLimit = limit;
		RakBot::app()->log("[RAKBOT] ���������� ����� ������: %d", limit);
		return;
	}

	if (cmdcmp("picks")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ������ �������, �� ��������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int count = 0;
		RakBot::app()->log("[RAKBOT] === ������ ������� ===");
		RakBot::app()->log("[RAKBOT] ID * ������ * ��� * ���������� * �������");
		for (int i = 0; i < MAX_PICKUPS; i++) {
			Pickup *pickup = RakBot::app()->getPickup(i);
			if (pickup == nullptr)
				continue;

			RakBot::app()->log("[RAKBOT] %d * %d * %d * %.2f * (%.2f; %.2f; %.2f)",
				i, pickup->getModel(), pickup->getType(), bot->distanceTo(pickup),
				pickup->getPosition(0), pickup->getPosition(1), pickup->getPosition(2));
			count++;
		}
		RakBot::app()->log("[RAKBOT] === [�����: %d] ===", count);

		return;
	}

	if (cmdcmp("press")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� �������� ������� ������� ������. �������� �����:\n\n"
				"!press <ID �������> - ������� ������ �� ID\n(���������� �� http://wiki.sa-mp.com/wiki/Keys)\n\n"
				"!press Y - ������� ������� Y\n"
				"!press N - ������� ������� N\n"
				"!press H - ������� ������� �������\n"
				"!press S - ������� ������� ������\n"

				"!press U - ������� ������� �����\n"
				"!press R - ������� ������� ������\n"
				"!press D - ������� ������� ����\n"
				"!press L - ������� ������� �����\n",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!bot->isConnected()) {
			RakBot::app()->log("[ERROR] ������� �������: ��� ������ ���� ��������� � �������!");
			return;
		}

		uint8_t weapon = 0;
		uint16_t keys = 0;
		uint16_t upDownKey = 0;
		uint16_t leftRightKey = 0;

		switch (cmd[6]) {
			case 'Y':
				weapon = 64;
				RakBot::app()->log("[RAKBOT] ������� ������ Y...");
				break;

			case 'N':
				weapon = 128;
				RakBot::app()->log("[RAKBOT] ������� ������ N...");
				break;

			case 'H':
				weapon = 192;
				RakBot::app()->log("[RAKBOT] ������� ������ �������...");
				break;

			case 'S':
				keys = 8;
				RakBot::app()->log("[RAKBOT] ������� ������ ������...");
				break;

			case 'U':
				upDownKey = -128;
				RakBot::app()->log("[RAKBOT] ������� ������ �����...");
				break;

			case 'R':
				leftRightKey = 128;
				RakBot::app()->log("[RAKBOT] ������� ������ ������...");
				break;

			case 'D':
				upDownKey = 128;
				RakBot::app()->log("[RAKBOT] ������� ������ ����...");
				break;

			case 'L':
				leftRightKey = -128;
				RakBot::app()->log("[RAKBOT] ������� ������ �����...");
				break;

			default:
			{
				keys = static_cast<uint16_t>(std::strtoul(&cmd[6], nullptr, 10));

				if (keys == 0) {
					RakBot::app()->log("[RAKBOT] ������� ID �������!");
					return;
				}

				RakBot::app()->log("[RAKBOT] ������� ������ � ID: %d...", keys);
				break;
			}
		}

		bot->getKeys()->setKeyId(keys);
		bot->getKeys()->setLeftRightKey(leftRightKey);
		bot->getKeys()->setUpDownKey(upDownKey);

		uint8_t currentWeapon = bot->getWeapon();
		bot->setWeapon(weapon);
		bot->sync();

		RakBot::app()->getEvents()->defCallAdd(500, false, [bot, currentWeapon](DefCall *) {
			if (!bot->isConnected())
				return;

			bot->getKeys()->reset();
			bot->setWeapon(currentWeapon);
		});

		return;
	}

	if (cmdcmp("reqid")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� �������� ���������� ID, ����������� ��� ����� (���� �� �������� - ���������������, ���� �� ����� ���������).\n"
				"��������� 2 ���������: ����������� ID � ������������ ID ��������������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (sscanf(cmd, "%*s%d%d", &vars.minId, &vars.maxId) < 2) {
			RakBot::app()->log("[RAKBOT] �������� ID ���������");
			vars.checkIdEnabled = false;
		} else {
			RakBot::app()->log("[RAKBOT] �������� ID ��������: %d-%d", vars.minId, vars.maxId);
			vars.checkIdEnabled = true;
		}

		return;
	}

	if (cmdcmp("reqonline")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� �������� ���������� �������, ����������� ��� ����� (���� �� �������� - ���������������, ���� �� ����� ���������).\n"
				"��������� 2 ���������: ����������� ������ � ������������ ������ ��������������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (sscanf(cmd, "%*s%d%d", &vars.minOnline, &vars.maxOnline) < 2) {
			RakBot::app()->log("[RAKBOT] �������� ������� ���������");
			vars.checkOnlineEnabled = false;
		} else {
			RakBot::app()->log("[RAKBOT] �������� ������� ��������: %d-%d", vars.minOnline, vars.maxOnline);
			vars.checkOnlineEnabled = true;
		}
		return;
	}

	if (cmdcmp("instream")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ������� ����������, �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int playerCount = 0;

		RakBot::app()->log("[RAKBOT] ===================[������ �����]===================");

		RakBot::app()->log("[RAKBOT] %-3s * %-24s * %-3s * %-16s * %s", "ID", "���", "���", "������", "�������");

		for (int i = 0; i < MAX_PLAYERS; i++) {
			Player *player = RakBot::app()->getPlayer(i);
			if (player == nullptr)
				continue;

			if (!player->isInStream())
				continue;

			std::stringstream ss;
			ss << player->getPlayerStateName();

			if (player->getPlayerState() == PLAYER_STATE_DRIVER || player->getPlayerState() == PLAYER_STATE_PASSENGER)
				ss << "[" << player->getVehicle()->getVehicleId() << "]";

			char positionBuffer[64];
			snprintf(positionBuffer, sizeof(positionBuffer), "(%.2f; %.2f; %.2f)", player->getPosition(0), player->getPosition(1), player->getPosition(2));

			RakBot::app()->log("[RAKBOT] %-3d * %-24s * %-3d * %-16s * %s", i, player->getName().c_str(), player->getInfo()->getScore(), ss.str().c_str(), positionBuffer);
			playerCount++;
		}
		if (playerCount == 0) {
			RakBot::app()->log("[RAKBOT] ����� ��� ������� :(");
			RakBot::app()->log("[RAKBOT] ===================[������ �����]===================");
		} else {
			RakBot::app()->log("[RAKBOT] ====================[�����: %3d]====================", playerCount);
		}
		return;
	}

	if (cmdcmp("autopick")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ������������ �������, ����� ��� �������� �� ���. �� ��������� ���������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.autoPickEnabled ^= true;

		if (vars.autoPickEnabled) {
			RakBot::app()->log("[RAKBOT] ������������ �������: ��������");
		} else {
			RakBot::app()->log("[RAKBOT] ������������ �������: ���������");
		}

		return;
	}

	if (cmdcmp("timestamp")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ������ ������� � ���. �� ��������� ���������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.timeStamp ^= 1;
		if (vars.timeStamp) {
			RakBot::app()->log("[RAKBOT] ����� � ���� ��������");
		} else {
			RakBot::app()->log("[RAKBOT] ����� � ���� ���������");
		}
	}

	// INFO
	if (cmdcmp("info")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ���������� �� ������. � �������� ��������� ��������� ID ������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int playerId;

		if (sscanf(cmd, "%*s%d", &playerId) < 1) {
			RakBot::app()->log("[RAKBOT] ���������� �� ������: ������� ID");
			return;
		}

		if (playerId < 0 || playerId >= MAX_PLAYERS) {
			RakBot::app()->log("[RAKBOT] ���������� �� ������: �������� ID");
			return;
		}

		Player *player = RakBot::app()->getPlayer(playerId);
		if (player == nullptr) {
			RakBot::app()->log("[RAKBOT] ���������� �� ������: ����� �� ������!");
			return;
		}

		RakBot::app()->log("[RAKBOT] =====[���������� � ������]=====");
		RakBot::app()->log("[RAKBOT]  ���: %s    ID: %d", player->getName().c_str(), playerId);

		char szBuf[256];
		snprintf(szBuf, sizeof(szBuf), "[RAKBOT]  ����������: %f, %f, %f",
			player->getPosition(0), player->getPosition(1), player->getPosition(2));
		RakBot::app()->log(szBuf);

		RakBot::app()->log("[RAKBOT]  ��������: %d", player->getHealth());
		RakBot::app()->log("[RAKBOT]  �����: %d", player->getArmour());
		RakBot::app()->log("[RAKBOT]  ��������: %d", player->getAnimation()->getAnimId());
		RakBot::app()->log("[RAKBOT] =====[���������� � ������]=====");

		return;
	}

	if (cmdcmp("admins")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ������ �������, �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		RakBot::app()->log("[RAKBOT] =====[������ �������]=====");

		vars.adminsMutex.lock();
		size_t adminsCount = vars.admins.size();
		vars.adminsMutex.unlock();

		if (adminsCount == 0) {
			RakBot::app()->log("[RAKBOT] ��� ������� � ������");
			RakBot::app()->log("[RAKBOT] =====[������ �������]=====");
		} else {
			RakBot::app()->log("[RAKBOT] # * ���");
			for (int i = 0; i < static_cast<int>(adminsCount); i++) {
				vars.adminsMutex.lock();
				RakBot::app()->log("[RAKBOT] %d * %s", (i + 1), vars.admins[i].c_str());
				vars.adminsMutex.unlock();
			}
			RakBot::app()->log("[RAKBOT] ===========[�����: %d]===========", adminsCount);
		}
		return;
	}

	if (cmdcmp("admonline")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������������ �������� ��� ���������� ������ � ����. �� ��������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		switch (vars.adminOnlineAction) {
			case 0:
				RakBot::app()->log("[RAKBOT] ���������������, ���� ����� � ����");
				vars.adminOnlineAction = 1;
				break;

			case 1:
				RakBot::app()->log("[RAKBOT] �����, ���� ����� � ����");
				vars.adminOnlineAction = 2;
				break;

			case 2:
				RakBot::app()->log("[RAKBOT] �����������, ���� ����� � ����");
				vars.adminOnlineAction = 0;
				break;
		}

		return;
	}

	if (cmdcmp("admnear")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������������ �������� ��� ���������� ������ �����. �� ��������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		switch (vars.adminNearAction) {
			case 0:
				RakBot::app()->log("[RAKBOT] ���������������, ���� ����� �����");
				vars.adminNearAction = 1;
				break;

			case 1:
				RakBot::app()->log("[RAKBOT] �����, ���� ����� �����");
				vars.adminNearAction = 2;
				break;

			case 2:
				RakBot::app()->log("[RAKBOT] �����������, ���� ����� �����");
				vars.adminNearAction = 0;
				break;
		}

		return;
	}

	// GOTO
	if (cmdcmp("goto")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� � ������. � �������� ��������� ��������� ID ������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!bot->isSpawned())
			return;

		int playerId;

		if (sscanf(cmd, "%*s%d", &playerId) < 1) {
			RakBot::app()->log("[RAKBOT] �������� � ������: ������� ID");
			return;
		}

		if (playerId < 0 || playerId > MAX_PLAYERS) {
			RakBot::app()->log("[RAKBOT] �������� � ������: �������� ID");
			return;
		}

		Player *player = RakBot::app()->getPlayer(playerId);
		if (player == nullptr) {
			RakBot::app()->log("[RAKBOT] �������� � ������: ����� �� ������!");
			return;
		}

		if (player->getPosition(0) == 0.f
			&& player->getPosition(1) == 0.f
			&& player->getPosition(2) == 0.f) {
			RakBot::app()->log("[RAKBOT] �������� � ������: �������������� ������ ����������", playerId);
			return;
		}

		bot->teleport(player->getPosition(0), player->getPosition(1), player->getPosition(2));

		RakBot::app()->log("[RAKBOT] �������� � ������: �������������� � ������ %s[%d]", player->getName().c_str(), playerId);
		return;
	}

	// ������ �����������
	if (cmdcmp("vlist")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ������� ������������ �������. �� ��������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int vehicleModel = 0;

		if (sscanf(cmd, "%*s%d", &vehicleModel) == 1) {
			if (vehicleModel < 400 || vehicleModel > 611) {
				RakBot::app()->log("[RAKBOT] ����� ���������� ������ ���� ����� �� 400 �� 611");
				return;
			}
		}

		int cmd_count = 0;

		if (vehicleModel)
			RakBot::app()->log("[RAKBOT] =====[��������� ������ %d]=====", vehicleModel);
		else
			RakBot::app()->log("[RAKBOT] =====[���������]=====");

		RakBot::app()->log("[RAKBOT] ID * �������� * ������ * ���� * ��������� * �������");
		for (uint16_t i = 0; i < MAX_VEHICLES; i++) {
			Vehicle *vehicle = RakBot::app()->getVehicle(i);
			if (vehicle == nullptr)
				continue;

			if (vehicleModel && vehicle->getModel() == vehicleModel || !vehicleModel) {
				RakBot::app()->log("[RAKBOT] %d * %s * %s * %d/%d * %.2f * (%.2f; %.2f; %.2f)",
					i, vehicle->getName().c_str(), vehicle->isDoorsOpened() ? "�������" : "�������",
					vehicle->getFirstColor(), vehicle->getSecondColor(), bot->distanceTo(vehicle),
					vehicle->getPosition(0), vehicle->getPosition(1), vehicle->getPosition(2));
				cmd_count++;
			}
		}

		if (cmd_count)
			RakBot::app()->log("[RAKBOT] =====[�����: %d]=====", cmd_count);
		else
			RakBot::app()->log("[RAKBOT] =====[�����: 0]=====");

		return;
	}

	if (cmdcmp("toplace")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� � ������������ � ����� �����. � �������� ��������� ��������� ����� �����.\n"
				"����� ����� ���������� � ������� ������� \"!places\"",
				"������",
				MB_ICONASTERISK);

			return;
		}

		char szBuf[512];

		if (FILE *f = fopen(GetRakBotPath("places.txt").c_str(), "r")) {
			for (int i = 0; i < 300; i++) {
				if (fgets(szBuf, 128, f)) {
					strcpy(vars.teleportPlaces[i].szName, strtok(szBuf, "|"));

					for (int n = 0; n < 3; n++)
						vars.teleportPlaces[i].position[n] = std::strtof(strtok(NULL, "|"), nullptr);
				}
			}
			fclose(f);

			int iPlaceIndex = std::strtoul(&cmd[8], nullptr, 10);
			float *pos = vars.teleportPlaces[iPlaceIndex - 1].position;
			DoCoordMaster(true, pos[0], pos[1], pos[2]);

			sprintf(szBuf, "[RAKBOT] CoordMaster: �������� �� ���������� (%0.2f; %0.2f; %0.2f)", vars.coordMasterTarget[0], vars.coordMasterTarget[1], vars.coordMasterTarget[2]);
			RakBot::app()->log(szBuf);
		}

		return;
	}

	if (cmdcmp("places")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ����, ����������� � ����� \"places.txt\". �� ��������� ����������.\n"
				"������ ������ ����:\n\n"
				"��������|X|Y|Z\n"
				"����� 1|10,0|-12,5|5,3",
				"������",
				MB_ICONASTERISK);
			return;
		}

		char buf[128];
		if (FILE *f = fopen(GetRakBotPath("places.txt").c_str(), "r")) {
			for (int i = 0; i < 300; i++) {
				if (fgets(buf, 128, f)) {
					strcpy(vars.teleportPlaces[i].szName, strtok(buf, "|"));

					for (int n = 0; n < 3; n++)
						vars.teleportPlaces[i].position[n] = std::strtof(strtok(NULL, "|"), nullptr);
				}
			}
			fclose(f);

			for (int i = 0; i < 300; i++) {
				if (vars.teleportPlaces[i].szName[0] != NULL)
					RakBot::app()->log("[RAKBOT] !toplace %d - %s\n", i + 1, vars.teleportPlaces[i].szName);
			}
		}

		return;
	}

	// ������� � ����������
	if (cmdcmp("seat")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������� ���� � ���������. � �������� ��������� ��������� ID ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!bot->isSpawned())
			return;

		int vehicleId;

		if (sscanf(cmd, "%*s%d", &vehicleId) < 1) {
			RakBot::app()->log("[RAKBOT] ������� � ������: ������� ID ������");
			return;
		}

		Vehicle *vehicle = RakBot::app()->getVehicle(vehicleId);
		bot->enterVehicle(vehicle, 0);
		return;
	}

	if (cmdcmp("pseat")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������� ���� � ��������� �� ������������ �����. � �������� ��������� ��������� ID ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!bot->isSpawned())
			return;

		int vehicleId;

		if (sscanf(cmd, "%*s%d", &vehicleId) < 1) {
			RakBot::app()->log("[RAKBOT] ������� � ������: ������� ID ������");
			return;
		}

		Vehicle *vehicle = RakBot::app()->getVehicle(vehicleId);
		bot->enterVehicle(vehicle, 1);
		RakBot::app()->log("[RAKBOT] ������� � ������ � ID %d", vehicleId);
		return;
	}

	if (cmdcmp("antisat")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ����������. �� ��������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.bAntiSat ^= 1;
		if (vars.bAntiSat)
			RakBot::app()->log("[RAKBOT] ������������ ��������");
		else
			RakBot::app()->log("[RAKBOT] ������������ ���������");
	}

	if (cmdcmp("exit")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ �� ����������. �� ��������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!bot->isSpawned())
			return;

		if (bot->getPlayerState() != PLAYER_STATE_DRIVER
			&& bot->getPlayerState() != PLAYER_STATE_PASSENGER) {
			RakBot::app()->log("[RAKBOT] ��� �� ��������� � ����������");
			return;
		}

		bot->exitVehicle();
		RakBot::app()->log("[RAKBOT] ��� ����� �� ����������");
		return;
	}

	if (cmdcmp("farm")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ������ �� �����. � �������� ��������� �������� ID �����.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		FarmIndex = std::strtoul(&cmd[5], nullptr, 10);
		DoCoordMaster(true, FarmPos[FarmIndex][0], FarmPos[FarmIndex][1], FarmPos[FarmIndex][2]);

		vars.botFarmerEnabled = 1;
		vars.botFarmerAutomated = 1;
		FarmWork = 0;
		RakBot::app()->log("[RAKBOT] �������� �� ����� %d", FarmIndex);

		return;
	}

	// ������� ����� � ����
	if (cmdcmp("menu")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ������ ����. � �������� ��������� ��������� ����� ������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		RakNet::BitStream bsSend;
		bsSend.Write<int>(std::strtoul(&cmd[5], nullptr, 10));
		rakClient->RPC(&RPC_MenuSelect, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

		return;
	}

	if (cmdcmp("bank")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������� ������� � �����. � �������� ��������� ��������� ����� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int moneyAmount;

		if (sscanf(cmd, "%*s%d", &moneyAmount) < 1) {
			RakBot::app()->log("[RAKBOT] ������� ����� ��� ���������� ����� � �����");
			return;
		}

		DoCoordMaster(true, 1414.69f, -1700.48f, 13.54f);

		vars.iBankPutMoney = moneyAmount;
		RakBot::app()->log("[RAKBOT] �������������� ���������� ������� �� %d ����", vars.iBankPutMoney);
		return;
	}

	if (cmdcmp("parsenicks")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� �������� ������� � ����. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int minLvl;

		if (cmd[11] == ' ' || cmd[11] == '\0') {
			minLvl = 0;
		} else {
			minLvl = atol(&cmd[11]);
		}

		FILE *fOut = fopen(GetRakBotPath("nicks.txt").c_str(), "w");

		int iCount = 0;
		for (int i = 0; i < MAX_PLAYERS; i++) {
			Player *player = RakBot::app()->getPlayer(i);
			if (player == nullptr)
				continue;

			if (player->getInfo()->getScore() < minLvl)
				continue;

			fprintf(fOut, "%s|%d\n", player->getName().c_str(), player->getInfo()->getScore());
			iCount++;
		}

		fclose(fOut);

		if (iCount == 0)
			RakBot::app()->log("[RAKBOT] ������ �����: ������ �� ��������� � ������� ��� �� ������� ��� �������");
		else
			RakBot::app()->log("[RAKBOT] ������ �����: %d ������� �������� � ���� \"nicks.txt\" � �������� ����� ����", iCount);
		return;
	}

	if (cmdcmp("follow")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������� �������� �� �������. � �������� ��������� ��������� ID ������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (vars.stickEnabled) {
			RakBot::app()->log("[RAKBOT] �������������: ������� ��������� �������");
			RakBot::app()->log("[RAKBOT] �������������: ���������");
			vars.followEnabled = false;
			return;
		}

		int playerId;

		if (sscanf(cmd, "%*s%d", &playerId) < 1) {
			RakBot::app()->log("[RAKBOT] �������������: ������� ID ������");
			RakBot::app()->log("[RAKBOT] �������������: ���������");
			vars.followEnabled = false;
			return;
		}

		if (playerId < 0 || playerId >= MAX_PLAYERS) {
			RakBot::app()->log("[RAKBOT] �������������: ������� ���������� ID ������");
			RakBot::app()->log("[RAKBOT] �������������: ���������");
			vars.followEnabled = false;
			return;
		}

		Player *player = RakBot::app()->getPlayer(playerId);
		if (player == nullptr) {
			RakBot::app()->log("[RAKBOT] �������������: ����� %d �� ������", vars.followPlayerID);
			RakBot::app()->log("[RAKBOT] �������������: ���������");
			vars.followEnabled = false;
			return;
		}

		if (!player->isInStream()) {
			RakBot::app()->log("[RAKBOT] �������������: ����� %d �� �����", vars.followPlayerID);
			RakBot::app()->log("[RAKBOT] �������������: ���������");
			vars.followEnabled = false;
			return;
		}

		vars.followPlayerID = playerId;
		vars.followEnabled ^= true;

		if (vars.followEnabled)
			RakBot::app()->log("[RAKBOT] �������������: ��������. �����: %s[%d]", player->getName().c_str(), vars.followPlayerID);
		else
			RakBot::app()->log("[RAKBOT] �������������: �����������");

		return;
	}

	if (cmdcmp("stick")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� �������. � �������� ��������� ��������� ID ������",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (vars.followEnabled) {
			RakBot::app()->log("[RAKBOT] �������: ������� ��������� �������������");
			RakBot::app()->log("[RAKBOT] �������: ���������");
			vars.stickEnabled = false;
			return;
		}

		int playerId;

		if (sscanf(cmd, "%*s%d", &playerId) < 1) {
			RakBot::app()->log("[RAKBOT] �������: ������� ID ������");
			RakBot::app()->log("[RAKBOT] �������: ���������");
			vars.stickEnabled = false;
			return;
		}

		if (playerId < 0 || playerId >= MAX_PLAYERS) {
			RakBot::app()->log("[RAKBOT] �������: ������� ���������� ID ������");
			RakBot::app()->log("[RAKBOT] �������: ���������");
			vars.stickEnabled = false;
			return;
		}

		Player *player = RakBot::app()->getPlayer(playerId);
		if (player == nullptr) {
			RakBot::app()->log("[RAKBOT] �������������: ����� %d �� ������", vars.followPlayerID);
			RakBot::app()->log("[RAKBOT] �������������: ���������");
			vars.followEnabled = false;
			return;
		}

		if (!player->isInStream()) {
			RakBot::app()->log("[RAKBOT] �������������: ����� %d �� �����", vars.followPlayerID);
			RakBot::app()->log("[RAKBOT] �������������: ���������");
			vars.followEnabled = false;
			return;
		}

		vars.followPlayerID = playerId;
		vars.stickEnabled ^= true;

		if (vars.stickEnabled)
			RakBot::app()->log("[RAKBOT] �������: ��������. �����: %s[%d]", player->getName().c_str(), vars.followPlayerID);
		else
			RakBot::app()->log("[RAKBOT] �������: �����������");

		return;
	}

	if (cmdcmp("virtualworld")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ������������ ����. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!SampRpFuncs::isSampRpServer()) {
			RakBot::app()->log("[ERROR] ������ ������� ������������� ������ ��� �������� Samp-Rp.Ru");
			return;
		}

		vars.virtualWorld ^= 1;
		if (vars.virtualWorld) {
			bot->requestClass(rand() % 299);
			RakBot::app()->log("[RAKBOT] ����������� ���: �������");
		} else {
			RakBot::app()->log("[RAKBOT] ����������� ���: ��������");
		}
		return;
	}

	if (cmdcmp("botloader")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ���� ��������. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!SampRpFuncs::isSampRpServer()) {
			RakBot::app()->log("[ERROR] ������ ������� ������������� ������ ��� �������� Samp-Rp.Ru");
			return;
		}

		vars.botLoaderEnabled ^= true;
		if (vars.botLoaderEnabled) {
			DoCoordMaster(true, 2126.78f, -2281.03f, 24.88f);
			RakBot::app()->log("[RAKBOT] ��� ��������: �������");
		} else {
			RakBot::app()->log("[RAKBOT] ��� ��������: ��������");
			vars.sendBadSync = false;
		}
		return;
	}

	if (cmdcmp("skipdialog")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� �������� ��������. ���������: ������� \"!skipdialog\"",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int skipDialogAction;

		if (sscanf(cmd, "%*s%d", &skipDialogAction) < 1) {
			RakBot::app()->log("[RAKBOT] ������� ��������: ������� ��������");
			RakBot::app()->log("[RAKBOT]   0 - ���������� ����������� ��������");
			RakBot::app()->log("[RAKBOT]   1 - �������� ENTER");
			RakBot::app()->log("[RAKBOT]   2 - �������� ESC");
			RakBot::app()->log("[RAKBOT]   3 - ������ �������������");
			return;
		}

		if (skipDialogAction < 0 || skipDialogAction > 3) {
			RakBot::app()->log("[RAKBOT] ������� ��������: ������� ������ ��������");
			RakBot::app()->log("[RAKBOT]   0 - ���������� ����������� ��������");
			RakBot::app()->log("[RAKBOT]   1 - �������� ENTER");
			RakBot::app()->log("[RAKBOT]   2 - �������� ESC");
			RakBot::app()->log("[RAKBOT]   3 - ������ �������������");
			return;
		}

		vars.skipDialog = skipDialogAction;

		switch (vars.skipDialog) {
			case 0:
				RakBot::app()->log("[RAKBOT] ������� ��������: ���������� ����������� ��������");
				break;

			case 1:
				RakBot::app()->log("[RAKBOT] ������� ��������: �������� ENTER");
				break;

			case 2:
				RakBot::app()->log("[RAKBOT] ������� ��������: �������� ESC");
				break;

			case 3:
				RakBot::app()->log("[RAKBOT] ������� ��������: ������������� ��������");
				break;
		}

		return;
	}

	if (cmdcmp("whereiscp")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������ ���������. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (checkpoint.active || raceCheckpoint.active) {
			float *position = (checkpoint.active) ? (checkpoint.position) : (raceCheckpoint.position);
			RakBot::app()->log("[RAKBOT] ������� ���������: (%.2f; %.2f; %.2f)!", position[0], position[1], position[2]);
		} else {
			RakBot::app()->log("[RAKBOT] ����� ���������: �������� ���������!");
		}
		return;
	}

	if (cmdcmp("takecp")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� �������� ���������. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		bot->takeCheckpoint();
		return;
	}

	if (cmdcmp("cpm")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� �������� �������. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.checkPointMaster ^= 1;
		if (vars.checkPointMaster) {
			RakBot::app()->log("[RAKBOT] �������� ������ �������");
		} else {
			RakBot::app()->log("[RAKBOT] �������� ������ ��������");
		}
		return;
	}

	if (cmdcmp("antideath")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ���������� (��� �� ��������� ��� 0 ��). �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.antiDeath ^= 1;
		if (vars.antiDeath) {
			RakBot::app()->log("[RAKBOT] ���������� ��������");
		} else {
			RakBot::app()->log("[RAKBOT] ���������� ���������");
		}
		return;
	}

	if (cmdcmp("botfarmer")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ���� ������� (���������� ��� �� ����, ��� ������ ������������� ����������� \"!farm\"). �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.botFarmerEnabled ^= 1;
		FarmWork = vars.botFarmerEnabled;

		if (vars.botFarmerEnabled) {
			RakBot::app()->log("[RAKBOT] ��� ������� �������");
		} else {
			RakBot::app()->log("[RAKBOT] ��� ������� ��������");
			vars.botFarmerAutomated = 0;
			vars.sendBadSync = false;
		}

		return;
	}

	if (cmdcmp("parsestat")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ����� ���������� �� �������� (������ Samp-Rp). �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!SampRpFuncs::isSampRpServer()) {
			RakBot::app()->log("[ERROR] ������ ������� ������������� ������ ��� �������� Samp-Rp.Ru");
			return;
		}

		vars.parseStatistic = true;
		bot->sendInput("/mm");
		return;
	}

	if (cmdcmp("map")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������/�������� �����. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!vars.mapWindowOpened) {
			ShowMapWindow();
		} else {
			CloseMapWindow();
		}
		return;
	}

	if (cmdcmp("farchat")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� �������� ���� (��������� ��������� ��������� ��������� ����). �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.farChatEnabled ^= 1;
		if (vars.farChatEnabled) {
			RakBot::app()->log("[RAKBOT] ������� ��� ��������");
		} else {
			RakBot::app()->log("[RAKBOT] ������� ��� �������");
		}
		return;
	}

	if (cmdcmp("skipmsg")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� �������� ��������� ������� � ����. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.ignoreServerMessages ^= 1;
		if (vars.ignoreServerMessages) {
			RakBot::app()->log("[RAKBOT] ������� ��������� ������� �������");
		} else {
			RakBot::app()->log("[RAKBOT] ������� ��������� ������� ��������");
		}
		return;
	}

	if (cmdcmp("badpos")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ���������� �������������. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.sendBadSync ^= 1;
		if (vars.sendBadSync) {
			RakBot::app()->log("[RAKBOT] ���������� ������� ��������");
		} else {
			RakBot::app()->log("[RAKBOT] ���������� ������� ���������");
		}
		return;
	}

	if (cmdcmp("smartinvis")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������/���������� ���������. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		vars.smartInvis ^= 1;
		if (vars.smartInvis) {
			RakBot::app()->log("[RAKBOT] SmartInvis by 0x32789 (http://ugbase.eu) �������");
		} else {
			RakBot::app()->log("[RAKBOT] SmartInvis by 0x32789 (http://ugbase.eu) ��������");
		}
		return;
	}

	if (cmdcmp("autoschool")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������/���������� ����� �� �����. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!SampRpFuncs::isSampRpServer()) {
			RakBot::app()->log("[ERROR] ������ ������� ������������� ������ ��� �������� Samp-Rp.Ru");
			return;
		}

		vars.botAutoSchoolEnabled ^= true;
		if (vars.botAutoSchoolEnabled) {
			DoCoordMaster(true, -2026.00f, -101.00f, 35.00f);
			RakBot::app()->log("[RAKBOT] ������ ����� �������� �� �����. �������� � ���������...");
		} else {
			RakBot::app()->log("[RAKBOT] ����� �������� �� ����� ������������� ���������");
			vars.botAutoSchoolActive = 0;
		}
		return;
	}

	if (cmdcmp("botkill")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� �������� ����. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		if (!bot->isSpawned())
			return;

		bot->kill();
		return;
	}

	if (cmdcmp("debug")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ������� ���� �������. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		AllocConsole();
		SetConsoleTitle("RakBot Debug");
		SetConsoleCP(1251);
		SetConsoleOutputCP(1251);

		freopen("CONOUT$", "w", stdout);
		freopen("CONIN$", "r", stdin);

		printf("* ===================================================== *\n");
		printf("  RakBot " RAKBOT_VERSION " debug console inited\n");
		printf("  RakBot.Ru\n");
		printf("* ===================================================== *\n");
		return;
	}


	if (cmdcmp("tdclick")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ����� �� ����������. � �������� ��������� ��������� ID ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int textDrawId;

		if (sscanf(cmd, "%*s%d", &textDrawId) < 1) {
			RakBot::app()->log("[RAKBOT] ���� ����������: ������� ID");
			return;
		}

		bot->clickTextdraw(textDrawId);
		RakBot::app()->log("[RAKBOT] ���� ����������: ������� ��������� � ID %d", textDrawId);
		return;
	}

	if (cmdcmp("setnick")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� ���� ����. � �������� ��������� ��������� ����� ��� (�� 20 ��������).",
				"������",
				MB_ICONASTERISK);

			return;
		}

		char nickName[25];

		if (sscanf(cmd, "%*s%s", nickName) < 1) {
			RakBot::app()->log("[RAKBOT] ��������� ����: ������� ����� ���");
			return;
		}

		RakBot::app()->getSettings()->setName(nickName);
		RakBot::app()->log("[RAKBOT] ��������� ����: ���������� ��� - %s", RakBot::app()->getSettings()->getName().c_str());
		return;
	}

	if (cmdcmp("setpass")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� ������ ����. � �������� ��������� ��������� ����� ������ (�� 120 ��������).",
				"������",
				MB_ICONASTERISK);

			return;
		}

		char loginPass[128];

		if (sscanf(cmd, "%*s%s", loginPass) < 1) {
			RakBot::app()->log("[RAKBOT] ��������� ������: ������� ����� ������");
			return;
		}

		RakBot::app()->getSettings()->setLoginPassword(loginPass);
		RakBot::app()->log("[RAKBOT] ��������� ������: ���������� ������ - %s",
			RakBot::app()->getSettings()->getLoginPassword());
		return;
	}

	if (cmdcmp("setip")) {

		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� IP �������. � �������� ��������� ��������� ����� IP (������ �����:����).",
				"������",
				MB_ICONASTERISK);

			return;
		}

		char fullAddr[256];

		char addrName[256];
		int addrPort;

		if (sscanf(cmd, "%*s%s", fullAddr) < 1) {
			RakBot::app()->log("[RAKBOT] ��������� �������: ������� ����� ����� � ������� IP:PORT");
			return;
		}

		char *pch = fullAddr;
		while (*pch) {
			if (*pch == ':') {
				*pch = 0;
				addrPort = std::strtoul(pch + 1, nullptr, 10);
			}
			pch++;
		}
		strcpy(addrName, fullAddr);

		if (strlen(addrName) < 1 || addrPort < 0 || addrPort > 65535) {
			RakBot::app()->log("[RAKBOT] ��������� �������: �������� ������ ������");
			return;
		}

		RakBot::app()->getSettings()->getAddress()->setIp(addrName);
		RakBot::app()->getSettings()->getAddress()->setPort(static_cast<uint16_t>(addrPort));
		RakBot::app()->log("[RAKBOT] ��������� �������: ���������� ������ - %s:%d",
			RakBot::app()->getSettings()->getAddress()->getIp().c_str(),
			RakBot::app()->getSettings()->getAddress()->getPort());
		return;
	}

	if (cmdcmp("setweapon")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ��������� ������. � �������� ��������� ��������� ID ������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		int weaponId;

		if (sscanf(cmd, "%*s%d", &weaponId) < 1) {
			RakBot::app()->log("[RAKBOT] ������: ������� ID ������");
			return;
		}

		bot->setWeapon(weaponId);
		RakBot::app()->log("[RAKBOT] ������: ����������� ������ � ID %d", weaponId);
		bot->sync();
		return;
	}

	if (cmdcmp("openlog")) {
		char dir[MAX_PATH];
		GetModuleFileName(NULL, dir, sizeof(dir));
		*strrchr(dir, '\\') = 0;

		std::stringstream ss;
		ss << "\"" << dir << "\\logs\\"
			<< RakBot::app()->getSettings()->getAddress()->getIp() << ";" << RakBot::app()->getSettings()->getAddress()->getPort() << "\\"
			<< RakBot::app()->getSettings()->getName() << ".log\"";
		std::string filePath = "/select," + ss.str();

		RakBot::app()->log("[RAKBOT] ���� ����: %s", ss.str().c_str());
		ShellExecute(g_hWndMain, "open", "explorer.exe", filePath.c_str(), dir, SW_SHOWMAXIMIZED);
		return;
	}

	if (cmdcmp("saveakk")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� ���������� �������� (���������� ������ � ��������). �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		HRESULT hResult;
		IShellLink *pShellLink = NULL;
		IPersistFile *pPersistFile = NULL;

		char szAddress[32];
		_snprintf_s(szAddress, sizeof(szAddress), "%s;%d",
			RakBot::app()->getSettings()->getAddress()->getIp().c_str(),
			RakBot::app()->getSettings()->getAddress()->getPort());

		CoInitialize(NULL);
		hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&pShellLink);
		CoUninitialize();

		if (FAILED(hResult)) {
			if (pShellLink)
				pShellLink->Release();

			RakBot::app()->log("[ERROR] ������ ��� ���������� ��������!");
			return;
		}

		hResult = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);

		if (FAILED(hResult)) {
			if (pPersistFile)
				pPersistFile->Release();
			if (pShellLink)
				pShellLink->Release();

			RakBot::app()->log("[ERROR] ������ ��� ���������� ��������!");
			return;
		}

		char szLauncherPath[MAX_PATH];
		GetModuleFileName(NULL, szLauncherPath, sizeof(szLauncherPath));
		*strrchr(szLauncherPath, '\\') = 0;
		strcat(szLauncherPath, "\\RakLaunch.exe");
		hResult = pShellLink->SetPath(szLauncherPath);

		if (FAILED(hResult)) {
			if (pPersistFile)
				pPersistFile->Release();
			if (pShellLink)
				pShellLink->Release();

			RakBot::app()->log("[ERROR] ������ ��� ���������� ��������!");
			return;
		}

		char szAccountsPath[MAX_PATH];
		GetModuleFileName(NULL, (char *)&szAccountsPath, sizeof(szAccountsPath));
		strcpy(strrchr(szAccountsPath, '\\') + 1, "accounts");
		CreateDirectory(szAccountsPath, NULL);

		char szAccountsDataPath[MAX_PATH];
		strcpy(szAccountsDataPath, szAccountsPath);
		strcat(szAccountsDataPath, "\\data");
		CreateDirectory(szAccountsDataPath, NULL);
		SetFileAttributes(szAccountsDataPath, FILE_ATTRIBUTE_HIDDEN);

		char szAccountData[MAX_PATH];
		_snprintf(szAccountData, sizeof(szAccountData), "%s\\%s@%s.ini", szAccountsDataPath, RakBot::app()->getSettings()->getName().c_str(), szAddress);
		SaveConfig(szAccountData);

		hResult = pShellLink->SetArguments(szAccountData);

		if (FAILED(hResult)) {
			if (pPersistFile)
				pPersistFile->Release();
			if (pShellLink)
				pShellLink->Release();

			RakBot::app()->log("[ERROR] ������ ��� ���������� ��������!");
			return;
		}

		char szAccountLink[MAX_PATH];
		_snprintf(szAccountLink, sizeof(szAccountLink), "%s\\%s@%s.lnk", szAccountsPath, RakBot::app()->getSettings()->getName().c_str(), szAddress);

		wchar_t wszAccountLink[MAX_PATH];
		MultiByteToWideChar(CP_ACP, 0, szAccountLink, -1, wszAccountLink, MAX_PATH);

		hResult = pPersistFile->Save(wszAccountLink, TRUE);

		if (FAILED(hResult)) {
			if (pPersistFile)
				pPersistFile->Release();
			if (pShellLink)
				pShellLink->Release();

			RakBot::app()->log("[ERROR] ������ ��� ���������� ��������!");
			return;
		}

		RakBot::app()->log("[RAKBOT] ������� ������� ��������");
		return;
	}

	if (cmdcmp("delakk")) {
		if (strstr(cmd, "+help")) {
			MessageBox(
				g_hWndMain,
				"������� �������� ������ �������� ��������. �� ������� ����������.",
				"������",
				MB_ICONASTERISK);

			return;
		}

		char szAddress[32];
		_snprintf_s(szAddress, sizeof(szAddress), "%s;%d",
			RakBot::app()->getSettings()->getAddress()->getIp().c_str(),
			RakBot::app()->getSettings()->getAddress()->getPort());

		char szAccountsPath[MAX_PATH];
		GetModuleFileName(NULL, (char *)&szAccountsPath, sizeof(szAccountsPath));
		strcpy(strrchr(szAccountsPath, '\\') + 1, "accounts");
		CreateDirectory(szAccountsPath, NULL);

		char szAccountsDataPath[MAX_PATH];
		strcpy(szAccountsDataPath, szAccountsPath);
		strcat(szAccountsDataPath, "\\data");
		CreateDirectory(szAccountsDataPath, NULL);
		SetFileAttributes(szAccountsDataPath, FILE_ATTRIBUTE_HIDDEN);

		char szAccountData[MAX_PATH];
		_snprintf(szAccountData, sizeof(szAccountData), "%s\\%s@%s.ini", szAccountsDataPath, RakBot::app()->getSettings()->getName().c_str(), szAddress);

		char szAccountLink[MAX_PATH];
		_snprintf(szAccountLink, sizeof(szAccountLink), "%s\\%s@%s.lnk", szAccountsPath, RakBot::app()->getSettings()->getName().c_str(), szAddress);

		if (!DeleteFile(szAccountData)) {
			RakBot::app()->log("[ERROR] ������ �������� ������ ������������ ��������");
			return;
		}

		if (!DeleteFile(szAccountLink)) {
			RakBot::app()->log("[ERROR] ������ �������� ������ ������������ ��������");
			return;
		}

		RakBot::app()->log("[RAKBOT] ����� ������� ������");
		return;
	}

	if (cmdcmp("adapters")) {
		IP_ADAPTER_INFO * AdapterInfo;
		ULONG    ulOutBufLen;
		DWORD    dwRetVal;
		IP_ADDR_STRING * pIPAddr;

		AdapterInfo = (IP_ADAPTER_INFO *)GlobalAlloc(GPTR, sizeof(IP_ADAPTER_INFO));
		ulOutBufLen = sizeof(IP_ADAPTER_INFO);

		if (ERROR_BUFFER_OVERFLOW == GetAdaptersInfo(AdapterInfo, &ulOutBufLen)) {
			GlobalFree(AdapterInfo);
			AdapterInfo = (IP_ADAPTER_INFO *)GlobalAlloc(GPTR, ulOutBufLen);
		}

		if (dwRetVal = GetAdaptersInfo(AdapterInfo, &ulOutBufLen)) {
			RakBot::app()->log("[ERROR] ������ ��������� ���������� ��������.");
		} else {
			while (AdapterInfo != NULL) {
				RakBot::app()->log("[RAKBOT] �������: %s", AdapterInfo->Description);

				printf("IP Address(s):\n");
				RakBot::app()->log("[RAKBOT]     IP: %s", AdapterInfo->IpAddressList.IpAddress.String);

				pIPAddr = AdapterInfo->IpAddressList.Next;
				while (pIPAddr) {
					RakBot::app()->log("[RAKBOT]     IP: %s", pIPAddr->IpAddress.String);
					pIPAddr = pIPAddr->Next;
				}
				AdapterInfo = AdapterInfo->Next;
			}
		}
		return;
	}

	if (cmdcmp("showdialog")) {
		RakBot::app()->getSampDialog()->showDialog();
		return;
	}

	if (cmdcmp("crash")) {
		int *test = 0;
		*test = 1337;
		return;
	}

	RakBot::app()->log("[ERROR] ������� \"!%s\" �� ���� �������", cmd);
}
