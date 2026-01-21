#include "glf_window.hpp"
#include "seop_math/math.hpp"

#include <iostream>

namespace seop::window
{
Glf_window::Glf_window()
{
}

void Glf_window::init()
{
    open_window();
    glfwSwapInterval(1); // 1 - on vsync
}

auto Glf_window::shut_down() -> bool
{
    return glfwWindowShouldClose(window_);
}

void Glf_window::buffer_swap()
{
    glfwSwapBuffers(window_);
    glfwPollEvents(); // ?
}

/* --------------------------
|(0,0)                       |
|                            |
|                            |
|                  (max, max)|
----------------------------*/
auto Glf_window::get_cursor_pos() -> math::Vec2
{
    double x, y;
    glfwGetCursorPos(window_, &x, &y);
    return math::Vec2{static_cast<float>(x), static_cast<float>(y)};
}

/* --------------------------
|             |              |
|_____________|_(0,0)________|
|             |              |
|             |              |
----------------------------*/
auto Glf_window::get_cursor_pos_normalized() -> math::Vec2
{
    math::Vec2 pos = get_cursor_pos();

    float      nx = static_cast<float>(pos.x_) - half_client_size_.x_;
    float      ny = -static_cast<float>(pos.y_) + half_client_size_.y_;
    if (nx < -half_client_size_.x_ || nx > half_client_size_.x_ || ny < -half_client_size_.y_ ||
        ny > half_client_size_.y_) {
        return math::Vec2(0.0f, 0.0f);
    } else {
        return math::Vec2(nx, ny);
    }
}

auto Glf_window::get_cursor_pos_ndc() -> math::Vec2
{
    // ndc 좌표 변환
    math::Vec2 pos = get_cursor_pos();

    float      x_ndc = (2.0f * pos.x_) / client_size_.x_ - 1.0f;
    float      y_ndc = 1.0f - (2.0f * pos.y_) / client_size_.y_;

    return math::Vec2(x_ndc, y_ndc);
}

void Glf_window::set_window_size(const math::Vec2& size)
{
    glfwSetWindowSize(window_, static_cast<int>(size.x_), static_cast<int>(size.y_));
    client_size_ = size;
    half_client_size_ = size / 2.0f;
    inv_client_size_ = math::Vec2{1.0f / size.x_, 1.0f / size.y_};
}

auto Glf_window::glfw_window() const -> GLFWwindow*
{
    return window_;
}

auto Glf_window::glfw_window() -> GLFWwindow*
{
    return window_;
}

auto Glf_window::client_size() const -> const math::Vec2&
{
    return client_size_;
}

auto Glf_window::inv_client_size() const -> const math::Vec2&
{
    return inv_client_size_;
}

auto Glf_window::half_client_size() const -> const math::Vec2&
{
    return half_client_size_;
}

auto Glf_window::open_window() -> bool
{
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    window_ = (glfwCreateWindow(static_cast<int>(client_size_.x_), static_cast<int>(client_size_.y_),
                                "Seop Particle Sim", NULL, NULL));
    if (!window_) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    return true;
}
} // namespace seop::window