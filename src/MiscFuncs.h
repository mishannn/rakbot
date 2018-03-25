class Pickup *FindNearestPickup(int model);
class Vehicle *FindNearestVehicle(int opened = -1, int model = -1, int color1 = -1, int color2 = -1);
class Player *FindNearestPlayer(int skin = -1);
class Player *FindPlayerByName(std::string name);
void DoCoordMaster(bool state, float x = 0.f, float y = 0.f, float z = 0.f);

char *GenRandom(char *s, const int len);
char *GetRakBotPath();
char *GetRakBotPath(char *path);

std::string UrlEncode(const std::string &s);

void LTrim(std::string &s);
void RTrim(std::string &s);
void Trim(std::string &s);
std::vector<std::string> Split(const std::string &s, char delim);

