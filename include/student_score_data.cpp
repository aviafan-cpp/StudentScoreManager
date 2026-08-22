#include "student_score_data.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <ctime>
#include <filesystem>
#include <algorithm>
#include <iomanip>

// 仅遍历当前目录（不递归子目录）
std::vector<std::string> traverse_non_recursive(const std::filesystem::path& root, const std::string& extension) {
    std::vector<std::string> result;
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (std::filesystem::is_regular_file(entry.status()) && entry.path().extension() == extension) {
            result.emplace_back(entry.path().filename().string());
        }
    }
    return result;
}

namespace student_score_data
{
    // sgroup
    void sgroup::load(std::string path)
    {
        std::ifstream file(path);
        std::string line;
        while(std::getline(file, line)) student_names.emplace_back(line);
    }

    void sgroup::save(std::string path) const
    {
        std::ofstream file(path);
        for(auto& line : student_names) file << line << '\n';
    }

    // event
    void event::load_from_line(const std::string& line)
    {
        std::string word; // word为临时单词存储
        std::istringstream iss(line);
        std::vector<std::string> parts;
        while(iss >> word) parts.push_back(word);

        if(parts.size() < 5) return;
        try
        {
            this->name = parts[0];
            this->score = std::stoi(parts[1]);
            this->time = [&]() {
                std::tm tm = {};
                std::istringstream ss(parts[2]);
                ss >> std::get_time(&tm, "%Y-%m-%d_%H:%M:%S");
                return std::chrono::system_clock::from_time_t(std::mktime(&tm));
            }();
            this->pt = static_cast<penalty_type>(std::stoi(parts[3]));
            this->column = std::stoi(parts[4]);
            if(parts.size() > 5)
            {
                for(size_t i = 5; i < parts.size(); ++i) {
                    if(i > 5) this->remark += " ";  // 添加空格分隔
                    this->remark += parts[i];
                }
            }
        }
        catch(const std::exception& e)
        {
            return;
        }
    }

    std::string event::save_to_line() const
    {
        std::ostringstream oss;
        std::time_t tt = std::chrono::system_clock::to_time_t(time);
        
        // 使用 put_time 输出格式化时间
        oss << name << " "
            << score << " "
            << std::put_time(std::localtime(&tt), "%Y-%m-%d_%H:%M:%S") << " "
            << static_cast<int>(pt) << " "
            << column << " "
            << remark;
        
        return oss.str();
    }

    // event_table
    void event_table::load(std::string path)
    {
        std::ifstream file(path);
        std::string line;
        while(std::getline(file, line))
        {
            event e;
            e.load_from_line(line);

            // 时间冲突就往后推1秒，直到不冲突为止
            while(true) {
                bool conflict = false;
                for(const auto& existing : events) {
                    if(existing.time == e.time) {
                        conflict = true;
                        break;
                    }
                }
                if(!conflict) break;
                e.time += std::chrono::seconds(1);
            }

            events.emplace_back(e);
        }
    }

    void event_table::save(std::string path) const
    {
        std::ofstream file(path);
        for(auto& e : events) file << e.save_to_line() << '\n';
    }

    int event_table::score_total() const
    {
        int result = 0;
        for(const auto& et : this->events)
        {
            if(et.pt != META) result += et.score;
        }
        return result;
    }

    event_table event_table::no_meta_data() const
    {
        event_table result;
        for(const auto& et : this->events)
        {
            if(et.pt != META) result.events.emplace_back(et);
        }
        return result;
    }

    std::vector<std::string> event_table::get_all_name() const
    {
        std::vector<std::string> result;
        for(const auto& et : this->events)
        {
            if(std::find(result.begin(), result.end(), et.name) == result.end())
            {
                result.emplace_back(et.name);
            } 
        }
        return result;
    }

    event_table event_table::filter_name(std::string name) const
    {
        event_table result;
        for(const auto& et : this->events)
        {
            if(et.name == name) result.events.emplace_back(et);
        }
        return result;
    }

    std::vector<std::chrono::time_point<std::chrono::system_clock>> event_table::get_all_time() const
    {
        std::vector<std::chrono::time_point<std::chrono::system_clock>> result;
        for(const auto& et : this->events)
        {
            if(std::find(result.begin(), result.end(), et.time) == result.end())
            {
                result.emplace_back(et.time);
            }
        }
        return result;
    }

    event_table event_table::filter_time(std::chrono::time_point<std::chrono::system_clock> time) const
    {
        event_table result;
        for(const auto& et : this->events)
        {
            if(et.time == time) result.events.emplace_back(et);
        }
        return result;
    }

    void event_table::del_time_all(std::chrono::time_point<std::chrono::system_clock> time)
    {
        for(auto it = this->events.begin(); it != this->events.end(); )
        {
            if(it->time == time)
            {
                it = this->events.erase(it);  // erase 返回下一个有效迭代器
            }
            else
            {
                ++it;
            }
        }
    }

    std::vector<std::chrono::year_month_day> event_table::get_all_time_day() const
    {
        std::vector<std::chrono::year_month_day> result;
        for(const auto& et : this->events)
        {
            std::chrono::year_month_day ymd{floor<std::chrono::days>(et.time)};
            if(std::find(result.begin(), result.end(), ymd) == result.end())
            {
                result.emplace_back(ymd);
            }
        }
        return result;
    }

    event_table event_table::filter_time_day(std::chrono::year_month_day time) const
    {
        event_table result;
        for(const auto& et : this->events)
        {
            std::chrono::year_month_day ymd{floor<std::chrono::days>(et.time)};
            if(ymd == time) result.events.emplace_back(et);
        }
        return result;
    }

    std::vector<int> event_table::get_all_column() const
    {
        std::vector<int> result;
        for(const auto& et : this->events)
        {
            if(std::find(result.begin(), result.end(), et.column) == result.end())
            {
                result.emplace_back(et.column);
            }
        }
        return result;
    }

    event_table event_table::filter_column(int column) const
    {
        event_table result;
        for(const auto& et : this->events)
        {
            if(et.column == column) result.events.emplace_back(et);
        }
        return result;
    }

    // sclass
    void sclass::load(std::vector<std::string> student_names_paths, std::vector<std::string> event_paths)
    {
        groups.clear();
        event_tables.clear();
        groups.resize(student_names_paths.size());
        event_tables.resize(event_paths.size());

        for(size_t i = 0; i < student_names_paths.size(); ++i) groups[i].load(student_names_paths[i]);
        for(size_t i = 0; i < event_paths.size(); ++i) event_tables[i].load(event_paths[i]);
    }

    void sclass::load_dir(std::string dir)
    {
        if(dir.back() != '/') dir += '/';

        // ---- 加载 groups ----
        groups.clear();
        auto ssdn = traverse_non_recursive(dir, ".ssdata_names");
        // 按数字排序
        std::sort(ssdn.begin(), ssdn.end(), [](const std::string& a, const std::string& b) {
            int num_a = std::stoi(a.substr(0, a.find('.')));
            int num_b = std::stoi(b.substr(0, b.find('.')));
            return num_a < num_b;
        });
        
        // 加载所有文件并重命名
        for(size_t i = 0; i < ssdn.size(); ++i) {
            std::string old_path = dir + ssdn[i];
            groups.emplace_back();
            groups.back().load(old_path);
            
            // 如果不是连续编号，则自动重命名
            int current_num = std::stoi(ssdn[i].substr(0, ssdn[i].find('.')));
            if(current_num != static_cast<int>(i)) {
                std::string new_path = dir + std::to_string(i) + ".ssdata_names";
                // 先保存新文件
                groups.back().save(new_path);
                // 删除旧文件
                std::filesystem::remove(old_path);
            }
        }

        // ---- 加载 event_tables ----
        event_tables.clear();
        auto ssdet = traverse_non_recursive(dir, ".ssdata_event_table");
        std::sort(ssdet.begin(), ssdet.end(), [](const std::string& a, const std::string& b) {
            int num_a = std::stoi(a.substr(0, a.find('.')));
            int num_b = std::stoi(b.substr(0, b.find('.')));
            return num_a < num_b;
        });
        
        // 加载所有文件并重命名
        for(size_t i = 0; i < ssdet.size(); ++i) {
            std::string old_path = dir + ssdet[i];
            event_tables.emplace_back();
            event_tables.back().load(old_path);
            
            // 如果不是连续编号，则自动重命名
            int current_num = std::stoi(ssdet[i].substr(0, ssdet[i].find('.')));
            if(current_num != static_cast<int>(i)) {
                std::string new_path = dir + std::to_string(i) + ".ssdata_event_table";
                // 先保存新文件
                event_tables.back().save(new_path);
                // 删除旧文件
                std::filesystem::remove(old_path);
            }
        }
    }

    void sclass::save(std::vector<std::string> student_names_paths, std::vector<std::string> event_paths) const
    {
        if(student_names_paths.size() != groups.size() || event_paths.size() != event_tables.size()) throw "size don't same";
        for(size_t i = 0; i < student_names_paths.size(); ++i) groups[i].save(student_names_paths[i]);
        for(size_t i = 0; i < event_paths.size(); ++i) event_tables[i].save(event_paths[i]);
    }

    void sclass::save_dir(std::string dir) const
    {
        if(dir.back() != '/') dir += '/';
        for(size_t i = 0; i < groups.size(); ++i) groups[i].save(dir + std::to_string(i) + ".ssdata_names");
        for(size_t i = 0; i < event_tables.size(); ++i) event_tables[i].save(dir + std::to_string(i) + ".ssdata_event_table");
    }

    std::vector<std::string> sclass::get_all_name() const
    {
        std::vector<std::string> result;
        for(const auto& g : this->groups)
        {
            for(const auto& s : g.student_names)
            {
                result.emplace_back(s);
            }
        }
        return result;
    }

    int sclass::find_name_group(std::string name) const
    {
        for(size_t i = 0; i < this->groups.size(); ++i)
        {
            for(const auto& s : this->groups[i].student_names)
            {
                if(s == name) return i;
            }
        }
        return -1;
    }
}