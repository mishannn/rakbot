#pragma once

#include "Includes.h"
#include "Defines.h"
#include "Structs.h"

extern struct TeleportPlace TeleportPlaces[300];
extern bool ConnectRequested, GameInited;
extern class Timer BotConnectedTimer, BotSpawnedTimer, GameInitedTimer, ReconnectTimer;