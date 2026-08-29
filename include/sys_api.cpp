#include "sys_api.hpp"
#include <string>
#include <algorithm>

namespace sys_api
{
    #if defined(_WIN32) || defined(_WIN64)
        #include <windows.h>
        #include <shellapi.h>

        std::wstring stow(const std::string& str, UINT code_page = CP_UTF8) {
            // 1. 参数校验
            if (str.empty()) {
                return L"";
            }
        
            // 2. 计算转换后宽字符串所需的长度（包含结尾的'\0'）
            int requiredLen = MultiByteToWideChar(code_page, 0, str.c_str(), -1, nullptr, 0);
            if (requiredLen == 0) {
                // 转换失败，可根据需要抛出异常或返回空字符串
                return L"";
            }
        
            // 3. 为 std::wstring 分配所需空间
            std::wstring wstr(requiredLen, L'\0');
        
            // 4. 执行实际的编码转换
            int convertedLen = MultiByteToWideChar(code_page, 0, str.c_str(), -1, &wstr[0], requiredLen);
            if (convertedLen == 0) {
                return L"";
            }
        
            // 5. 移除末尾的 '\0' 字符
            wstr.pop_back(); 
            
            return wstr;
        }

        void open_url(std::string url)
        {
            std::wstring wurl = stow(url);
            ShellExecuteW(NULL, L"open", wurl.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }

        void open_file(std::string path)
        {
            std::wstring wpath = stow(path);
            std::replace(wpath.begin(), wpath.end(), L'/', L'\\');
            ShellExecuteW(NULL, L"open", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }

        void open_file(std::string program_path, std::string path)
        {
            std::wstring wprogram_path = stow(program_path);
            std::replace(wprogram_path.begin(), wprogram_path.end(), L'/', L'\\');
            std::wstring wpath = stow(path);
            std::replace(wpath.begin(), wpath.end(), L'/', L'\\');
            ShellExecuteW(NULL, L"open", wprogram_path.c_str(), wpath.c_str(), NULL, SW_SHOWNORMAL);
        }

        void open_program(std::string program_path)
        {
            std::wstring wprogram_path = stow(program_path);
            std::replace(wprogram_path.begin(), wprogram_path.end(), L'/', L'\\');
            ShellExecuteW(NULL, L"open", wprogram_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }

        bool create_file(std::string path, bool send_error_to_user)
        {
            std::wstring wpath = stow(path);
            std::replace(wpath.begin(), wpath.end(), L'/', L'\\');
            HANDLE hfile = CreateFileW(
                wpath.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );
            if(hfile == INVALID_HANDLE_VALUE)
            {
                if(send_error_to_user)
                {
                    std::wstring error_information = L"创建文件 " + wpath + L" 失败，原因：";
                    switch (GetLastError()){
                    case ERROR_PATH_NOT_FOUND:      // 3
                        error_information += L"目录不存在";
                        break;
                    case ERROR_ACCESS_DENIED:       // 5
                        error_information += L"权限不足";
                        break;
                    case ERROR_SHARING_VIOLATION:   // 32
                        error_information += L"被其他程序占用";
                        break;
                    case ERROR_NOT_SUPPORTED:       // 50
                        error_information += L"尝试在不支持文件系统的设备上操作";
                        break;
                    case ERROR_DISK_FULL:           // 112
                        error_information += L"磁盘空间不足";
                        break;
                    case ERROR_INVALID_NAME:        // 123
                        error_information += L"文件名包含非法字符";
                        break;
                    case ERROR_FILENAME_EXCED_RANGE:// 206
                        error_information += L"文件路径超过 MAX_PATH";
                        break;
                    default:
                        error_information += L"未知原因 (错误码: " + std::to_wstring(GetLastError()) + L")";
                        break;
                    }
            
                    MessageBoxW(NULL, error_information.c_str(), NULL, MB_ICONERROR);
                }

                return false;
            }
            CloseHandle(hfile);
            return true;
        }

        bool message_box(std::string title, std::string content, mb_icon mbi ,mb_button mbb, bool topmost)
        {
            UINT utype = 0;

            switch (mbi)
            {
            case info:
                utype = MB_ICONINFORMATION;
                break;
            case warning:
                utype = MB_ICONWARNING;
                break;
            case error:
                utype = MB_ICONERROR;
                break;
            case question:
                utype = MB_ICONQUESTION;
                break;
            default:
                break;
            }
            switch (mbb)
            {
            case ok:
                utype |= MB_OK;
                break;
            case yes_no:
                utype |= MB_YESNO;
                break;
            case ok_cencle:
                utype |= MB_OKCANCEL;
                break;
            case retry_cancel:
                utype |= MB_RETRYCANCEL;
                break;
            default:
                break;
            }

            if(topmost) utype |= MB_SYSTEMMODAL;
            
            int result = MessageBoxW(
                NULL,
                stow(content).c_str(),
                stow(title).c_str(),
                utype
            );
            if(result == IDOK || result == IDYES || result == IDRETRY) return true;
            else return false;
        }

        void hide_taskbar()
        {
            HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
            if (hTaskbar != NULL)
            {
                ShowWindow(hTaskbar, SW_HIDE);
            }
        }

        void show_taskbar()
        {
            HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
            if (hTaskbar != NULL)
            {
                ShowWindow(hTaskbar, SW_SHOW);
            }
        }

        void enable_taskbar_auto_hide()
        {
            HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
            if (hTaskbar == NULL) return;

            APPBARDATA abd = { 0 };
            abd.cbSize = sizeof(APPBARDATA);
            abd.hWnd = hTaskbar;
            abd.lParam = ABS_AUTOHIDE;  // 值为 1

            SHAppBarMessage(ABM_SETSTATE, &abd);
        }

        void disable_taskbar_auto_hide()
        {
            HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
            if (hTaskbar == NULL) return;

            APPBARDATA abd = { 0 };
            abd.cbSize = sizeof(APPBARDATA);
            abd.hWnd = hTaskbar;
            abd.lParam = 0;  // 取消自动隐藏

            SHAppBarMessage(ABM_SETSTATE, &abd);
        }

        bool is_taskbar_auto_hide_enabled()
        {
            APPBARDATA abd = {0};
            abd.cbSize = sizeof(APPBARDATA);

            // 发送 ABM_GETSTATE 消息查询任务栏状态
            UINT uState = (UINT)SHAppBarMessage(ABM_GETSTATE, &abd);

            // 如果返回值包含 ABS_AUTOHIDE 标志，则表示自动隐藏已开启
            return (uState & ABS_AUTOHIDE) != 0;
        }

    #endif
}