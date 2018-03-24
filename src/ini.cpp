#include "RakBot.h"

#include "MiscFuncs.h"

#include "Settings.h"

void LoadCustom() {
	char szBuf[512], *szAppName, *szPath = GetRakBotPath("settings\\custom.ini");

	FILE *fdCustomSetings = fopen(szPath, "r");
	if (fdCustomSetings) {
		fclose(fdCustomSetings);

		szAppName = "Bot";
		GetPrivateProfileString(szAppName, "MainDelay", "50", szBuf, sizeof(szBuf), szPath);
		vars.mainDelay = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "UpdateDelay", "50", szBuf, sizeof(szBuf), szPath);
		vars.updateDelay = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "SpawnDelay", "500", szBuf, sizeof(szBuf), szPath);
		vars.spawnDelay = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "DialogResponseDelay", "500", szBuf, sizeof(szBuf), szPath);
		vars.dialogResponseDelay = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "NoAfkDelay", "100", szBuf, sizeof(szBuf), szPath);
		vars.dialogResponseDelay = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "AfterSpawnDelay", "1000", szBuf, sizeof(szBuf), szPath);
		vars.afterSpawnDelay = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "EnterVehicleDelay", "2000", szBuf, sizeof(szBuf), szPath);
		vars.enterVehicleDelay = std::strtoul(szBuf, nullptr, 10);

		szAppName = "AdminChecker";
		GetPrivateProfileString(szAppName, "URL", "http://api.sanek.love/al_list.php?responseType=colon&from=RakBot", szBuf, sizeof(szBuf), szPath);
		vars.adminsUrl = std::string(szBuf);

		szAppName = "Route";
		GetPrivateProfileString(szAppName, "Loop", "1", szBuf, sizeof(szBuf), szPath);
		vars.routeLoop = static_cast<bool>(std::strtoul(szBuf, nullptr, 10));

		szAppName = "TextDraw";
		GetPrivateProfileString(szAppName, "CreateLog", "1", szBuf, sizeof(szBuf), szPath);
		vars.textDrawCreateLogging = static_cast<bool>(std::strtoul(szBuf, nullptr, 10));

		GetPrivateProfileString(szAppName, "HideLog", "0", szBuf, sizeof(szBuf), szPath);
		vars.textDrawHideLogging = static_cast<bool>(std::strtoul(szBuf, nullptr, 10));

		GetPrivateProfileString(szAppName, "SetStringLog", "0", szBuf, sizeof(szBuf), szPath);
		vars.textDrawSetStringLogging = static_cast<bool>(std::strtoul(szBuf, nullptr, 10));

		szAppName = "NetworkAdapter";
		GetPrivateProfileString(szAppName, "Address", "0.0.0.0", szBuf, sizeof(szBuf), szPath);
		vars.adapterAddress = std::string(szBuf);
	} else {
		szAppName = "Bot";
		WritePrivateProfileString(szAppName, "MainDelay", "50", szPath);
		WritePrivateProfileString(szAppName, "UpdateDelay", "50", szPath);
		WritePrivateProfileString(szAppName, "SpawnDelay", "500", szPath);
		WritePrivateProfileString(szAppName, "DialogResponseDelay", "500", szPath);
		WritePrivateProfileString(szAppName, "NoAfkDelay", "100", szPath);
		WritePrivateProfileString(szAppName, "AfterSpawnDelay", "1000", szPath);
		WritePrivateProfileString(szAppName, "EnterVehicleDelay", "2000", szPath);

		szAppName = "AdminChecker";
		WritePrivateProfileString(szAppName, "URL", "http://api.sanek.love/al_list.php?responseType=colon&from=RakBot", szPath);

		szAppName = "Route";
		WritePrivateProfileString(szAppName, "Loop", "1", szPath);

		szAppName = "TextDraw";
		WritePrivateProfileString(szAppName, "CreateLog", "1", szPath);
		WritePrivateProfileString(szAppName, "HideLog", "0", szPath);
		WritePrivateProfileString(szAppName, "SetStringLog", "0", szPath);

		szAppName = "NetworkAdapter";
		WritePrivateProfileString(szAppName, "Address", "0.0.0.0", szPath);

		MessageBox(NULL, "Файл \"settings\\custom.ini\" создан! Перезапустите бота.", "Первая настройка", MB_ICONASTERISK);
		vars.botOff = true;
	}
}

void LoadRoute(char *routeName) {
	vars.routeData.clear();

	OnfootData onfootData;

	char buf[512];
	snprintf(buf, sizeof(buf), "routes\\%s.route", routeName);
	std::string routePath = GetRakBotPath(buf);
	std::fstream routeFile(routePath, std::ios::in | std::ios::binary);

	if (routeFile.is_open()) {
		while (!routeFile.eof()) {
			routeFile.read((char *)&onfootData, sizeof(OnfootData));
			vars.routeData.push_back(onfootData);
		}

		routeFile.close();

		vars.routeSpeed = 25.f;
		RakBot::app()->log("[RAKBOT] Маршрут '%s' успешно загружен", routeName);
	} else {
		RakBot::app()->log("[RAKBOT] Маршрут '%s' не найден", routeName);
	}
}

void LoadConfig() {
	ZeroMemory(&vars, sizeof(Vars));
	CreateDirectory(GetRakBotPath("settings"), NULL);

	char szBuf[64], *szAppName, *szPath = GetRakBotPath("settings\\settings.ini");
	FILE *fdSetings = fopen(szPath, "r");
	if (fdSetings) {
		fclose(fdSetings);

		szAppName = "Account";
		GetPrivateProfileString(szAppName, "Server", "127.0.0.1:7777", szBuf, sizeof(szBuf), szPath);

		char *pChar = szBuf;
		while (*pChar) {
			if (*pChar == ':') {
				*pChar = 0;
				RakBot::app()->getSettings()->getAddress()->setPort(static_cast<uint16_t>(std::strtoul(pChar + 1, nullptr, 10)));
			}
			pChar++;
		}
		RakBot::app()->getSettings()->getAddress()->setIp(std::string(szBuf));

		GetPrivateProfileString(szAppName, "NickName", "MishaN", szBuf, sizeof(szBuf), szPath);
		RakBot::app()->getSettings()->setName(std::string(szBuf));

		GetPrivateProfileString(szAppName, "Password", "qwerty", szBuf, sizeof(szBuf), szPath);
		RakBot::app()->getSettings()->setLoginPassword(std::string(szBuf));

		szAppName = "AutoReg";
		GetPrivateProfileString(szAppName, "Enabled", "0", szBuf, sizeof(szBuf), szPath);
		vars.autoRegEnabled = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "Mail", "mail@mail.ru", szBuf, sizeof(szBuf), szPath);
		vars.autoRegMail = std::string(szBuf);

		GetPrivateProfileString(szAppName, "NickName", "CMishaN", szBuf, sizeof(szBuf), szPath);
		vars.autoRegReferer = std::string(szBuf);

		GetPrivateProfileString(szAppName, "Sex", "1", szBuf, sizeof(szBuf), szPath);
		vars.autoRegSex = std::strtoul(szBuf, nullptr, 10);

		szAppName = "CoordMaster";
		GetPrivateProfileString(szAppName, "Distance", "80.0", szBuf, sizeof(szBuf), szPath);
		vars.coordMasterDist = std::strtof(szBuf, nullptr);

		GetPrivateProfileString(szAppName, "Height", "-40.0", szBuf, sizeof(szBuf), szPath);
		vars.coordMasterHeight = std::strtof(szBuf, nullptr);

		GetPrivateProfileString(szAppName, "Delay", "1500", szBuf, sizeof(szBuf), szPath);
		vars.coordMasterDelay = std::strtoul(szBuf, nullptr, 10);

		szAppName = "Loader";
		GetPrivateProfileString(szAppName, "Delay", "10000", szBuf, sizeof(szBuf), szPath);
		vars.botLoaderDelay = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "Limit", "20", szBuf, sizeof(szBuf), szPath);
		vars.botLoaderCount = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "Enabled", "0", szBuf, sizeof(szBuf), szPath);
		vars.botLoaderEnabled = static_cast<bool>(std::strtoul(szBuf, nullptr, 10));

		szAppName = "Settings";
		GetPrivateProfileString(szAppName, "KickReconnect", "15", szBuf, sizeof(szBuf), szPath);
		vars.reconnectDelay = std::strtoul(szBuf, nullptr, 10) * 1000;

		GetPrivateProfileString(szAppName, "AdminReconnect", "300", szBuf, sizeof(szBuf), szPath);
		vars.adminReconnectDelay = std::strtoul(szBuf, nullptr, 10) * 1000;

		GetPrivateProfileString(szAppName, "PasswordDialog", "1", szBuf, sizeof(szBuf), szPath);
		vars.dialogIdPassword = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "BanDialog", "58", szBuf, sizeof(szBuf), szPath);
		vars.dialogIdBan = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "AdminOnline", "0", szBuf, sizeof(szBuf), szPath);
		vars.adminActionOnline = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "AdminNear", "0", szBuf, sizeof(szBuf), szPath);
		vars.adminActionNear = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "BusRoute", "0", szBuf, sizeof(szBuf), szPath);
		vars.busWorkerRoute = std::strtoul(szBuf, nullptr, 10);

		GetPrivateProfileString(szAppName, "AntiAfkDelay", "1", szBuf, sizeof(szBuf), szPath);
		vars.antiAfkDelay = std::strtoul(szBuf, nullptr, 10) * 1000;

		GetPrivateProfileString(szAppName, "AntiAfkOffset", "0.001", szBuf, sizeof(szBuf), szPath);
		vars.antiAfkOffset = std::strtof(szBuf, nullptr);
	} else {
		szAppName = "Account";
		WritePrivateProfileString(szAppName, "Server", "127.0.0.1:7777", szPath);
		WritePrivateProfileString(szAppName, "NickName", "MishaN", szPath);
		WritePrivateProfileString(szAppName, "Password", "qwerty", szPath);

		szAppName = "AutoReg";
		WritePrivateProfileString(szAppName, "Enabled", "0", szPath);
		WritePrivateProfileString(szAppName, "Mail", "mail@mail.ru", szPath);
		WritePrivateProfileString(szAppName, "NickName", "CMishaN", szPath);
		WritePrivateProfileString(szAppName, "Sex", "1", szPath);

		szAppName = "CoordMaster";
		WritePrivateProfileString(szAppName, "Distance", "80.0", szPath);
		WritePrivateProfileString(szAppName, "Height", "-40.0", szPath);
		WritePrivateProfileString(szAppName, "Delay", "1500", szPath);

		szAppName = "Loader";
		WritePrivateProfileString(szAppName, "Delay", "10000", szPath);
		WritePrivateProfileString(szAppName, "Limit", "20", szPath);
		WritePrivateProfileString(szAppName, "Enabled", "0", szPath);

		szAppName = "Settings";
		WritePrivateProfileString(szAppName, "KickReconnect", "15", szPath);
		WritePrivateProfileString(szAppName, "AdminReconnect", "300", szPath);
		WritePrivateProfileString(szAppName, "PasswordDialog", "1", szPath);
		WritePrivateProfileString(szAppName, "BanDialog", "58", szPath);
		WritePrivateProfileString(szAppName, "AdminOnline", "0", szPath);
		WritePrivateProfileString(szAppName, "AdminNear", "0", szPath);
		WritePrivateProfileString(szAppName, "BusRoute", "0", szPath);
		WritePrivateProfileString(szAppName, "AntiAfkDelay", "1", szPath);
		WritePrivateProfileString(szAppName, "AntiAfkOffset", "0.001", szPath);

		LoadCustom();

		MessageBox(NULL, "Файл настроек был создан, так как отсутствовал!\nПерезапустите приложение", "Предупреждение", MB_ICONASTERISK);
		RakBot::app()->exit();
	}

	if (vars.busWorkerRoute > 0 && vars.busWorkerRoute < 9)
		vars.checkPointMaster = 1;

	vars.noAfk = true;
	vars.timeStamp = true;
	vars.routeSpeed = 0;
	vars.logFile = nullptr;

	LoadCustom();
}