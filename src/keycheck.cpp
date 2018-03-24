#include "RakBot.h"

#include "MiscFuncs.h"
#include "Settings.h"

#include "licwnd.h"

char Bios[256], Processor[256];

HRESULT InitializeCom() {
	HRESULT result = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
	if (FAILED(result))
		return result;

	result = CoInitializeSecurity(
		NULL,
		-1,                             // cAuthSvc (COM authentication)
		NULL,                           // asAuthSvc
		NULL,                           // pReserved1
		RPC_C_AUTHN_LEVEL_DEFAULT,      // dwAuthnLevel
		RPC_C_IMP_LEVEL_IMPERSONATE,    // dwImpLevel
		NULL,                           // pAuthList
		EOAC_NONE,                      // dwCapabilities
		NULL                            // Reserved
	);

	if (FAILED(result) && result != RPC_E_TOO_LATE) {
		CoUninitialize();

		return result;
	}

	return NOERROR;
}

HRESULT GetWbemService(IWbemLocator** pLocator, IWbemServices** pService) {
	HRESULT result = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, reinterpret_cast<LPVOID*>(pLocator));

	if (FAILED(result)) {
		return result;
	}

	result = (*pLocator)->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),    // strNetworkResource
		NULL,                       // strUser  
		NULL,                       // strPassword
		NULL,                       // strLocale
		0,                          // lSecurityFlags
		NULL,                       // strAuthority
		NULL,                       // pCtx
		pService                    // ppNamespace
	);

	if (FAILED(result)) {
		(*pLocator)->Release();

		return result;
	}

	result = CoSetProxyBlanket(
		*pService,                      // pProxy
		RPC_C_AUTHN_WINNT,              // dwAuthnSvc
		RPC_C_AUTHZ_NONE,               // dwAuthzSvc
		NULL,                           // pServerPrincName
		RPC_C_AUTHN_LEVEL_CALL,         // dwAuthnLevel
		RPC_C_IMP_LEVEL_IMPERSONATE,    // dwImpLevel
		NULL,                           // pAuthInfo
		EOAC_NONE                       // dwCapabilities
	);

	if (FAILED(result)) {
		(*pService)->Release();
		(*pLocator)->Release();

		return result;
	}

	return NOERROR;
}

HRESULT QueryValue(IWbemServices* pService, const wchar_t* query, const wchar_t* propertyName, char* propertyValue, int maximumPropertyValueLength) {
	USES_CONVERSION;

	IEnumWbemClassObject* pEnumerator = NULL;
	HRESULT result = pService->ExecQuery(
		bstr_t(L"WQL"),                                         // strQueryLanguage
		bstr_t(query),                                          // strQuery
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,  // lFlags
		NULL,                                                   // pCtx
		&pEnumerator                                            // ppEnum
	);

	if (FAILED(result))
		return result;

	IWbemClassObject *pQueryObject = NULL;
	while (pEnumerator) {
		try {
			ULONG returnedObjectCount = 0;
			result = pEnumerator->Next(WBEM_INFINITE, 1, &pQueryObject, &returnedObjectCount);

			if (returnedObjectCount == 0)
				break;

			VARIANT objectProperty;
			result = pQueryObject->Get(propertyName, 0, &objectProperty, 0, 0);
			if (FAILED(result)) {
				if (pEnumerator != NULL)
					pEnumerator->Release();

				if (pQueryObject != NULL)
					pQueryObject->Release();

				return result;
			}

			if ((objectProperty.vt & VT_BSTR) == VT_BSTR) {
				strcpy_s(propertyValue, maximumPropertyValueLength, OLE2A(objectProperty.bstrVal));
				break;
			}

			VariantClear(&objectProperty);
		} catch (...) {
			if (pEnumerator != NULL)
				pEnumerator->Release();

			if (pQueryObject != NULL)
				pQueryObject->Release();

			return NOERROR;
		}
	}

	if (pEnumerator != NULL)
		pEnumerator->Release();

	if (pQueryObject != NULL)
		pQueryObject->Release();

	return NOERROR;
}

HRESULT GetCMB() {
	HRESULT result = InitializeCom();
	if (FAILED(result))
		return result;

	IWbemLocator* pLocator = NULL;
	IWbemServices* pService = NULL;
	result = GetWbemService(&pLocator, &pService);
	if (FAILED(result)) {
		CoUninitialize();
		return result;
	}

	result = QueryValue(pService, L"Select Name from Win32_Processor", L"Name", Processor, sizeof(Processor));
	if (FAILED(result)) {
		pService->Release();
		pLocator->Release();
		CoUninitialize();
		return result;
	}

	result = QueryValue(pService, L"Select ReleaseDate from Win32_BIOS", L"ReleaseDate", Bios, sizeof(Bios));
	if (FAILED(result)) {
		pService->Release();
		pLocator->Release();
		CoUninitialize();
		return result;
	}

	pService->Release();
	pLocator->Release();
	CoUninitialize();

	return NOERROR;
}

uint32_t GetDeviceID() {
	GetCMB();

	uint8_t checkSum[4];
	ZeroMemory(checkSum, 4);

	for (int i = 0; i < (int)strlen(Processor) - 4; i++) {
		checkSum[0] ^= Processor[i];
		checkSum[1] ^= Processor[i + 1];
		checkSum[2] ^= Processor[i + 2];
		checkSum[3] ^= Processor[i + 3];
	}

	for (int i = 0; i < (int)strlen(Bios) - 4; i++) {
		checkSum[0] ^= Bios[i];
		checkSum[1] ^= Bios[i + 1];
		checkSum[2] ^= Bios[i + 2];
		checkSum[3] ^= Bios[i + 3];
	}

	checkSum[0] ^= 0x1D;

	return *(uint32_t *)checkSum;
}

bool IsWrongKey() {
	if (vars.regKey.length() != 29 ||
		vars.regKey[5] != '-' ||
		vars.regKey[11] != '-' ||
		vars.regKey[17] != '-' ||
		vars.regKey[23] != '-')
		return 1;
	else
		return 0;
}

#define CURL_BUFFER_SIZE (512 * 1024)

char CurlBuffer[CURL_BUFFER_SIZE];
char CurlBufferUtf8[CURL_BUFFER_SIZE];
int CurlWriteOffset = 0;

size_t CurlWrite(char *data, size_t size, size_t count, char *buf) {
	size_t length = size * count;
	if (buf && ((CurlWriteOffset + length) < CURL_BUFFER_SIZE)) {
		memcpy(&buf[CurlWriteOffset], data, length);
		CurlWriteOffset += length;
	}

	return length;
}

CURLcode OpenURL(const std::string &url) {
	CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, CurlBufferUtf8);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, false);

	ZeroMemory(CurlBufferUtf8, sizeof(CurlBufferUtf8));
	CURLcode result = curl_easy_perform(curl);

	CurlWriteOffset = 0;
	curl_easy_cleanup(curl);
	curl_free(curl);

	ZeroMemory(CurlBuffer, sizeof(CurlBuffer));

	size_t inBytesLeft = sizeof(CurlBufferUtf8) - 1;
	size_t outBytesLeft = sizeof(CurlBuffer) - 1;
	char *inBuffer = CurlBufferUtf8;
	char *outBuffer = CurlBuffer;

	iconv_t cd = iconv_open("windows-1251", "utf-8");
	iconv(cd, const_cast<const char **>(&inBuffer), &inBytesLeft, &outBuffer, &outBytesLeft);
	iconv_close(cd);

	return result;
}

void HandleCheckKey() {
	char *buf = NULL;

	buf = strtok(CurlBuffer, ":");
	if (buf == NULL)
		return;

	int statusCode = atoi(buf);

	buf = strtok(NULL, ":");
	if (buf == NULL)
		return;

	int messageCode = atoi(buf);

	buf = strtok(NULL, ":");
	if (buf == NULL)
		return;

	char *message = buf;

	switch (statusCode) {
		case 1:
		{
			switch (messageCode) {
				case 1:
					vars.keyAccepted = true;
					break;

				case 2:
					vars.keyAccepted = true;
					MessageBox(NULL, message, "����������", MB_ICONASTERISK);
					break;

				default:
					break;
			}
			break;
		}

		case 2:
		{
			char error[512];
			snprintf(error, sizeof(error), "������ #%d ��� �������� �����!\n%s", messageCode, message);
			MessageBox(NULL, error, "������", MB_ICONERROR);
			break;
		}

		case 4:
		{
			switch (messageCode) {
				case 1:
					MessageBox(NULL, message, "����������", MB_ICONASTERISK);
					break;

				case 2:
					MessageBox(NULL, message, "��������������", MB_ICONWARNING);
					break;

				case 3:
					MessageBox(NULL, message, "������", MB_ICONERROR);
					break;

				default:
					break;
			}
			break;
		}

		default:
			break;
	}
}

void CheckKey() {
	char *keyPath = GetRakBotPath("settings\\license.key");
	std::fstream keyFile(keyPath, std::ios::in | std::ios::binary);

	if (keyFile.is_open()) {
		char regKey[30];
		keyFile.read(regKey, sizeof(regKey));
		regKey[29] = 0;
		vars.regKey = std::string(regKey);
		keyFile.close();
	}

	if (IsWrongKey()) {
		showLicenseWindow();
		return;
	}

	char pcName[256];
	DWORD pcNameLen = sizeof(pcName);
	GetComputerName(pcName, &pcNameLen);

	char buffer[512];
	sprintf(buffer, "http://rakbot.ru/keys/action/check?key=%s&hwid=%u&ver=%s&pc_name=%s",
		UrlEncode(vars.regKey).c_str(), GetDeviceID(), UrlEncode(std::string(RAKBOT_VERSION)).c_str(), pcName);
	CURLcode curlCode = OpenURL(buffer);

	if (curlCode == CURLE_OK) {
		HandleCheckKey();
	} else {
		char message[4096];
		snprintf(message, sizeof(message),
			"��� �������� ����� �������� ������ #%d ��� ���������� � ������ RakBot\n"
			"��������� ���������� � ����������, � ����� ������ ��������, ����������� �����������\n"
			"\n"
			"���� �� ��������, ��� ���� ��������� ������� �� ����� ����� ����, �� ���������� � ��������� (�� ������ ��� � ������)\n",
			curlCode);

		MessageBox(NULL, message, "������", MB_ICONERROR);
	}
}