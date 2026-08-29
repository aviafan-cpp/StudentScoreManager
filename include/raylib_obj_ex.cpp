#include <vector>
#include <string>
#include <filesystem>
#include <utility>
#include <unordered_map>
#include "raylib-cpp.hpp"
#include "raylib_obj_ex.hpp"

namespace raylib_obj_ex
{
    namespace str_ex
    {
        // ustring
        void ustring::from_str(const std::string& str)
        {
            this->codepoints.clear();
            
            size_t index = 0;
            int bytes_processed = 0;
            const char* cstr = str.data();
            size_t len = str.length();

            while(index < len)
            {
                int codepoint = GetCodepointNext(cstr + index, &bytes_processed);
                if(bytes_processed <= 0)
                    throw("\"" + str + "\"[" + std::to_string(index) + "].bytes_processed <= 0");

                this->codepoints.emplace_back(codepoint);
                index += static_cast<size_t>(bytes_processed);
            }
        }

        std::string ustring::to_str() const
        {
            std::string result;
            int utf8Size = 0; // 用于接收字节长度

            for (int codepoint : this->codepoints) {
                const char* utf8Bytes = CodepointToUTF8(codepoint, &utf8Size);

                // 检查是否转换成功 (utf8Size > 0)
                if (utf8Size > 0) {
                    // 3. 将指定长度的字节追加到result中
                    result.append(utf8Bytes, utf8Size);
                }
                else
                    throw("utf8Size <= 0");
                // 注意：不需要手动释放 utf8Bytes 指向的内存
            }

            return result;
        }

        // 构造函数实现
        ustring::ustring(std::vector<int> other_code_point) 
        : codepoints(std::move(other_code_point)) {}

        ustring::ustring(const std::string& str) {  // 注意改为 const 引用
            from_str(str);
        }

        // 运算符重载实现
        ustring ustring::operator+(const ustring& other) const {
            ustring result = *this;
            result += other;
            return result;
        }

        ustring& ustring::operator+=(const ustring& other) {
            codepoints.insert(codepoints.end(), other.codepoints.begin(), other.codepoints.end());
            return *this;
        }

        ustring ustring::operator+(const std::string other) const {
            ustring result = *this;
            result += other;
            return result;
        }

        ustring& ustring::operator+=(const std::string other) {
            ustring temp;
            temp.from_str(other);  // 直接用 from_str
            codepoints.insert(codepoints.end(), temp.codepoints.begin(), temp.codepoints.end());
            return *this;
        }
    }

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
    
    namespace input_ns
    {
        // input
        input::input(str_ex::ustring content, size_t index)
        {
            this->content = content;
            if(is_index_correct(index)) this->index = index;
            else this->index = content.codepoints.size();
        }

        input::input(std::string content, size_t index)
        {
            this->content.from_str(content);
            if(is_index_correct(index)) this->index = index;
            else this->index = this->content.codepoints.size();
        }

        bool input::is_index_correct() const
        {
            if(content.codepoints.size() == 0) return index == 0;
            return index <= content.codepoints.size();
        }

        bool input::is_index_correct(size_t index) const
        {
            if(content.codepoints.size() == 0) return index == 0;
            return index <= content.codepoints.size();
        }

        void input::let_index_correct()
        {
            if(content.codepoints.size() == 0) index = 0;
            if(index > content.codepoints.size()) index = content.codepoints.size();
        }

        void input::update()
        {
            if(!is_index_correct()) let_index_correct();

            if(IsKeyPressed(KEY_BACKSPACE) && index > 0) content.codepoints.erase(content.codepoints.begin() + index - 1), --index;
            else if(IsKeyPressed(KEY_DELETE) && index < content.codepoints.size()) content.codepoints.erase(content.codepoints.begin() + index);
            else if(IsKeyPressed(KEY_LEFT) && is_index_correct(index - 1)) --index;
            else if(IsKeyPressed(KEY_RIGHT) && is_index_correct(index + 1)) ++index;
            else if((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) && IsKeyPressed(KEY_V))
            {
                const char *text = GetClipboardText();
                if(text == nullptr) return;
                str_ex::ustring utext = std::string(text);
                content.codepoints.insert(content.codepoints.begin() + index, utext.codepoints.begin(), utext.codepoints.end());
                index += utext.codepoints.size();
            }
            else
            {
                str_ex::ustring utext;
                int ch = 0;
                do
                {
                    ch = GetCharPressed();
                    if(ch > 0) utext.codepoints.emplace_back(ch);
                } while (ch > 0);
                
                content.codepoints.insert(content.codepoints.begin() + index, utext.codepoints.begin(), utext.codepoints.end());
                index += utext.codepoints.size();
            }
        }

        // input_field
        input_field::input_field(
            str_ex::ustring content,
            size_t index,
            text_ns::text draw_title,
            text_ns::text draw_content,
            button_ns::button draw_cancle,
            button_ns::button draw_ok,
            raylib::Color index_color
        ){
            input(content, index);
            this->draw_title = draw_title;
            this->draw_content = draw_content;
            this->draw_cancle = draw_cancle;
            this->draw_ok = draw_ok;
            this->index_color = index_color;
        }

        input_field::input_field(
            std::string content,
            size_t index,
            text_ns::text draw_title,
            text_ns::text draw_content,
            button_ns::button draw_cancle,
            button_ns::button draw_ok,
            raylib::Color index_color
        ){
            input(content, index);
            this->draw_title = draw_title;
            this->draw_content = draw_content;
            this->draw_cancle = draw_cancle;
            this->draw_ok = draw_ok;
            this->index_color = index_color;
        }

        void input_field::draw()
        {
            draw_title.draw();
            draw_content.draw();
            draw_cancle.draw();
            draw_ok.draw();

            // ---------- 光标绘制 ----------
            static float timer = 0;
            timer += GetFrameTime();
            if (timer > 0.5f) timer = 0;
            if (timer < 0.25f)  // 显示半秒
            {
                // 1. 截取光标前的文本（codepoint 安全）
                std::vector<int> before_codepoints(
                    content.codepoints.begin(),
                    content.codepoints.begin() + index
                );
                str_ex::ustring before_str;
                before_str.codepoints = before_codepoints;
                std::string before = before_str.to_str();

                // 2. 测量前缀宽度
                Vector2 prefix_size = MeasureTextEx(
                    draw_content.font_d,
                    before.c_str(),
                    draw_content.font_size_d,
                    draw_content.spacing_d
                );

                // 3. 计算文本实际绘制起点（与 text::draw() 逻辑一致）
                Vector2 full_text_size = MeasureTextEx(
                    draw_content.font_d,
                    draw_content.st_d.c_str(),   // 整个文本
                    draw_content.font_size_d,
                    draw_content.spacing_d
                );

                Vector2 draw_pos = draw_content.position_d;  // 原始坐标

                switch (draw_content.current_horizontal_d)
                {
                case Left:
                    draw_pos.y -= full_text_size.y / 2;
                    draw_pos.y = std::max(draw_pos.y, 0.0f);
                    break;
                case Center:
                    draw_pos.x -= full_text_size.x / 2;
                    draw_pos.x = std::max(draw_pos.x, 0.0f);
                    draw_pos.y -= full_text_size.y / 2;
                    draw_pos.y = std::max(draw_pos.y, 0.0f);
                    break;
                case Right:
                    draw_pos.x -= full_text_size.x;
                    draw_pos.x = std::max(draw_pos.x, 0.0f);
                    draw_pos.y -= full_text_size.y / 2;
                    draw_pos.y = std::max(draw_pos.y, 0.0f);
                    break;
                }

                // 4. 光标位置 = 实际绘制起点 + 前缀宽度
                Vector2 cursor_pos = draw_pos;
                cursor_pos.x += prefix_size.x;

                // 5. 光标高度：使用整个文本的高度（或字体大小）
                float cursor_height = full_text_size.y;  // 或 draw_content.font_size_d

                // 6. 绘制光标（竖线）
                DrawLine(
                    cursor_pos.x, cursor_pos.y,
                    cursor_pos.x, cursor_pos.y + cursor_height,
                    index_color
                );
            }
        }

        void input_field::update_draw()
        {
            update();
            draw_content.st_d = content.to_str();
            draw();
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
