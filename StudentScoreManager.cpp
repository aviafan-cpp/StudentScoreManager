#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <format>
#include <chrono>
#include <array>
#include <cstdlib>
#include <stdexcept>
#include <climits>
#include "raylib-cpp.hpp"
#include "raylib_obj_ex.hpp"
#include "setting_system.hpp"
#include "student_score_data.hpp"
#include "sys_api.hpp"

using namespace raylib_obj_ex::str_ex;
using namespace raylib_obj_ex::coord;
using namespace raylib_obj_ex::text_ns;
using namespace raylib_obj_ex::button_ns;
using namespace raylib_obj_ex::input_ns;
using raylib_obj_ex::horizontal;
namespace roe = raylib_obj_ex;
namespace ssdata = student_score_data;
namespace sets = SettingSystem;

const unsigned int DESIGN_X = 1920;
const unsigned int DESIGN_Y = 1080;
const unsigned int DESIGN_MIN_X = 1920;
const unsigned int DESIGN_MIN_Y = 1080;
unsigned int fps = 30;

const std::string version = "1.2.0";

const std::array<raylib::Color, 6> colors {BLUE, SKYBLUE, PURPLE, ORANGE, RED, GREEN};
const std::array<std::string, 8> subjects {"语文", "数学", "英语", "科学", "道法", "地理", "历史", "副课"};

std::string current_dir, data_dir;
ssdata::sclass g_sclass;
SettingSystem::Settings settings;
enum pages {choose_res, about, class_information, group_information, student_information, choose_event_table, edit_event_table, edit_one_student_event_table, score_overview};
pages g_pages;
raylib::Font font;
bool draw_sidebar;

int show_choose_list(coord_calc_vec2& menu, float start_y, float spacing, size_t page_size, button btn_template, std::vector<std::string> content);
raylib::Color get_color(int num);
raylib::Color get_color(size_t num);
std::string format_time_chinese(const std::chrono::system_clock::time_point& tp);

int main(int argc, char *argv[])
{
    raylib::Window window(DESIGN_MIN_X, DESIGN_MIN_Y, "学生分数管理器");
    window.SetTargetFPS(fps);
    
    current_dir = GetApplicationDirectory();
    data_dir = current_dir + "Data/0/";
    std::filesystem::create_directories(current_dir + "Data/0/");
    g_sclass.load_dir(data_dir);
    settings.AddSettingFromFile(current_dir + "Data/Settings");
    roe::coord::coord_calc_vec2 g_menu(DESIGN_X, DESIGN_Y);
    font = roe::font_ex::load_font_chinese(current_dir + "Font/LXGWWenKai-Regular.ttf", 80, true, false, false, true);

    if(settings.IsSettingExist("res_x") && settings.IsSettingExist("res_y"))
    {
        window.SetSize(settings.GetSettingF("res_x"), settings.GetSettingF("res_y"));
        g_pages = choose_event_table;
        draw_sidebar = true;
    }
    
    sys_api::enable_taskbar_auto_hide();
    std::atexit(sys_api::disable_taskbar_auto_hide);

    while (!window.ShouldClose()) {
        g_menu.x.runtime = GetScreenWidth();
        g_menu.y.runtime = GetScreenHeight();

        window.BeginDrawing();
            window.ClearBackground(RAYWHITE);
            
            text text_template;
            text_template.font(font).horizontal_func(roe::Center).tint(BLUE).font_size(g_menu.calc_min(20));
            button button_template;
            button_template.font(font).horizontal_func(roe::Center).tint(RAYWHITE).text_tint(BLUE).show_border(false).text_size(g_menu.calc_min(20));
            button button_null;
            button_null.size({0, 0}).text_size(0);

            // 侧边栏
            if(draw_sidebar)
            {
                DrawLine(g_menu.x.calc_pct(20), 0, g_menu.x.calc_pct(20), g_menu.y.runtime, BLUE);
                std::vector<std::string> content = {
                    "班级信息",
                    "选择表格",
                    "编辑表格",
                    "分数总览"
                };
                button button_sidebar(button_template);
                button_sidebar
                    .position_x(g_menu.x.calc_pct(10))
                    .text_size(g_menu.calc_min(40))
                    .size_height_than_text(g_menu.y.calc(10))
                    .size_width_than_text(g_menu.x.calc(10))
                    .tint(RAYWHITE)
                    .show_border(false)
                    .text_tint(BLUE);
                int result = show_choose_list(g_menu, g_menu.y.calc_pct(0), g_menu.y.calc(45), content.size(), button_sidebar, content);
                if(result >= 0 || result < content.size())
                {
                    if(result == 0) g_pages = class_information;
                    if(result == 1) g_pages = choose_event_table;
                    if(result == 2) g_pages = edit_event_table;
                    if(result == 3) g_pages = score_overview;
                }

                std::vector<std::string> under_content = {
                    "屏幕键盘",
                    "选择分辨率",
                    "关于",
                    "撤回至上一次保存",
                    "保存",
                    "不保存并退出",
                    "保存并退出"
                };
                int under_result = show_choose_list(
                    g_menu,
                    g_menu.y.runtime - (under_content.size() - 1) * g_menu.y.calc(45) - button_sidebar.size_d.y - 10, // 10 为距离底部的距离
                    g_menu.y.calc(45),
                    under_content.size(),
                    button_sidebar, under_content
                );
                if(under_result >= 0 || under_result < under_content.size())
                {
                    if(under_result == 0){
                        #if defined(_WIN32)
                            sys_api::open_program("osk.exe");
                        #endif
                    }
                    if(under_result == 1){
                        g_pages = choose_res;
                        draw_sidebar = false;
                    }
                    if(under_result == 2) g_pages = about;
                    if(under_result == 3) g_sclass.load_dir(data_dir);
                    if(under_result == 4){
                        g_sclass.save_dir(data_dir);
                        settings.SaveSettingToFile(current_dir + "Data/Settings");
                    }
                    if(under_result == 5) return 0;
                    if(under_result == 6) break;
                }
            }


            switch (g_pages)
            {
            case choose_res:{
                text_template.position({g_menu.x.runtime / 2, g_menu.y.calc_pct(25)}).font(font).tint(BLUE).st("请设置窗口大小").horizontal_func(roe::Center).font_size(g_menu.calc_min(30)).draw();
                text_template.position_y(g_menu.y.runtime - 20).st("Copyright (c) 2026 aviafan-cpp https://github.com/aviafan-cpp").draw();
                
                button_template.font(font).position_x(g_menu.x.runtime / 2).tint(RAYWHITE).show_border(false).horizontal_func(roe::Center).text_tint(BLUE).text_size(g_menu.calc_min(30));
                std::vector<raylib::Vector2> reses = {
                    //{1280, 720},
                    //{1366, 768},
                    //{1600, 900},
                    {1920, 1080},
                    {2560, 1440},
                    {3840, 2160}
                };
                std::vector<std::string> reses_string;
                for(const auto& v2 : reses) reses_string.emplace_back(std::to_string(int(v2.x)) + " " + std::to_string(int(v2.y)));
                int result = show_choose_list(g_menu, g_menu.y.calc_pct(27.5), g_menu.calc_min(45), reses_string.size(), button_template, reses_string);
                if(result >= 0 && result < reses_string.size())
                {
                    settings.AddSetting("res_x", reses[result].x, sets::Local);
                    settings.AddSetting("res_y", reses[result].y, sets::Local);
                    window.SetSize(reses[result]);

                    g_pages = choose_event_table;
                    draw_sidebar = true;
                }
                break;
            }

            case about:{
                text text_a_big = text(text_template).position_x(g_menu.x.calc_pct(30)).font_size(g_menu.calc_min(40)).horizontal_func(roe::Left);
                text text_a = text(text_template).position_x(g_menu.x.calc_pct(30)).font_size(g_menu.calc_min(30)).horizontal_func(roe::Left);
                button button_a = button(button_template).position_x(g_menu.x.calc_pct(90)).text_size(g_menu.calc_min(30)).horizontal_func(roe::Left);

                // 作者:aviafan-cpp
                text(text_a_big).position_y(g_menu.y.calc(54)).st("作者").draw();
                text(text_a).position_y(g_menu.y.calc(100)).st("aviafan-cpp").draw();
                if(button(button_a).position_x(g_menu.x.calc_pct(80)).position_y(g_menu.y.calc(80)).text("Bilibili").text_tint(PINK).size_height_than_text(0).draw().is_clicked())
                    sys_api::open_url("https://space.bilibili.com/3546620213857006");
                if(button(button_a).position_y(g_menu.y.calc(80)).text("Github").text_tint(BLACK).size_height_than_text(0).draw().is_clicked())
                    sys_api::open_url("https://github.com/aviafan-cpp");
                DrawLine(g_menu.x.calc_pct(25), g_menu.y.calc(80), g_menu.x.calc_pct(95), g_menu.y.calc(80), BLUE);
                
                // 图形库:raylib
                text(text_a_big).position_y(g_menu.y.calc(150)).st("图形库").draw();
                text(text_a).position_y(g_menu.y.calc(195)).st("raylib").draw();
                if(button(button_a).position_x(g_menu.x.calc_pct(80)).position_y(g_menu.y.calc(175)).text("官网").size_height_than_text(0).draw().is_clicked())
                    sys_api::open_url("https://www.raylib.com/");
                if(button(button_a).position_y(g_menu.y.calc(175)).text("Github").text_tint(BLACK).size_height_than_text(0).draw().is_clicked())
                    sys_api::open_url("https://github.com/raysan5/raylib");
                DrawLine(g_menu.x.calc_pct(25), g_menu.y.calc(175), g_menu.x.calc_pct(95), g_menu.y.calc(175), BLUE);

                // 图形库:raylib-cpp
                text(text_a).position_y(g_menu.y.calc(225)).st("raylib-cpp").draw();
                if(button(button_a).position_y(g_menu.y.calc(205)).text("Github").text_tint(BLACK).size_height_than_text(0).draw().is_clicked())
                    sys_api::open_url("https://github.com/RobLoach/raylib-cpp");
                
                // 字体:LXGW
                text(text_a_big).position_y(g_menu.y.calc(275)).st("字体").draw();
                text(text_a).position_y(g_menu.y.calc(320)).st("霞鹜文楷").draw();
                if(button(button_a).position_y(g_menu.y.calc(300)).text("Github").text_tint(BLACK).size_height_than_text(0).draw().is_clicked())
                    sys_api::open_url("https://github.com/lxgw/LxgwWenKai");
                DrawLine(g_menu.x.calc_pct(25), g_menu.y.calc(300), g_menu.x.calc_pct(95), g_menu.y.calc(300), BLUE);

                // 基础信息
                text(text_a_big).position_y(g_menu.y.calc(370)).st("基础信息").draw();
                text(text_a).position_y(g_menu.y.calc(415)).st("软件名：学生分数管理器(StudentScoreManager)").draw();
                if(button(button_a).position_y(g_menu.y.calc(395)).text("Github").text_tint(BLACK).size_height_than_text(0).draw().is_clicked())
                    sys_api::open_url("https://github.com/aviafan-cpp/StudentScoreManager");
                text(text_a).position_y(g_menu.y.calc(445)).st("软件位数：" + std::to_string(sizeof(void*) * 8)).draw();
                text(text_a).position_y(g_menu.y.calc(475)).st("软件版本：" + version).draw();
                #if defined(_WIN32)
                text(text_a).position_y(g_menu.y.calc(505)).st("软件平台：Windows").draw();
                #endif
                DrawLine(g_menu.x.calc_pct(25), g_menu.y.calc(395), g_menu.x.calc_pct(95), g_menu.y.calc(395), BLUE);

                break;
            }

            case class_information:{
                text(text_template).position({g_menu.x.calc_pct(30), g_menu.y.calc_pct(4)}).font_size(g_menu.calc_min(20)).horizontal_func(roe::Left).st("点击编号修改组信息。").draw();

                button button_ci = button_template;
                button_ci.position_x(g_menu.x.calc_pct(30)).text_size(g_menu.calc_min(40)).size_height_than_text(g_menu.y.calc(10)).size_width_than_text(g_menu.x.calc(10)).horizontal_func(roe::Left);

                //if(button(button_ci).position_y(g_menu.y.calc_pct(5)).text("刷新").draw().is_clicked()) g_sclass.load_dir(data_dir);
                if(button(button_ci).position_y(g_menu.y.calc_pct(5)).text("新建").draw().is_clicked())
                {
                    g_sclass.save_dir(data_dir);
                    size_t i = 0;
                    while(std::filesystem::exists(data_dir + std::to_string(i) + ".ssdata_names")) ++i;
                    sys_api::create_file(data_dir + std::to_string(i) + ".ssdata_names", true);
                    g_sclass.load_dir(data_dir);
                }

                std::vector<std::string> names;
                for(const auto& g : g_sclass.groups)
                {
                    names.emplace_back();
                    names.back() = std::to_string(names.size()) + "组 ";
                    for(const auto& s : g.student_names) names.back() += (s + " ");
                }
                int result = show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_ci, names);
                if(result >= 0 && result < names.size())
                {
                    settings.AddSetting("choice_group", result, sets::Temp);
                    g_pages = group_information;
                    break;
                }

                std::vector<std::string> string_del, string_swap = {""};
                for(size_t i = 0; i < names.size(); ++i)
                {
                    if(i == names.size() - 1) string_del.emplace_back("删除");
                    else string_del.emplace_back("");
                    if(i != 0) string_swap.emplace_back("上移");
                }
                // 删除按钮
                button_ci.position_x(g_menu.x.calc_pct(80));
                int sd_result = show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_ci, string_del);
                if(sd_result >= 0 && sd_result < names.size() && sys_api::message_box("学生分数管理器", "您确定要删除组" + std::to_string(sd_result + 1) + "吗？", sys_api::info, sys_api::yes_no)){
                    g_sclass.groups.erase(g_sclass.groups.begin() + sd_result);
                    std::filesystem::remove(data_dir + std::to_string(sd_result) + ".ssdata_names");
                }
                // 上移按钮
                button_ci.position_x(g_menu.x.calc_pct(90));
                int ss_result = show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_ci, string_swap);
                if(ss_result >= 1 && ss_result < names.size()) std::swap(g_sclass.groups[ss_result], g_sclass.groups[ss_result - 1]);
                break;
            }

            case group_information:{
                if(g_sclass.groups.size() == 0)
                {
                    sys_api::message_box("学生分数管理器", "请创建组！", sys_api::warning, sys_api::ok);
                    g_pages = choose_event_table;
                    break;
                }
                if(!settings.IsSettingExist("choice_group"))
                {
                    sys_api::message_box("学生分数管理器", "请选择组！", sys_api::warning, sys_api::ok);
                    g_pages = choose_event_table;
                    break;
                }
                if(g_sclass.groups.size() <= settings.GetSettingI("choice_group"))
                {
                    sys_api::message_box("学生分数管理器", "选择的组不存在！", sys_api::warning, sys_api::ok);
                    g_pages = choose_event_table;
                    break;
                }

                text(text_template).position({g_menu.x.calc_pct(30), g_menu.y.calc_pct(4)}).font_size(g_menu.calc_min(20)).horizontal_func(roe::Left).st("点击名称修改单元格绑定的学生。").draw();

                button button_gi = button_template;
                button_gi.position_x(g_menu.x.calc_pct(30)).text_size(g_menu.calc_min(40)).size_height_than_text(g_menu.y.calc(10)).size_width_than_text(g_menu.x.calc(10)).horizontal_func(roe::Left);

                // 新建按钮
                if(button(button_gi).position_y(g_menu.y.calc_pct(5)).text("新建").draw().is_clicked())
                    settings.AddSetting("set_student_name", INT_MAX, sets::Temp);

                // 修改单元格绑定
                std::vector<std::string> names;
                for(const auto& s : g_sclass.groups[settings.GetSettingI("choice_group")].student_names)
                    names.emplace_back(s);
                int result = show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_gi, names);
                if(result >= 0 && result < names.size()){
                    if(!settings.IsSettingExist("dont_say_change_cell_binding_will_not_change_data"))
                    {
                        bool mb_result = sys_api::message_box("学生分数管理器", "提示：\n此处是修改绑定学生名称的功能，不会修改此人绑定的数据。\n想要修改数据绑定请到“信息”一栏的“重命名”！\n“确定”代表不再提示。", sys_api::info, sys_api::ok_cencle);
                        if(mb_result) settings.AddSetting("dont_say_change_cell_binding_will_not_change_data", 1, sets::Local);
                    }
                    settings.AddSetting("set_student_name", result, sets::Temp);
                }

                std::vector<std::string> string_information, string_del, string_swap = {""};
                for(size_t i = 0; i < names.size(); ++i)
                {
                    string_information.emplace_back("信息");
                    string_del.emplace_back("删除");
                    if(i != 0) string_swap.emplace_back("上移");
                }
                // 信息按钮
                button_gi.position_x(g_menu.x.calc_pct(70));
                int si_result = show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_gi, string_information);
                if(si_result >= 0 && si_result < names.size())
                {
                    settings.AddSetting("choice_student", g_sclass.groups[settings.GetSettingI("choice_group")].student_names[si_result], sets::Temp);
                    g_pages = student_information;
                    break;
                }

                // 删除按钮
                button_gi.position_x(g_menu.x.calc_pct(80));
                int sd_result = show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_gi, string_del);
                if(sd_result >= 0 && sd_result < names.size() && sys_api::message_box("学生分数管理器", "您确定要删除" + g_sclass.groups[settings.GetSettingI("choice_group")].student_names[sd_result] + "吗？", sys_api::info, sys_api::yes_no))
                    g_sclass.groups[settings.GetSettingI("choice_group")].student_names.erase(g_sclass.groups[settings.GetSettingI("choice_group")].student_names.begin() + sd_result, g_sclass.groups[settings.GetSettingI("choice_group")].student_names.begin() + sd_result + 1);
                
                // 上移按钮
                button_gi.position_x(g_menu.x.calc_pct(90));
                int ss_result = show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_gi, string_swap);
                if(ss_result >= 1 && ss_result < names.size())
                    std::swap(g_sclass.groups[settings.GetSettingI("choice_group")].student_names[ss_result], g_sclass.groups[settings.GetSettingI("choice_group")].student_names[ss_result - 1]);

                // 新建
                if(settings.GetSettingI("set_student_name") != -1)
                {
                    DrawLine(g_menu.x.calc_pct(22), g_menu.y.calc_pct(85), g_menu.x.calc_pct(95), g_menu.y.calc_pct(85), BLUE);
                    auto title = text(text_template).horizontal_func(roe::Left).position({g_menu.x.calc_pct(30), g_menu.y.calc_pct(87.5)}).font_size(g_menu.calc_min(25)).st("请输入新名称");
                    auto content = text(text_template).horizontal_func(roe::Left).position({g_menu.x.calc_pct(30), g_menu.y.calc_pct(92.5)}).font_size(g_menu.calc_min(30));
                    auto cencle = button(button_template).horizontal_func(roe::Left).position({g_menu.x.calc_pct(30), g_menu.y.calc_pct(95)}).text("取消");
                    auto clear = button(button_template).horizontal_func(roe::Left).position({g_menu.x.calc_pct(40), g_menu.y.calc_pct(95)}).text("清空").draw();
                    auto ok = button(button_template).horizontal_func(roe::Left).position({g_menu.x.calc_pct(50), g_menu.y.calc_pct(95)}).text("确定");
                    static input_field input_f;                     // 只保留内容，不初始化绘制属性
                    input_f.draw_title = title;                     // 更新绘制属性为当前动态值
                    input_f.draw_content = content;
                    input_f.draw_cancle = cencle;
                    input_f.draw_ok = ok;
                    input_f.index_color = BLUE;
                    input_f.update_draw();

                    if(input_f.draw_cancle.is_clicked())
                    {
                        input_f.content.codepoints.clear();
                        settings.AddSetting("set_student_name", -1, sets::Temp);
                    }
                    if(clear.is_clicked())
                        input_f.content.codepoints.clear();
                    if((input_f.draw_ok.is_clicked() || IsKeyPressed(KEY_ENTER)) && input_f.content.codepoints.size())
                    {
                        if(g_sclass.is_name_same(input_f.content.to_str()))
                        {
                            sys_api::message_box("学生分数管理器", "名称已重复！", sys_api::warning);
                            break;
                        }

                        if(settings.GetSettingI("set_student_name") == INT_MAX)
                            g_sclass.groups[settings.GetSettingI("choice_group")].student_names.emplace_back(input_f.content.to_str());
                        else
                        {
                            try
                            {
                                g_sclass.groups[settings.GetSettingI("choice_group")].student_names.at(settings.GetSettingI("set_student_name")) = input_f.content.to_str();
                            }
                            catch(const std::out_of_range& e)
                            {
                                sys_api::message_box("学生分数管理器", std::string("数组越界：") + e.what(), sys_api::warning);
                            }
                        }

                        input_f.content.codepoints.clear();
                        settings.AddSetting("set_student_name", -1, sets::Temp);
                    }

                    // 换行符自动确认机制
                    if(settings.GetSettingI("set_student_name") == INT_MAX)
                    {
                        auto treat_newline_as_ok = button(button_template)
                            .horizontal_func(roe::Left)
                            .position({g_menu.x.calc_pct(60), g_menu.y.calc_pct(95)})
                            .text("将换行符视为确认")
                            .text_tint(settings.GetSettingI("dont_treat_newline_as_ok") > 0 ? BLUE : PURPLE)
                            .draw();
                        if(treat_newline_as_ok.is_clicked())
                        {
                            if(settings.GetSettingI("dont_treat_newline_as_ok") > 0) settings.AddSetting("dont_treat_newline_as_ok", 0, sets::Temp);
                            else settings.AddSetting("dont_treat_newline_as_ok", 1, sets::Temp);
                        }

                        if(settings.GetSettingI("dont_treat_newline_as_ok") <= 0 && !input_f.content.codepoints.empty())
                        {
                            auto& codepoints = input_f.content.codepoints;
                            bool changed = false;
                            while (true)
                            {
                                // 查找换行符 (codepoint = 10)
                                auto it = std::find(codepoints.begin(), codepoints.end(), 10);
                                if (it == codepoints.end())
                                    break;

                                // 提取换行符前的 codepoints
                                std::vector<int> before(codepoints.begin(), it);
                                if (!before.empty())
                                {
                                    roe::str_ex::ustring before_ustr;
                                    before_ustr.codepoints = before;
                                    std::string name = before_ustr.to_str();
                                    // 添加至当前组
                                    g_sclass.groups[settings.GetSettingI("choice_group")].student_names.emplace_back(name);
                                }

                                // 移除已处理部分（含换行符）
                                codepoints.erase(codepoints.begin(), it + 1);
                                changed = true;
                            }

                            if (changed)
                            {
                                // 将光标移至末尾（因为删除了前面的内容）
                                input_f.index = codepoints.size();

                                // 处理学生名称中的换行符和回车符
                                for(auto& s : g_sclass.groups[settings.GetSettingI("choice_group")].student_names)
                                {
                                    std::erase(s, '\n');
                                    std::erase(s, '\r');
                                }
                            }
                        }
                    }
                }
                
                break;
            }

            case student_information:{
                if(!settings.IsSettingExist("choice_student"))
                {
                    sys_api::message_box("学生分数管理器", "找不到choice_student", sys_api::error, sys_api::ok);
                    g_pages = edit_event_table;
                    break;
                }

                button button_si = button_template;
                button_si.position_x(g_menu.x.calc_pct(30)).text_size(g_menu.calc_min(40)).size_height_than_text(g_menu.y.calc(10)).size_width_than_text(g_menu.x.calc(10)).horizontal_func(roe::Left);

                static bool change_name = false;

                // 顶边栏
                button(button_si).position_y(g_menu.y.calc_pct(5)).text(settings.GetSetting("choice_student")).draw().is_clicked();
                if(button(button_si).position_x(g_menu.x.calc_pct(80)).position_y(g_menu.y.calc_pct(5)).text("重命名").draw().is_clicked())
                {
                    if(!settings.IsSettingExist("dont_say_rename_will_change_data"))
                    {
                        bool mb_result = sys_api::message_box("学生分数管理器", "提示：\n此处是修改学生绑定的数据的功能。\n想要修改单元格显示的名称请到“组信息”界面点击名称！\n“确定”代表不再提示。", sys_api::info, sys_api::ok_cencle);
                        if(mb_result) settings.AddSetting("dont_say_rename_will_change_data", 1, sets::Local);
                    }
                    change_name = true;
                }

                // 中间内容
                ssdata::event_table student_event_table;
                for(const auto& et : g_sclass.event_tables){
                    auto student_et = et.no_meta_data().filter_name(settings.GetSetting("choice_student"));
                    student_event_table.events.insert(student_event_table.events.end(), student_et.events.begin(), student_et.events.end());
                }
                std::vector<std::string> student_data = {
                    "组别：" + std::to_string(g_sclass.find_name_group(settings.GetSetting("choice_student")) + 1),
                    "总分：" + std::to_string(student_event_table.score_total()) + "分（" + std::to_string(student_event_table.events.size()) + "次）",
                    ">纪律原因：" + std::to_string(student_event_table.filter_pt(ssdata::DISCIPLINE).score_total()) + "分（" + std::to_string(student_event_table.filter_pt(ssdata::DISCIPLINE).events.size()) + "次）",
                    ">作业原因：" + std::to_string(student_event_table.filter_pt(ssdata::COURSEWORK).score_total()) + "分（" + std::to_string(student_event_table.filter_pt(ssdata::COURSEWORK).events.size()) + "次）",
                    ">作业补交：" + std::to_string(student_event_table.filter_pt(ssdata::COURSEWORK_MAKEUP).score_total()) + "分（" + std::to_string(student_event_table.filter_pt(ssdata::COURSEWORK_MAKEUP).events.size()) + "次）",
                    ">其他原因：" + std::to_string(student_event_table.filter_pt(ssdata::OTHER).score_total()) + "分（" + std::to_string(student_event_table.filter_pt(ssdata::OTHER).events.size()) + "次）"
                };
                show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), student_data.size(), button_si, student_data);

                // 分割线，为了最高层级放到最后面
                DrawLine(g_menu.x.calc_pct(22), g_menu.y.calc_pct(10), g_menu.x.calc_pct(95), g_menu.y.calc_pct(10), BLUE);
                
                // 输入栏
                if(change_name)
                {
                    DrawLine(g_menu.x.calc_pct(22), g_menu.y.calc_pct(85), g_menu.x.calc_pct(95), g_menu.y.calc_pct(85), BLUE);
                    auto title = text(text_template).horizontal_func(roe::Left).position({g_menu.x.calc_pct(30), g_menu.y.calc_pct(87.5)}).font_size(g_menu.calc_min(25)).st("请输入新名称");
                    auto content = text(text_template).horizontal_func(roe::Left).position({g_menu.x.calc_pct(30), g_menu.y.calc_pct(92.5)}).font_size(g_menu.calc_min(30));
                    auto cencle = button(button_template).horizontal_func(roe::Left).position({g_menu.x.calc_pct(30), g_menu.y.calc_pct(95)}).text("取消");
                    auto clear = button(button_template).horizontal_func(roe::Left).position({g_menu.x.calc_pct(40), g_menu.y.calc_pct(95)}).text("清空").draw();
                    auto ok = button(button_template).horizontal_func(roe::Left).position({g_menu.x.calc_pct(50), g_menu.y.calc_pct(95)}).text("确定");
                    static input_field input_f;                     // 只保留内容，不初始化绘制属性
                    input_f.draw_title = title;                     // 更新绘制属性为当前动态值
                    input_f.draw_content = content;
                    input_f.draw_cancle = cencle;
                    input_f.draw_ok = ok;
                    input_f.index_color = BLUE;
                    input_f.update_draw();

                    if(input_f.draw_cancle.is_clicked())
                    {
                        input_f.content.codepoints.clear();
                        change_name = false;
                    }
                    if(clear.is_clicked())
                        input_f.content.codepoints.clear();
                    if((input_f.draw_ok.is_clicked() || IsKeyPressed(KEY_ENTER)) && input_f.content.codepoints.size())
                    {
                        if(g_sclass.is_name_same(input_f.content.to_str()))
                        {
                            sys_api::message_box("学生分数管理器", "名称已重复！", sys_api::warning);
                            break;
                        }

                        // 转换
                        for(auto& g : g_sclass.groups)
                        {
                            for(auto& s : g.student_names)
                            {
                                if(s == settings.GetSetting("choice_student")) s = input_f.content.to_str();
                            }
                        }
                        for(auto& et : g_sclass.event_tables)
                        {
                            for(auto& e : et.events)
                            {
                                if(e.name == settings.GetSetting("choice_student")) e.name = input_f.content.to_str();
                            }
                        }

                        sys_api::message_box("学生分数管理器", "重命名成功！");
                        change_name = false;
                        input_f.content.codepoints.clear();
                        g_pages = class_information;
                        break;
                    }
                }

                break;
            }

            case choose_event_table:{
                button button_ci = button_template;
                button_ci.position_x(g_menu.x.calc_pct(30)).text_size(g_menu.calc_min(40)).size_height_than_text(g_menu.y.calc(10)).size_width_than_text(g_menu.x.calc(10)).horizontal_func(roe::Left);

                if(button(button_ci).position_y(g_menu.y.calc_pct(5)).text("新建").draw().is_clicked())
                {
                    g_sclass.save_dir(data_dir);
                    size_t i = 0;
                    while(std::filesystem::exists(data_dir + std::to_string(i) + ".ssdata_event_table")) ++i;
                    sys_api::create_file(data_dir + std::to_string(i) + ".ssdata_event_table", true);
                    g_sclass.load_dir(data_dir);

                    // 增加时间元数据
                    auto& tm = g_sclass.event_tables.back().events.emplace_back();
                    tm.name = "TIME_MARK",
                    tm.score = 0,
                    tm.time = std::chrono::system_clock::now() + i * std::chrono::hours(24),
                    tm.pt = ssdata::META,
                    tm.column = -1,
                    tm.remark = "元数据:时间";
                    g_sclass.save_dir(data_dir);
                }
                
                if(
                    button(button_ci).position_x(g_menu.x.calc_pct(45)).position_y(g_menu.y.calc_pct(5)).text("汇总到.csv").draw().is_clicked() &&
                    sys_api::message_box("学生分数管理器", "你确定要在软件根目录创建汇总的.csv文件吗？", sys_api::info, sys_api::yes_no)
                ){
                    bool with_bom = sys_api::message_box("学生分数管理器", "是否要在文件头创建BOM？\n我们建议普通办公用户选择“是”。", sys_api::info, sys_api::yes_no);

                    std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    std::tm local_tm = *std::localtime(&tt);
                    std::ostringstream oss;
                    // 注意：put_time 需要指针
                    oss << std::put_time(&local_tm, "%Y-%m-%d %H-%M-%S");
                    std::string csv_name = "total " + oss.str() + ".csv";

                    try
                    {
                        ssdata::sclass_to_csv(g_sclass, current_dir + csv_name, with_bom);
                    }
                    catch(const std::exception& e)
                    {
                        sys_api::message_box("学生分数管理器", std::string("文件创建失败，原因：") + e.what(), sys_api::warning);
                        break;
                    }

                    sys_api::message_box("学生分数管理器", current_dir + csv_name + "文件创建完成。");
                }

                std::vector<std::string> names;
                for(const auto& g : g_sclass.event_tables)
                {
                    names.emplace_back();
                    if(g.events.size() != 0)
                    {
                        std::time_t tt = std::chrono::system_clock::to_time_t(g.events[0].time);
                        std::tm local_tm = *std::localtime(&tt);
                        std::ostringstream oss;
                        // 注意：put_time 需要指针
                        oss << std::put_time(&local_tm, "%Y年%m月%d日 %H:%M:%S");
                        names.back() = std::to_string(names.size()) + "表 " + oss.str();
                    }
                    else
                        names.back() = std::to_string(names.size()) + "表";
                }
                show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_ci, names);

                if(!settings.IsSettingExist("event_table_num")) settings.AddSetting("event_table_num", 0, sets::Temp); // 如果没有选择的表格，就设置为0

                std::vector<std::string> string_choice, string_del, string_swap;
                for(size_t i = 0; i < names.size(); ++i)
                {
                    if(settings.GetSettingI("event_table_num") != i) string_choice.emplace_back("选择");
                    else string_choice.emplace_back("");
                    if(i == names.size() - 1) string_del.emplace_back("删除");
                    else string_del.emplace_back("");
                    if(i != 0) string_swap.emplace_back("上移");
                    else string_swap.emplace_back("");
                }
                // 选择按钮
                button_ci.position_x(g_menu.x.calc_pct(70));
                int sc_result = show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_ci, string_choice);
                if(sc_result >= 0 && sc_result < names.size()) settings.AddSetting("event_table_num", sc_result, sets::Temp);
                // 删除按钮
                button_ci.position_x(g_menu.x.calc_pct(80));
                int sd_result = show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_ci, string_del);
                if(sd_result >= 0 && sd_result < names.size() && sys_api::message_box("学生分数管理器", "您确定要删除表" + std::to_string(sd_result + 1) + "吗？", sys_api::info, sys_api::yes_no)){
                    g_sclass.event_tables.erase(g_sclass.event_tables.begin() + sd_result);
                    std::filesystem::remove(data_dir + std::to_string(sd_result) + ".ssdata_event_table");
                }
                // 上移按钮
                button_ci.position_x(g_menu.x.calc_pct(90));
                int ss_result = show_choose_list(g_menu, g_menu.y.calc_pct(10), g_menu.y.calc(45), names.size(), button_ci, string_swap);
                if(ss_result >= 1 && ss_result < names.size()) std::swap(g_sclass.event_tables[ss_result], g_sclass.event_tables[ss_result - 1]);
                break;
            }

            case edit_event_table:{
                if(g_sclass.event_tables.size() == 0)
                {
                    sys_api::message_box("学生分数管理器", "请创建表格！", sys_api::warning, sys_api::ok);
                    g_pages = choose_event_table;
                    break;
                }
                if(!settings.IsSettingExist("event_table_num"))
                {
                    sys_api::message_box("学生分数管理器", "请选择表格！", sys_api::warning, sys_api::ok);
                    g_pages = choose_event_table;
                    break;
                }
                if(g_sclass.event_tables.size() <= settings.GetSettingI("event_table_num"))
                {
                    sys_api::message_box("学生分数管理器", "选择的表格不存在！", sys_api::warning, sys_api::ok);
                    g_pages = choose_event_table;
                    break;
                }
                if(g_sclass.get_all_name().size() > 48 && settings.GetSettingI("dont_say_students_too_much") != 1)
                {
                    bool result = sys_api::message_box("学生分数管理器", "同学数量太多了！\n（确认代表不再报此错误）", sys_api::warning, sys_api::ok_cencle);
                    if(result)
                    {
                        settings.AddSetting("dont_say_students_too_much", 1, sets::Local);
                    }
                    else
                    {
                        g_pages = class_information;
                        break;
                    }
                }

                size_t et_num = settings.GetSettingI("event_table_num");
                auto draw_et = g_sclass.event_tables[et_num].no_meta_data();

                text(text_template)
                    .horizontal_func(roe::Left)
                    .position({g_menu.x.calc_pct(22), g_menu.y.calc_pct(2.5)})
                    .font_size(g_menu.calc_min(30))
                    .st("表" + std::to_string(et_num + 1) + " " + data_dir + std::to_string(et_num) + ".ssdata_event_table")
                    .draw();

                button bt_to_csv = button_template;
                bt_to_csv.position({g_menu.x.calc_pct(85), g_menu.y.calc_pct(1)}).text_size(g_menu.calc_min(30)).text("打包为.csv").draw();
                if(bt_to_csv.is_clicked() && sys_api::message_box("学生分数管理器", "你确定要在软件根目录创建本表的.csv文件吗？", sys_api::info, sys_api::yes_no))
                {
                    bool with_bom = sys_api::message_box("学生分数管理器", "是否要在文件头创建BOM？\n我们建议普通办公用户选择“是”。", sys_api::info, sys_api::yes_no);

                    std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    std::tm local_tm = *std::localtime(&tt);
                    std::ostringstream oss;
                    // 注意：put_time 需要指针
                    oss << std::put_time(&local_tm, "%Y-%m-%d %H-%M-%S");
                    std::string csv_name = std::to_string(et_num + 1) + " " + oss.str() + ".csv";

                    try
                    {
                        ssdata::event_table_to_csv(g_sclass.groups, draw_et, 5, current_dir + csv_name, with_bom);
                    }
                    catch(const std::exception& e)
                    {
                        sys_api::message_box("学生分数管理器", std::string("文件创建失败，原因：") + e.what(), sys_api::warning);
                        break;
                    }

                    sys_api::message_box("学生分数管理器", current_dir + csv_name + "文件创建完成。");
                }

                button bt_legend = button_template;
                bt_legend.position({g_menu.x.calc_pct(95), g_menu.y.calc_pct(1)}).text_size(g_menu.calc_min(30)).text("图例").draw();
                if(bt_legend.is_clicked())
                {
                    sys_api::message_box(
                        "学生分数管理器",
                        "棕色（默认颜色）：无加分/扣分\n蓝色：其他原因\n紫色：纪律原因\n橙色：作业原因\n红色：作业补交\n粉色：混合原因"
                    );
                }

                const size_t ROW_X = 7, ROW_Y = 24;

                std::vector<std::array<std::string, ROW_X>> table;
                for(const auto& s : g_sclass.get_all_name())
                {
                    auto& t = table.emplace_back();
                    auto st = draw_et.filter_name(s);

                    int sum = 0;
                    t[0] = s;
                    for(size_t i = 1; i < ROW_X - 1; ++i)
                    {
                        t[i] = std::to_string(st.filter_column(i - 1).score_total());
                        sum += std::stoi(t[i]);
                    }
                    t[6] = std::to_string(sum);
                }

                try
                {
                    // 绘制（2大列）
                    const float x_start = g_menu.x.calc_pct(20), x_end = g_menu.x.runtime, x_center = x_start + (x_end - x_start) / 2; // x轴常量
                    const float y_start = g_menu.y.calc(48), y_end = g_menu.y.runtime; // y轴常量
                    for(size_t chlomn = 0; chlomn < 2; ++chlomn)
                    {
                        // 当前大列x的起始和结束
                        float draw_x_start, draw_x_end;
                        if(chlomn == 0) draw_x_start = x_start, draw_x_end = x_center;
                        else draw_x_start = x_center, draw_x_end = x_end;

                        button bt_eet = button_template;
                        bt_eet.size({(draw_x_end - draw_x_start) / ROW_X, (y_end - y_start) / ROW_Y}); // x和y轴的偏移量均基于此
                        float y = y_start;
                        for(size_t i = 0; i < ROW_Y; ++i) // y轴
                        {
                            float x = draw_x_start + bt_eet.size_d.x / 2;
                            for(size_t j = 0; j < ROW_X; ++j) // x轴
                            {
                                // 选择对应的颜色
                                bt_eet.text_tint_d = DARKGRAY;
                                if(j == 0) bt_eet.text_tint_d = get_color(g_sclass.find_name_group(table.at(i + chlomn * ROW_Y).at(j)));
                                else
                                {
                                    int pt = INT_MIN; // INT_MIN代表无，INT_MAX代表混合
                                    auto s_et = draw_et.filter_name(table.at(i + chlomn * ROW_Y).at(0));
                                    if(j != ROW_X - 1) s_et = s_et.filter_column(j - 1);
                                    for(const auto& e : s_et.events)
                                    {
                                        if(pt == INT_MIN) pt = e.pt;
                                        else if(pt != e.pt) 
                                        {
                                            pt = INT_MAX;
                                            break;
                                        }
                                    }

                                    if(pt != INT_MIN && pt != INT_MAX) bt_eet.text_tint_d = get_color(pt);
                                    if(pt == INT_MAX) bt_eet.text_tint_d = PINK;
                                }

                                if(bt_eet.text(table.at(i + chlomn * ROW_Y).at(j)).position({x, y}).text_size(g_menu.calc_min(35)).draw().is_clicked())
                                {
                                    if(j != 0)
                                    {
                                        settings.AddSetting("choice_student", table.at(i + chlomn * ROW_Y).at(0), sets::Temp);
                                        settings.AddSetting("choice_column", j != ROW_X - 1 ? int(j - 1) : INT_MAX, sets::Temp);
                                        g_pages = edit_one_student_event_table;
                                        break;
                                    }
                                    else
                                    {
                                        settings.AddSetting("choice_student", table.at(i + chlomn * ROW_Y).at(0), sets::Temp);
                                        g_pages = student_information;
                                        break;
                                    }
                                }
                                x += bt_eet.size_d.x;
                            }
                            y += bt_eet.size_d.y;
                        }
                    }
                }
                catch(const std::out_of_range& e)
                {
                    // bool result = sys_api::message_box("学生分数管理器", "绘制过程中发生数组访问越界：" + std::string(e.what()), sys_api::warning, sys_api::retry_cancel);
                    // if(!result) g_pages = choose_event_table;
                    // break;
                    // 为了使学生数量小于 48 也能正常运行，不对该错误进行处理。
                }

                break;
            }

            case edit_one_student_event_table:
            {
                if(!settings.IsSettingExist("event_table_num"))
                {
                    sys_api::message_box("学生分数管理器", "请选择表格", sys_api::warning, sys_api::ok);
                    g_pages = choose_event_table;
                    break;
                }
                if(!settings.IsSettingExist("choice_student"))
                {
                    sys_api::message_box("学生分数管理器", "找不到choice_student", sys_api::error, sys_api::ok);
                    g_pages = edit_event_table;
                    break;
                }
                if(!settings.IsSettingExist("choice_column"))
                {
                    sys_api::message_box("学生分数管理器", "找不到choice_column", sys_api::error, sys_api::ok);
                    g_pages = edit_event_table;
                    break;
                }

                const float center_x = g_menu.x.calc_pct(60);
                if(settings.GetSettingI("choice_column") != INT_MAX) DrawLine(center_x, g_menu.y.calc_pct(2.5), center_x, g_menu.y.calc_pct(97.5), BLUE);

                text(text_template).position({g_menu.x.calc_pct(22), g_menu.y.calc_pct(4)}).font_size(g_menu.calc_min(20)).horizontal_func(roe::Left).st("点击删除对应记录。").draw();

                // 历史记录展示
                auto draw_et = g_sclass.event_tables[settings.GetSettingI("event_table_num")]
                    .no_meta_data()
                    .filter_name(settings.GetSetting("choice_student"));
                if(settings.GetSettingI("choice_column") != INT_MAX) draw_et = draw_et.filter_column(settings.GetSettingI("choice_column"));
                
                std::vector<std::string> draw_et_string;
                for(const auto& e : draw_et.events)
                {
                    draw_et_string.emplace_back(format_time_chinese(e.time) + " " + std::to_string(e.score) + "分");
                    if(e.remark != "") draw_et_string.back() += ":" + e.remark;

                    // 在全局展示页面显示列
                    if(settings.GetSettingI("choice_column") == INT_MAX) draw_et_string.back() = std::to_string(e.column + 1) + "列 " + draw_et_string.back();
                }
                button bt_show_et = button_template;
                bt_show_et.position_x(g_menu.x.calc_pct(22)).text_size(g_menu.calc_min(30)).horizontal_func(roe::Left);
                int result = show_choose_list(g_menu, g_menu.y.calc_pct(5), g_menu.y.calc(45), draw_et_string.size(), bt_show_et, draw_et_string);

                if(
                    result >= 0 &&
                    sys_api::message_box(
                        "学生分数管理器",
                        "您确定要删除" + format_time_chinese(draw_et.events[result].time) + "的这一数据吗？",
                        sys_api::info,
                        sys_api::yes_no
                    )
                )
                {
                    g_sclass.event_tables[settings.GetSettingI("event_table_num")].del_time_all(draw_et.events[result].time);
                }
                
                if(settings.GetSettingI("choice_column") != INT_MAX)
                {
                    // 添加扣分的界面
                    static int choose_score;
                    static ssdata::penalty_type choose_pt;
                    static size_t choose_subject_index; // 使用时取subjects[choose_subject_index]

                    // 选择分数
                    const int SCORE_START = -5, SCORE_END = 5;
                    std::vector<std::string> score_list;
                    for(int i = SCORE_START; i <= SCORE_END; ++i)
                    {
                        score_list.emplace_back();
                        score_list.back() += (i == choose_score ? '>' : ' ');
                        score_list.back() += std::to_string(i) + "分";
                    }
                    button bt_show_score = bt_show_et;
                    bt_show_score.position_x(g_menu.x.calc_pct(65)).horizontal_func(roe::Left);
                    int choose_score_list = show_choose_list(g_menu, g_menu.y.calc_pct(5), g_menu.y.calc(45), score_list.size(), bt_show_score, score_list);
                    if(choose_score_list != -1) choose_score = SCORE_START + choose_score_list;

                    // 选择扣分原因
                    std::vector<std::string> pt_list;
                    for(size_t i = 0; i < 5; ++i)
                    {
                        if(i == 1) continue; // 跳过 META_DATA
                        
                        pt_list.emplace_back();
                        
                        // 计算当前项在 pt_list 中的索引位置
                        size_t pt_list_index = (i > 1) ? i - 1 : i;
                        
                        // 计算 choose_pt 在 pt_list 中对应的索引
                        int choose_pt_index = static_cast<int>(choose_pt);
                        if(choose_pt_index > 1) choose_pt_index -= 1;
                        
                        // 判断是否选中
                        pt_list.back() += (pt_list_index == static_cast<size_t>(choose_pt_index) ? '>' : ' ');
                        
                        std::string pt_str;
                        switch (i)
                        {
                        case 0:
                            pt_str = "其他";
                            break;
                        case 2:
                            pt_str = "纪律";
                            break;
                        case 3:
                            pt_str = "作业未交";
                            break;
                        case 4:
                            pt_str = "作业补交";
                            break;
                        default:
                            pt_str = "";
                            break;
                        }
                        pt_list.back() += pt_str;
                    }

                    button bt_show_pt = bt_show_et;
                    bt_show_pt.position_x(g_menu.x.calc_pct(75)).horizontal_func(roe::Left);
                    int choose_pt_list = show_choose_list(g_menu, g_menu.y.calc_pct(5), g_menu.y.calc(45), pt_list.size(), bt_show_pt, pt_list);
                    if(choose_pt_list != -1)
                    {
                        // 将 pt_list 索引映射回 penalty_type
                        int real_pt = choose_pt_list;
                        if(real_pt >= 1) real_pt += 1;
                        choose_pt = static_cast<ssdata::penalty_type>(real_pt);
                    }

                    // 选择remark的科目
                    std::vector<std::string> subject_list;
                    for(size_t i = 0; i < subjects.size(); ++i)
                    {
                        subject_list.emplace_back();
                        subject_list.back() += (i == choose_subject_index ? '>' : ' ');
                        subject_list.back() += subjects[i];
                    }
                    button bt_show_subject = bt_show_et;
                    bt_show_subject.position_x(g_menu.x.calc_pct(85)).horizontal_func(roe::Left);
                    int choose_subject_list = show_choose_list(g_menu, g_menu.y.calc_pct(5), g_menu.y.calc(45), subject_list.size(), bt_show_subject, subject_list);
                    if(choose_subject_list != -1) choose_subject_index = choose_subject_list;

                    // 确定按钮
                    button bt_add = bt_show_et;
                    bt_add.position({g_menu.x.calc_pct(85), g_menu.y.calc_pct(85)}).horizontal_func(roe::Left).text("添加");
                    if(bt_add.draw().is_clicked())
                    {
                        ssdata::event add_event;
                        add_event.name = settings.GetSetting("choice_student");
                        add_event.score = choose_score;
                        
                        // 处理时间，确保不重复
                        auto now_tp = std::chrono::system_clock::now();
                        while(true)
                        {
                            bool conflict = false;
                            for(const auto& t : g_sclass.event_tables[settings.GetSettingI("event_table_num")].get_all_time())
                            {
                                if(t == now_tp)
                                {
                                    conflict = true;
                                    break;
                                }
                            }
                            if(!conflict) break;
                            now_tp += std::chrono::seconds(1);
                        }
                        add_event.time = now_tp;

                        add_event.pt = choose_pt;
                        add_event.column = settings.GetSettingI("choice_column");

                        // 处理备注
                        if(choose_pt == ssdata::OTHER)
                            add_event.remark = "因" + subjects[choose_subject_index] + "相关原因";
                        if(choose_pt == ssdata::DISCIPLINE)
                            add_event.remark = "因" + subjects[choose_subject_index] + "纪律问题";
                        if(choose_pt == ssdata::COURSEWORK)
                            add_event.remark = "因" + subjects[choose_subject_index] + "作业原因";
                        if(choose_pt == ssdata::COURSEWORK_MAKEUP)
                            add_event.remark = "因" + subjects[choose_subject_index] + "作业补交";
                        
                        g_sclass.event_tables[settings.GetSettingI("event_table_num")].events.emplace_back(add_event);
                    }
                }

                break;
            }

            case score_overview:
            {
                if(!settings.IsSettingExist("event_table_num"))
                {
                    sys_api::message_box("学生分数管理器", "请选择表格", sys_api::warning, sys_api::ok);
                    g_pages = choose_event_table;
                    break;
                }
                if(g_sclass.event_tables.size() <= settings.GetSettingI("event_table_num"))
                {
                    sys_api::message_box("学生分数管理器", "选择的表格不存在！", sys_api::warning, sys_api::ok);
                    g_pages = choose_event_table;
                    break;
                }

                struct overview
                {
                    size_t group_name;
                    ssdata::sgroup group;
                    int score_title;

                    std::string to_string() const
                    {
                        std::string result = std::to_string(group_name + 1) + "组";
                        for(const auto& s : group.student_names) result += " " + s;
                        result += " " + std::to_string(score_title) + "分";

                        return result;
                    }
                };

                std::vector<overview> draw_overviews;
                for(size_t i = 0; i < g_sclass.groups.size(); ++i)
                {
                    overview new_overview{};
                    new_overview.group_name = i;
                    new_overview.group = g_sclass.groups[i];
                    for(const auto& s : g_sclass.groups[i].student_names)
                    {
                        new_overview.score_title += g_sclass.event_tables[settings.GetSettingI("event_table_num")].no_meta_data().filter_name(s).score_total();
                    }
                    draw_overviews.emplace_back(new_overview);
                }

                enum sort_model {GROUP_NAME_MIN_TO_MAX, SCORE_TITLE_MIN_TO_MAX, SCORE_TITLE_MAX_TO_MIN};
                static sort_model show_model;

                button button_sm = button_template;
                button_sm.position_y(g_menu.y.calc_pct(1.5)).horizontal_func(roe::Left).text_size(g_menu.calc_min(40)).size_width(g_menu.x.calc(200));
                if(button_sm.position_x(g_menu.x.calc_pct(30)).text("按组名排序").text_tint((show_model == GROUP_NAME_MIN_TO_MAX ? PURPLE : BLUE)).draw().is_clicked()) show_model = GROUP_NAME_MIN_TO_MAX;
                if(button_sm.position_x(g_menu.x.calc_pct(45)).text("按分数排序（从小到大）").text_tint((show_model == SCORE_TITLE_MIN_TO_MAX ? PURPLE : BLUE)).draw().is_clicked()) show_model = SCORE_TITLE_MIN_TO_MAX;
                if(button_sm.position_x(g_menu.x.calc_pct(70)).text("按分数排序（从大到小）").text_tint((show_model == SCORE_TITLE_MAX_TO_MIN ? PURPLE : BLUE)).draw().is_clicked()) show_model = SCORE_TITLE_MAX_TO_MIN;

                switch(show_model)
                {
                    case GROUP_NAME_MIN_TO_MAX:
                        std::sort(draw_overviews.begin(), draw_overviews.end(), [](const overview& a, const overview& b){ return a.group_name < b.group_name; });
                        break;
                    case SCORE_TITLE_MIN_TO_MAX:
                        std::sort(draw_overviews.begin(), draw_overviews.end(), [](const overview& a, const overview& b){ return a.score_title < b.score_title; });
                        break;
                    case SCORE_TITLE_MAX_TO_MIN:
                        std::sort(draw_overviews.begin(), draw_overviews.end(), [](const overview& a, const overview& b){ return a.score_title > b.score_title; });
                        break;
                    default:
                        break;
                }

                std::vector<std::string> draw_overviews_s;
                draw_overviews_s.resize(draw_overviews.size());
                std::transform(draw_overviews.begin(), draw_overviews.end(), draw_overviews_s.begin(), [](const overview& s){ return s.to_string(); });

                button button_so = button_template;
                button_so.position_x(g_menu.x.calc_pct(30)).text_size(g_menu.calc_min(40)).size_height_than_text(g_menu.y.calc(10)).size_width_than_text(g_menu.y.calc(10)).horizontal_func(roe::Left);
                show_choose_list(g_menu, g_menu.y.calc_pct(5), g_menu.y.calc(45), draw_overviews_s.size(), button_so, draw_overviews_s);

                break;
            }

            default:
                break;
            }
            
        window.EndDrawing();
    }

    g_sclass.save_dir(data_dir);
    settings.SaveSettingToFile(current_dir + "Data/Settings");

    return 0;
}

int show_choose_list(coord_calc_vec2& menu, float start_y, float spacing, size_t page_size, button btn_template, std::vector<std::string> content)
{
    static size_t current_page = 0;
    static std::vector<std::string> last_content;

    // 如果内容改变，重置页码
    if (last_content != content)
    {
        current_page = 0;
        last_content = content;
    }

    int result = -1;
    
    // 计算总页数
    if(page_size == 0) return -2;
    size_t total_pages = (content.size() + page_size - 1) / page_size;
    if (total_pages == 0) total_pages = 1;
    
    // 确保当前页在有效范围内
    if (current_page >= total_pages) current_page = total_pages - 1;
    
    // 计算当前页的起始和结束索引
    size_t start_index = current_page * page_size;
    size_t end_index = std::min(start_index + page_size, content.size());
    
    // 显示当前页的内容
    for (size_t i = start_index; i < end_index; ++i)
    {
        float y = start_y + (i - start_index + 1) * spacing - spacing;

        button btn = btn_template;
        btn.position_y(y)
           .text(content[i])
           .draw();

        if (btn.is_clicked())
        {
            result = i;
        }
    }
    
    // 翻页按钮
    float button_y = menu.y.calc(start_y + (end_index - start_index + 1) * spacing + 10);
    float button_width = menu.x.calc(60);
    float button_height = menu.y.calc(20);
    
    // 上一页按钮
    if (current_page > 0)
    {
        button prev_btn = btn_template;
        prev_btn.position({menu.x.calc(50), button_y})
               .size({button_width, button_height})
               .text("上一页")
               .text_size(std::min(menu.x.calc(15), menu.y.calc(15)))
               .draw();
        
        if (prev_btn.is_clicked())
        {
            current_page--;
        }
    }
    
    // 下一页按钮
    if (current_page < total_pages - 1)
    {
        button next_btn = btn_template;
        next_btn.position({menu.x.runtime - menu.x.calc(50), button_y})
               .size({button_width, button_height})
               .text("下一页")
               .text_size(std::min(menu.x.calc(15), menu.y.calc(15)))
               .draw();
        
        if (next_btn.is_clicked())
        {
            current_page++;
        }
    }
    
    return result;
}

raylib::Color get_color(int num)
{
    return colors[std::abs(num) % colors.size()];
}

raylib::Color get_color(size_t num)
{
    return colors[num % colors.size()];
}

std::string format_time_chinese(const std::chrono::system_clock::time_point& tp)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm local_tm = *std::localtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y年%m月%d日 %H:%M:%S");
    return oss.str();
}