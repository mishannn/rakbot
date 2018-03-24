int FindNearestPickup(int model);
int FindNearestVehicle();
int FindNearestPlayer();
int FindNearestVehicleByModel(int model);

int GetPlayerID(char *name);

char *GenRandom(char *s, const int len);
char *GetRakBotPath();
char *GetRakBotPath(char *path);

std::string UrlEncode(const std::string &s);

void LTrim(std::string &s);
void RTrim(std::string &s);
void Trim(std::string &s);
std::vector<std::string> Split(const std::string &s, char delim);