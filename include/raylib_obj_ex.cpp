#include <string>
#include <filesystem>
#include <utility>
#include <unordered_map>
#include "raylib-cpp.hpp"
#include "raylib_obj_ex.hpp"

namespace raylib_obj_ex
{
    namespace coord
    {
        // coord_calc_f
        float coord_calc_f::calc(float design_length)
        {return design_length * runtime / design;}

        float coord_calc_f::calc_pct(float design_pct)
        {return runtime * design_pct / 100;}

        // coord_calc_vec2
        float coord_calc_vec2::calc_min(float design_length)
        {return std::min(x.calc(design_length), y.calc(design_length));}

        float coord_calc_vec2::calc_min_pct(float design_pct)
        {return std::min(x.calc_pct(design_pct), y.calc_pct(design_pct));}
    }

    namespace text_ns
    {
        // text
        text& text::font(Font _font) { 
            font_d = std::move(_font); 
            return *this; 
        }
        text& text::st(std::string _st) { st_d = _st; return *this; }
        text& text::position(raylib::Vector2 _position) { position_d = _position; return *this; }
        text& text::position_x(float _x) { position_d.x = _x; return *this; }
        text& text::position_y(float _y) { position_d.y = _y; return *this; }
        text& text::font_size(float _font_size) { font_size_d = _font_size; return *this; }
        text& text::spacing(float _spacing) { spacing_d = _spacing; return *this; }
        text& text::tint(Color _tint) { tint_d = _tint; return *this; }
        text& text::horizontal_func(horizontal _current_horizontal) { current_horizontal_d = _current_horizontal; return *this; }

        void text::draw()
        {
            Vector2 text_size = MeasureTextEx(font_d, st_d.c_str(), font_size_d, spacing_d);
            Vector2 draw_pos = position_d;  // 使用临时变量
            
            switch (current_horizontal_d)
            {
            case Left:
                draw_pos.y -= text_size.y / 2;
                draw_pos.y = std::max(draw_pos.y, 0.0f);
                break;
            case Center:
                draw_pos.x -= text_size.x / 2;
                draw_pos.x = std::max(draw_pos.x, 0.0f);
                draw_pos.y -= text_size.y / 2;
                draw_pos.y = std::max(draw_pos.y, 0.0f);
                break;
            case Right:
                draw_pos.x -= text_size.x;
                draw_pos.x = std::max(draw_pos.x, 0.0f);
                draw_pos.y -= text_size.y / 2;
                draw_pos.y = std::max(draw_pos.y, 0.0f);
                break;
            }
            
            DrawTextEx(font_d, st_d.c_str(), draw_pos, font_size_d, spacing_d, tint_d);
        }
    }

    namespace button_ns
    {
        //button
        button& button::position(raylib::Vector2 _position) { position_d = _position; return *this; }
        button& button::position_x(float _x) { position_d.x = _x; return *this; }
        button& button::position_y(float _y) { position_d.y = _y; return *this; }
        button& button::size(raylib::Vector2 _size) { size_d = _size; return *this; }
        button& button::size_width(float _width) { size_d.x = _width; return *this; }
        button& button::size_height(float _height) { size_d.y = _height; return *this; }
        button& button::size_than_button(raylib::Vector2 _size) { size_d += _size * 2; return *this; }
        button& button::size_width_than_text(float _width) { size_d.x += _width * 2; return *this; }
        button& button::size_height_than_text(float _height) {size_d.y += _height * 2; return *this; }
        button& button::tint(Color _tint) { tint_d = _tint; return *this; }
        button& button::font(Font _font) { 
            font_d = std::move(_font); 
            return *this; 
        }
        button& button::text(std::string _text) { text_d = _text; return *this; }
        button& button::text_size(float _text_size) { text_size_d = _text_size; return *this; }
        button& button::text_spacing(float _text_spacing) { text_spacing_d = _text_spacing; return *this; }
        button& button::text_tint(Color _text_tint) { text_tint_d = _text_tint; return *this; }
        button& button::pressed(bool _is_pressed) { is_pressed_d = _is_pressed; return *this; }
        button& button::horizontal_func(horizontal _current_horizontal) { this->current_horizontal_d = _current_horizontal; return *this; }
        button& button::show_border(bool _show_border) { this->show_border_d = _show_border; return *this; }

        button& button::draw()
        {
            is_pressed_d = false; // 在绘制时重置上一次点击状态

            Vector2 text_size = MeasureTextEx(font_d, text_d.c_str(), text_size_d, text_spacing_d);
            Vector2 draw_pos = position_d;
            
            // 计算按钮实际绘制位置（根据对齐方式偏移）
            Vector2 button_draw_pos = position_d;
            
            switch (current_horizontal_d)
            {
            case Left:
                // 按钮从position开始向右延伸
                break;
            case Center:
                button_draw_pos.x -= size_d.x / 2;
                draw_pos.x -= text_size.x / 2;
                break;
            case Right:
                button_draw_pos.x -= size_d.x;
                draw_pos.x -= text_size.x;
                break;
            }
            
            // 垂直方向：文本在按钮中垂直居中
            draw_pos.y = button_draw_pos.y + (size_d.y - text_size.y) / 2;
            
            // 边界限制
            button_draw_pos.x = std::max(button_draw_pos.x, 0.0f);
            button_draw_pos.y = std::max(button_draw_pos.y, 0.0f);
            draw_pos.x = std::max(draw_pos.x, 0.0f);
            draw_pos.y = std::max(draw_pos.y, 0.0f);
            
            // 绘制按钮背景（使用偏移后的位置）
            DrawRectangleV(button_draw_pos, size_d, tint_d);
            // 根据 show_border_d 决定是否绘制边界
            if(show_border_d) DrawRectangleLinesEx(Rectangle{button_draw_pos.x, button_draw_pos.y, size_d.x, size_d.y}, 2, DARKGRAY);
            
            // 绘制文本
            DrawTextEx(font_d, text_d.c_str(), draw_pos, text_size_d, text_spacing_d, text_tint_d);
            
            return *this;
        }

        bool button::is_clicked()
        {
            // 计算按钮实际位置（与draw中使用相同的偏移逻辑）
            Vector2 button_pos = position_d;
            
            switch (current_horizontal_d)
            {
            case Left:
                break;
            case Center:
                button_pos.x -= size_d.x / 2;
                break;
            case Right:
                button_pos.x -= size_d.x;
                break;
            }
            
            Vector2 mouse_pos = GetMousePosition();
            bool hovered = (mouse_pos.x >= button_pos.x && 
                            mouse_pos.x <= button_pos.x + size_d.x &&
                            mouse_pos.y >= button_pos.y && 
                            mouse_pos.y <= button_pos.y + size_d.y);

            if(hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) is_pressed_d = true;
            return (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
        }
    }

    namespace picture
    {
        // texture2D_manager
        texture2D_manager::texture2D_manager(std::string dir)
        {
            this->dir = dir;
        }

        void texture2D_manager::load()
        {
            for(const auto& entry : std::filesystem::recursive_directory_iterator(dir))
                this->texture2Ds.emplace(entry.path().stem().string(), raylib::Image(entry.path().string()).LoadTexture());
        }

        void texture2D_manager::load(std::string name)
        {
            for(const auto& entry : std::filesystem::recursive_directory_iterator(dir))
            {
                if(entry.is_regular_file() && entry.path().stem().string() == name)
                {
                    this->texture2Ds.emplace(name, raylib::Image(entry.path().string()).LoadTexture());
                    return;
                }
            }
            // 找不到
            throw std::runtime_error("Texture not found: " + name);
        }

        raylib::Texture2D& texture2D_manager::find(std::string name)
        {
            if(this->texture2Ds.find(name) != this->texture2Ds.end()) return this->texture2Ds[name];
            // 未读取
            for(const auto& entry : std::filesystem::recursive_directory_iterator(dir))
            {
                if(entry.is_regular_file() && entry.path().stem().string() == name)
                {
                    this->texture2Ds.emplace(name, raylib::Image(entry.path().string()).LoadTexture());
                    return texture2Ds[name];
                }
            }
            // 找不到
            throw std::runtime_error("Texture not found: " + name);
        }
    }
    

    namespace font_ex
    {
        Font load_font_chinese(std::string fileName, int fontSize, bool chinese, bool chinese_ex_a, bool chinese_ex_b, bool chinese_symbol)
        {
            // 定义要加载的 Unicode 范围（例如基本汉字 0x4E00-0x9FFF）
            std::vector<int> fontChars;
            for (int i = 32; i < 127; i++) fontChars.push_back(i); // 添加ASCII字符
            for (int i = 0x0021; i <= 0x002F; i++) fontChars.push_back(i); // 常用标点：!"#$%&'()*+,-./
            for (int i = 0x003A; i <= 0x0040; i++) fontChars.push_back(i); // 常用标点：:;<=>?@
            for (int i = 0x005B; i <= 0x0060; i++) fontChars.push_back(i); // 常用标点：[\]^_`
            for (int i = 0x007B; i <= 0x007E; i++) fontChars.push_back(i); // 常用标点：{|}~
            for (int i = 0x2000; i <= 0x206F; i++) fontChars.push_back(i); // 常用标点：通用标点符号（空格、引号、连字符、省略号等）
            for (int i = 0x2070; i <= 0x209F; i++) fontChars.push_back(i); // 上标与下标
            for (int i = 0x00B9; i <= 0x00B9; i++) fontChars.push_back(i); // ¹
            for (int i = 0x00B2; i <= 0x00B3; i++) fontChars.push_back(i); // ²³

            if(chinese) for (int i = 0x4E00; i <= 0x9FFF; i++) fontChars.push_back(i); // 添加汉字
            if(chinese_ex_a) for (int i = 0x3400; i <= 0x4DBF; i++) fontChars.push_back(i); // 扩展A区
            if(chinese_ex_b) for (int i = 0x20000; i <= 0x2A6D6; i++) fontChars.push_back(i); // 扩展B区
            if(chinese_symbol)
            {
                for (int i = 0xFF00; i <= 0xFFEF; i++) fontChars.push_back(i); // 全角标点符号
                for (int i = 0x3000; i <= 0x303F; i++) fontChars.push_back(i); // 中文标点
            }
            fontChars.push_back(0); // 必须包含 0（NULL 终止符）

            // 加载字体
            return LoadFontEx(fileName.c_str(), fontSize, fontChars.data(), fontChars.size());
        }
    }
}