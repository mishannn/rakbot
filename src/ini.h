bool SetConfigValue(const std::string &filePath, const std::string &section, const std::string &key, const std::string &value);
const std::string &GetConfigValue(const std::string &filePath, const std::string &section, const std::string &key, const std::string &defaultValue);

void LoadRoute(char *routeName);
bool LoadConfig();
bool LoadCustom();
void SaveConfig(const std::string &filePath);