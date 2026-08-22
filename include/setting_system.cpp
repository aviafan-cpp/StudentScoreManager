#include "setting_system.hpp"
#include <string>
#include <cmath>
#include <fstream>

namespace SettingSystem
{
    bool Settings::IsSettingExist(std::string name)
    {
        for(int i = 0; i < settings.size(); ++i)
        {
            if(settings[i].setting_name == name)
            {
                return true;
            }
        }
        return false;
    }

    int Settings::GetSettingIndex(std::string name)
    {
        for(int i = 0; i < settings.size(); ++i)
        {
            if(settings[i].setting_name == name)
            {
                return i;
            }
        }
        return -1;
    }

    std::string Settings::GetSetting(std::string name)
    {
        if(!IsSettingExist(name)) return "";
        return settings[GetSettingIndex(name)].setting_content;
    }

    int Settings::GetSettingI(std::string name)
    {
        if(!IsSettingExist(name)) return -1;
        return std::stoi(settings[GetSettingIndex(name)].setting_content);
    }

    float Settings::GetSettingF(std::string name)
    {
        if(!IsSettingExist(name)) return NAN;
        return std::stof(settings[GetSettingIndex(name)].setting_content);
    }

    SettingLocation Settings::GetSettingSL(std::string name)
    {
        if(!IsSettingExist(name)) return Temp;
        return settings[GetSettingIndex(name)].setting_location;
    }

    int Settings::MatchSettingContent(std::string name, std::vector<std::string> contents)
    {
        if(!IsSettingExist(name)) return -1;
        for(int i = 0; i < contents.size(); ++i)
        {
            if(settings[GetSettingIndex(name)].setting_content == contents[i])
            {
                return i;
            }
        }
        return -1;
    }

    int Settings::MatchSettingContent(std::string name, std::vector<int> contents)
    {
        if(!IsSettingExist(name)) return -1;
        for(int i = 0; i < contents.size(); ++i)
        {
            if(settings[GetSettingIndex(name)].setting_content == std::to_string(contents[i]))
            {
                return i;
            }
        }
        return -1;
    }

    void Settings::SaveSettingToFile(std::string path)
    {
        std::ofstream file(path);
        if (!file.is_open())
        {
            return; // 文件打开失败，静默返回
        }
        
        for (const auto& setting : settings)
        {
            // 只保存 Local 类型的设置
            if (setting.setting_location == Local)
            {
                // 格式：setting_name=setting_content
                file << setting.setting_name << "=" << setting.setting_content << "\n";
            }
        }
        
        file.close();
    }

    void Settings::AddSetting(std::string name, std::string content, SettingLocation sl)
    {
        Setting new_setting = {name, content, sl};
        if(IsSettingExist(name))
        {
            settings[GetSettingIndex(name)] = new_setting;
        }
        else
        {
            settings.push_back(new_setting);
        }
    }

    void Settings::AddSetting(std::string name, int content, SettingLocation sl)
    {
        Setting new_setting = {name, std::to_string(content), sl};
        if(IsSettingExist(name))
        {
            settings[GetSettingIndex(name)] = new_setting;
        }
        else
        {
            settings.push_back(new_setting);
        }
    }

    void Settings::AddSetting(std::string name, float content, SettingLocation sl)
    {
        Setting new_setting = {name, std::to_string(content), sl};
        if(IsSettingExist(name))
        {
            settings[GetSettingIndex(name)] = new_setting;
        }
        else
        {
            settings.push_back(new_setting);
        }
    }

    void Settings::AddSettingFromFile(std::string path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return; // 文件打开失败，静默返回
        }
        
        std::string line;
        while (std::getline(file, line))
        {
            // 跳过空行
            if (line.empty()) continue;
            
            // 查找分隔符
            size_t eq_pos = line.find('=');
            if (eq_pos == std::string::npos)
            {
                continue; // 格式错误，跳过这一行
            }
            
            std::string name = line.substr(0, eq_pos);
            std::string content = line.substr(eq_pos + 1);
            
            // 从文件加载的设置默认作为 Local
            AddSetting(name, content, Local);
        }
        
        file.close();
    }
}