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
		RakBot::app()->log("[RAKBOT] Расстояние до чекпоинта больше 120 метров!");
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
		RakBot::app()->log("[RAKBOT] Расстояние до пикапа %d(%d) больше 120 метров!", pickup->getPickupId(), pickup->getModel());
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

		if (msg.find("Припаркуйте автомобиль и пройдите в здание автошколы") != std::string::npos) {
			bot->exitVehicle();
			vars.botAutoSchoolActive = false;
			vars.botAutoSchoolFinished = true;
		}

		if (vars.busWorkerRoute) {
			if (msg.find("Вы арендовали транспортное средство") != std::string::npos) {
				RakBot::app()->log("[RAKBOT] Заводим двигатель...");
				bot->sendInput("/en");
			}
		}

		if (msg.find("Сдача на права стоит 500 вирт") != std::string::npos)
			vars.botAutoSchoolActive = 0;

		std::smatch matches;
		if (std::regex_search(msg, matches, std::regex("Мешков перетащено: (\\d+)"))) {
			BagCount = std::stoul(matches[1], nullptr, 10);
		}

		if (vars.botFarmerAutomated) {
			if (msg.find("Сначала начните работу на этой ферме") != std::string::npos ||
				msg.find("На поле недостаточно урожая") != std::string::npos ||
				msg.find("В машине нет места") != std::string::npos) {
				FarmGetPay = true;
				vect3_copy(FarmPos[FarmIndex], vars.coordMasterTarget);
				vars.coordMasterEnabled = true;
				FarmWork = false;
				FarmCount = 0;
				if (vars.bQuestEnabled)
					ChangeFarm = true;
			}
		}

		if (msg.find("Сменить скин можно в магазинах \"Victim\"") != std::string::npos)
			return true;

		if (vars.botFarmerAutomated) {
			if (msg.find("Рабочий день начат") != std::string::npos) {
				vect3_copy(FarmFieldPos[FarmIndex], vars.coordMasterTarget);
				vars.coordMasterEnabled = true;
				FarmWork = 1;
				return true;
			}

			if (msg.find("Рабочий день окончен") != std::string::npos) {
				if (ChangeFarm) {
					FarmIndex++;
					if (FarmIndex > 4)
						FarmIndex = 0;
					vect3_copy(FarmPos[FarmIndex], vars.coordMasterTarget);
					vars.coordMasterEnabled = 1;
					RakBot::app()->log("[RAKBOT] Телепорт на ферму %d", FarmIndex);
				} else
					bot->pickUpPickup(RakBot::app()->getPickup(900 + (FarmIndex * 2)));
				return true;
			}
		}

		if (vars.botLoaderEnabled) {
			if (msg.find("Вы ничего не заработали") != std::string::npos)
				return true;

			if (msg.find("Вы не начинали работу") != std::string::npos) {
				LoaderStep = 6;
				return true;
			}
		}

		if (msg.find("Необходимо авторизироваться") != std::string::npos)
			return true;

		if (msg.find("Ваш IP адрес заблокирован") != std::string::npos) {
			RakBot::app()->log("[RAKBOT] IP адрес заблокирован");
			bot->disconnect(false);
			RakBot::app()->exit();
			return true;
		}
	}
	return false;
}
