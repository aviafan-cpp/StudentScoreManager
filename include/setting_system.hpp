#ifndef SETTING_SYSTEM
#define SETTING_SYSTEM

#include <string>
#include <vector>

namespace SettingSystem
{
    enum SettingLocation {Temp, Local};

    struct Setting
    {
        std::string setting_name;
        std::string setting_content;
        SettingLocation setting_location;
    };
    
    struct Settings
    {
        private:
        std::vector<Setting> settings;

        public:
        bool IsSettingExist(std::string name);
        int GetSettingIndex(std::string name);
        std::string GetSetting(std::string name);
        int GetSettingI(std::string name);
        float GetSettingF(std::string name);
        SettingLocation GetSettingSL(std::string name);
        int MatchSettingContent(std::string name, std::vector<std::string> contents); // 将传入数组的值与设置项对比
        int MatchSettingContent(std::string name, std::vector<int> contents); // 将传入数组的值与设置项对比(int)
        
        void SaveSettingToFile(std::string path);

        void AddSetting(std::string name, std::string content, SettingLocation sl);
        void AddSetting(std::string name, int content, SettingLocation sl);
        void AddSetting(std::string name, float content, SettingLocation sl);
        void AddSettingFromFile(std::string path);
    };
}

#endif