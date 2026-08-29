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
#include <stdexcept>
#include <unordered_set>

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

    event_table event_table::filter_pt(penalty_type pt) const
    {
        event_table result;
        for(const auto& et : this->events)
        {
            if(pt == et.pt) result.events.emplace_back(et);
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
        if(student_names_paths.size() != groups.size() || event_paths.size() != event_tables.size()) throw std::runtime_error("size don't same");;
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

    bool sclass::is_name_same(std::string name) const
    {
        for(const auto& g : this->groups)
        {
            for(const auto& s : g.student_names)
            {
                if(s == name) return true;
            }
        }
        return false;
    }

    void sclass::set_name_no_same()
    {
        std::unordered_set<std::string> unique_names;
        for (auto& group : groups) {
            for (auto& name : group.student_names) {
                // 如果姓名已存在，则不断追加空格直到唯一
                while (unique_names.find(name) != unique_names.end()) {
                    name += " ";
                }
                unique_names.insert(name);
            }
        }
    }

    // 全局函数
    void event_table_to_csv(const std::vector<sgroup>& groups, const event_table& et, size_t column, std::string path, bool with_bom)
    {
        struct row
        {
            size_t group;
            std::string name;
            std::vector<int> scores;
            int score_total = 0;
        };
        std::vector<row> rows;

        for(size_t i = 0; i < groups.size(); ++i)
        {
            for(const auto& s : groups[i].student_names)
            {
                row new_row;
                new_row.group = i;
                new_row.name = s;
                for(size_t j = 0; j < column; ++j)
                {
                    new_row.scores.emplace_back(et.filter_name(s).filter_column(j).score_total());
                    new_row.score_total += new_row.scores.back();
                }

                rows.emplace_back(new_row);
            }
        }

        std::ofstream file(path, std::ios::binary);
        if(file.fail()) throw std::runtime_error("can't open " + path);

        if(with_bom)
        {
            unsigned char bom[] = {0xEF, 0xBB, 0xBF};
            file.write(reinterpret_cast<char*>(bom), 3);
        }

        // 表头
        file << "学生组别,学生姓名,";
        for(size_t i = 0; i < column; ++i) file << std::to_string(i) << ",";
        file << "总分\n";
        // 内容
        for(const auto& r : rows)
        {
            // 组别
            file << std::to_string(r.group) << ",";
            // 姓名
            std::string name_result;
            for(char c : r.name)
            {
                if(c == '"') name_result += '"';
                name_result += c;
            }
            file << '"' << name_result << "\",";
            // 分数
            for(const auto& s : r.scores) file << std::to_string(s) << ",";
            // 总分
            file << std::to_string(r.score_total) << "\n";
        }
    }

    void sclass_to_csv(const sclass& sc, std::string path, bool with_bom)
    {
        std::ofstream file(path, std::ios::binary);
        if(file.fail()) throw std::runtime_error("can't open " + path);

        if(with_bom)
        {
            unsigned char bom[] = {0xEF, 0xBB, 0xBF};
            file.write(reinterpret_cast<char*>(bom), 3);
        }

        // 表头
        file << "学生组别,学生姓名,";
        for(size_t i = 0; i < sc.event_tables.size(); ++i) file << std::to_string(i) << "表,";
        file << "总分\n";
        // 内容
        for(size_t group = 0; group < sc.groups.size(); ++group)
        {
            for(size_t student = 0; student < sc.groups[group].student_names.size(); ++student)
            {
                std::string name_result; // 处理转义符之后的结果
                for(char c : sc.groups[group].student_names[student])
                {
                    if(c == '"') name_result += '"';
                    name_result += c;
                }
                file << std::to_string(group) << ",\""  << name_result << "\",";

                int result = 0;
                for(size_t et = 0; et < sc.event_tables.size(); ++et)
                {
                    int score_total = sc.event_tables[et].filter_name(sc.groups[group].student_names[student]).score_total();
                    file << std::to_string(score_total) << ",";
                    result += score_total;
                }
                file << std::to_string(result) << "\n";
            }
        }
    }
}
