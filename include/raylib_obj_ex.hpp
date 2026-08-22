#ifndef RAYLIB_OBJ_EX
#define RAYLIB_OBJ_EX

#include <string>
#include <vector>
#include <unordered_map>
#include "raylib-cpp.hpp"

namespace raylib_obj_ex
{
    enum horizontal {Left, Center, Right}; // 渲染对齐方向

    namespace coord
    {
        struct coord_calc_f
        {
            float runtime; // 实际运行的屏幕尺寸
            float design; // 设计时的基准屏幕尺寸

            float calc(float design_length); // 传入设计时位置
            float calc_pct(float design_pct); // 传入设计时百分比
        };
        
        struct coord_calc_vec2
        {
            coord_calc_f x;
            coord_calc_f y;

            coord_calc_vec2(float x_design, float y_design) : x{0, x_design}, y{0, y_design} {}

            float calc_min(float design_length); // 传入设计时位置并在x和y间筛选小值
            float calc_min_pct(float design_pct); // 传入设计时百分比并在x和y间筛选小值
        };
    }

    namespace text_ns
    {
        struct text
        {
            Font font_d; // 字体
            std::string st_d; // 文本
            raylib::Vector2 position_d; // 坐标
            float font_size_d; // 字体大小
            float spacing_d; // 字体间隔
            Color tint_d; // 字体颜色
            horizontal current_horizontal_d; // 渲染对齐方向

            // 添加构造函数
            text() 
            : font_d(GetFontDefault())  // 使用默认字体
            , position_d{0, 0}
            , font_size_d(20)
            , spacing_d(1)
            , tint_d(WHITE)
            , current_horizontal_d(Left)
            {}

            text(const text&) = default;

            // 链式设置函数
            text& font(Font _font);
            text& st(std::string _st);
            text& position(raylib::Vector2 _position);
            text& position_x(float _x);
            text& position_y(float _y);
            text& font_size(float _font_size);
            text& spacing(float _spacing);
            text& tint(Color _tint);
            text& horizontal_func(horizontal _current_horizontal);

            void draw();
        };
        
    }

    namespace button_ns
    {
        struct button
        {
            raylib::Vector2 position_d;
            raylib::Vector2 size_d;
            Color tint_d;
            Font font_d;
            std::string text_d;
            float text_size_d;
            float text_spacing_d;
            Color text_tint_d;
            bool is_pressed_d; // 按下
            horizontal current_horizontal_d; // 渲染对齐方向
            bool show_border_d; // 是否显示边界

            // 添加构造函数
            button()
            : position_d{0, 0}
            , size_d{100, 40}  // 默认按钮大小
            , tint_d(GRAY)
            , font_d(GetFontDefault())  // 使用默认字体
            , text_d("Button")
            , text_size_d(20)
            , text_spacing_d(1)
            , text_tint_d(BLACK)
            , is_pressed_d(false)
            , current_horizontal_d(Center)  // 按钮默认居中
            , show_border_d(true)  // 默认显示边界
            {}

            button(const button&) = default;

            // 链式设置函数
            button& position(raylib::Vector2 _position);
            button& position_x(float _x);
            button& position_y(float _y);
            button& size(raylib::Vector2 _size);
            button& size_width(float _width);
            button& size_height(float _height);
            button& size_than_button(raylib::Vector2 _size);
            button& size_width_than_text(float _width);
            button& size_height_than_text(float _height);
            button& tint(Color _tint);
            button& font(Font _font);
            button& text(std::string _text);
            button& text_size(float _text_size);
            button& text_spacing(float _text_spacing);
            button& text_tint(Color _text_tint);
            button& pressed(bool _is_pressed);
            button& horizontal_func(horizontal _current_horizontal);
            button& show_border(bool _show_border);

            button& draw();
            bool is_clicked();
        };
    }

    namespace picture
    {
        struct texture2D_manager
        {
            std::string dir;
            std::unordered_map<std::string, raylib::Texture2D> texture2Ds;

            texture2D_manager(std::string dir);

            void load();
            void load(std::string name);

            raylib::Texture2D& find(std::string name);
        };
    }

    namespace font_ex
    {
        Font load_font_chinese(std::string fileName, int fontSize, bool chinese, bool chinese_ex_a, bool chinese_ex_b, bool chinese_symbol);
    }
}

#endif