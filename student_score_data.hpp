#ifndef STUDENT_SCORE_DATA
#define STUDENT_SCORE_DATA

#include <vector>
#include <string>
#include <chrono>

namespace student_score_data
{
    struct sgroup
    {
        std::vector<std::string> student_names;

        void load(std::string path);
        void save(std::string path) const;
    };
    
    enum penalty_type {OTHER, META, DISCIPLINE, COURSEWORK, COURSEWORK_MAKEUP}; // 其他，纪律，作业，作业（补交）

    struct event
    {
        std::string name; // 学生名称
        int score; // 扣分分数
        std::chrono::time_point<std::chrono::system_clock> time; // 扣分时间
        penalty_type pt; // 扣分原因
        int column; // 列索引
        std::string remark; // 扣分备注

        void load_from_line(const std::string& line);
        std::string save_to_line() const;
    };

    struct event_table
    {
        std::vector<event> events; // 扣分事件

        void load(std::string path);
        void save(std::string path) const;

        int score_total() const;
        event_table no_meta_data() const;
        
        std::vector<std::string> get_all_name() const;
        event_table filter_name(std::string name) const;
        std::vector<std::chrono::time_point<std::chrono::system_clock>> get_all_time() const;
        event_table filter_time(std::chrono::time_point<std::chrono::system_clock> time) const;
        void del_time_all(std::chrono::time_point<std::chrono::system_clock> time);
        std::vector<std::chrono::year_month_day> get_all_time_day() const;
        event_table filter_time_day(std::chrono::year_month_day time) const;
        event_table filter_pt(penalty_type pt) const;
        std::vector<int> get_all_column() const;
        event_table filter_column(int column) const;
    };

    struct sclass
    {
        std::vector<sgroup> groups; // 学生姓名
        std::vector<event_table> event_tables; // 时间表格
        
        void load(std::vector<std::string> student_names_paths, std::vector<std::string> event_paths);
        void load_dir(std::string dir);
        void save(std::vector<std::string> student_names_paths, std::vector<std::string> event_paths) const;
        void save_dir(std::string dir) const;

        std::vector<std::string> get_all_name() const;
        int find_name_group(std::string name) const;
        bool is_name_same(std::string name) const;
        void set_name_no_same();
    };

    void event_table_to_csv(const std::vector<sgroup>& groups, const event_table& et, size_t column, std::string path, bool with_bom);
    void sclass_to_csv(const sclass& sc, std::string path, bool with_bom);
}

#endif