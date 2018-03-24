#include "RakBot.h"

#include "MiscFuncs.h"
#include "Settings.h"

#include "keycheck.h"

char authOut[64];

char *genAuthKey(const char *authIn) {
	char url[256];
	snprintf(url, sizeof(url), "http://rakbot.ru/keys/action/auth-key?auth_in=%s&key=%s&hwid=%u&ver=%s",
		authIn, UrlEncode(vars.regKey).c_str(), GetDeviceID(), UrlEncode(std::string(RAKBOT_VERSION)).c_str());
	
	CURLcode curlCode = OpenURL(url);

	if (curlCode == CURLE_OK) {
		strcpy(authOut, CurlBuffer);
		authOut[40] = 0;

		for (int i = 38; i > 1; i -= 2) {
			uint16_t t = *(uint16_t *)&authOut[i];
			*(uint16_t *)&authOut[i] = *(uint16_t *)&authOut[i - 2];
			*(uint16_t *)&authOut[i - 2] = t;
		}

		return authOut;
	} else {
		RakBot::app()->log("[RAKBOT] Ошибка #%d при получении AUTH_KEY");
		return nullptr;
	}
}