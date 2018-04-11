#include "StdAfx.h"

#include "RakBot.h"
#include "RakNet.h"
#include "PlayerBase.h"
#include "Player.h"
#include "Bot.h"
#include "Settings.h"
#include "Pickup.h"
#include "Script.h"
#include "Server.h"
#include "Events.h"

#include "AnimStuff.h"
#include "MathStuff.h"
#include "Funcs.h"

#include "netrpc.h"

#include "SampRpFuncs.h"

bool SampRpFuncs::_botSuspended = false;

SampRpFuncs::SampRpFuncs() {}

SampRpFuncs::~SampRpFuncs() {}

void SampRpFuncs::takeCheckpoint() {
	static bool takeCheckpointReady = true;

	if (!checkpoint.active && !raceCheckpoint.active)
		return;

	float *position = checkpoint.active ? checkpoint.position : raceCheckpoint.position;
	Bot *bot = RakBot::app()->getBot();

	if (bot->distanceTo(position) >= 120.0f) {
		RakBot::app()->log("[RAKBOT] ���������� �� ��������� ������ 120 ������!");
		return;
	}

	while (!takeCheckpointReady)
		return;
	takeCheckpointReady = false;

	_botSuspended = true;
	bot->setPosition(0, position[0]);
	bot->setPosition(1, position[1]);
	bot->setPosition(2, position[2] - 2.5f);
	bot->sync();

	RakBot::app()->getEvents()->defCallAdd(1500, false, [bot](DefCall *) {
		bot->takeCheckpoint();
		_botSuspended = false;
		takeCheckpointReady = true;
	});
}

void SampRpFuncs::pickUpPickup(Pickup *pickup) {
	static bool pickUpReady = true;

	if (pickup == nullptr)
		return;

	Bot *bot = RakBot::app()->getBot();

	if (bot->distanceTo(pickup) >= 120.0f) {
		RakBot::app()->log("[RAKBOT] ���������� �� ������ %d(%d) ������ 120 ������!", pickup->getPickupId(), pickup->getModel());
		return;
	}

	while (!pickUpReady)
		return;
	pickUpReady = false;

	_botSuspended = true;
	bot->setPosition(0, pickup->getPosition(0));
	bot->setPosition(1, pickup->getPosition(1));
	bot->setPosition(2, pickup->getPosition(2) - 2.5f);
	bot->sync();

	RakBot::app()->getEvents()->defCallAdd(1500, false, [bot, pickup](DefCall *) {
		bot->pickUpPickup(pickup);
		_botSuspended = false;
		pickUpReady = true;
	});
}

bool SampRpFuncs::isSampRpServer() {
	if (RakBot::app()->getServer()->getServerName().find("Samp-Rp.Ru") != std::string::npos)
		return true;

	return false;
}

bool SampRpFuncs::onServerMessage(std::string msg) {
	if (SampRpFuncs::isSampRpServer()) {
		Bot *bot = RakBot::app()->getBot();

		if (msg.find("����������� ���������� � �������� � ������ ���������") != std::string::npos) {
			bot->exitVehicle();
			vars.botAutoSchoolActive = false;
			vars.botAutoSchoolFinished = true;
		}

		if (vars.busWorkerRoute) {
			if (msg.find("�� ���������� ������������ ��������") != std::string::npos) {
				RakBot::app()->log("[RAKBOT] ������� ���������...");
				bot->sendInput("/en");
			}
		}

		if (msg.find("����� �� ����� ����� 500 ����") != std::string::npos)
			vars.botAutoSchoolActive = 0;

		std::smatch matches;
		if (std::regex_search(msg, matches, std::regex("������ ����������: (\\d+)"))) {
			BagCount = std::stoul(matches[1], nullptr, 10);
		}

		if (vars.botFarmerAutomated) {
			if (msg.find("������� ������� ������ �� ���� �����") != std::string::npos ||
				msg.find("�� ���� ������������ ������") != std::string::npos ||
				msg.find("� ������ ��� �����") != std::string::npos) {
				FarmGetPay = true;
				vect3_copy(FarmPos[FarmIndex], vars.coordMasterTarget);
				vars.coordMasterEnabled = true;
				FarmWork = false;
				FarmCount = 0;
				if (vars.bQuestEnabled)
					ChangeFarm = true;
			}
		}

		if (msg.find("������� ���� ����� � ��������� \"Victim\"") != std::string::npos)
			return true;

		if (vars.botFarmerAutomated) {
			if (msg.find("������� ���� �����") != std::string::npos) {
				vect3_copy(FarmFieldPos[FarmIndex], vars.coordMasterTarget);
				vars.coordMasterEnabled = true;
				FarmWork = 1;
				return true;
			}

			if (msg.find("������� ���� �������") != std::string::npos) {
				if (ChangeFarm) {
					FarmIndex++;
					if (FarmIndex > 4)
						FarmIndex = 0;
					vect3_copy(FarmPos[FarmIndex], vars.coordMasterTarget);
					vars.coordMasterEnabled = 1;
					RakBot::app()->log("[RAKBOT] �������� �� ����� %d", FarmIndex);
				} else
					bot->pickUpPickup(RakBot::app()->getPickup(900 + (FarmIndex * 2)));
				return true;
			}
		}

		if (vars.botLoaderEnabled) {
			if (msg.find("�� ������ �� ����������") != std::string::npos)
				return true;

			if (msg.find("�� �� �������� ������") != std::string::npos) {
				LoaderStep = 6;
				return true;
			}
		}

		if (msg.find("���������� ����������������") != std::string::npos)
			return true;

		if (msg.find("��� IP ����� ������������") != std::string::npos) {
			RakBot::app()->log("[RAKBOT] IP ����� ������������");
			bot->disconnect(false);
			RakBot::app()->exit();
			return true;
		}
	}
	return false;
}
