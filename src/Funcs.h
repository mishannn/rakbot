#pragma once

class Funcs {
public:
	Funcs();
	~Funcs();
};

enum BotLoaderStep {
	BOTLOADER_STEP_TAKEBAG = 100,
	BOTLOADER_STEP_WAITING,
	BOTLOADER_STEP_PUTBAG,
	BOTLOADER_STEP_GETPAY,
	BOTLOADER_STEP_STARTWORK
};

extern int LoaderStep;
extern int BagCount;
extern bool BotWithBag;

extern bool FarmWork;
extern float FarmPos[5][3];
extern float FarmFieldPos[5][3];
extern int FarmIndex;
extern int FarmCount;
extern bool ChangeFarm;
extern bool FarmGetPay;

void AntiAFK();
void KeepOnline();
void NoAfk();
void RoutePlay();
void CheckChangePos();
void UpdateInfo();
void AdminChecker();
void BotLoader();
void FuncsLoop();
void FuncsOff();