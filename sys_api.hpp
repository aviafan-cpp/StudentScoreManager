#ifndef SYS_API
#define SYS_API

#include <string>

namespace sys_api
{
    void open_url(std::string url);
    void open_file(std::string path);
    void open_file(std::string program_path, std::string path);
    void open_program(std::string program_path);

    bool create_file(std::string path, bool send_error_to_user);

    enum mb_icon {info, warning, error, question};
    enum mb_button {ok, yes_no, ok_cencle, retry_cancel};
    bool message_box(std::string title = "", std::string content = "", mb_icon mbi = info, mb_button mbb = ok, bool topmost = true);

    void hide_taskbar(); // 隐藏任务栏
    void show_taskbar(); // 显示任务栏
    void enable_taskbar_auto_hide(); // 启用任务栏自动隐藏
    void disable_taskbar_auto_hide(); // 关闭任务栏自动隐藏
    bool is_taskbar_auto_hide_enabled(); // 任务栏是否自动隐藏
}

#endif