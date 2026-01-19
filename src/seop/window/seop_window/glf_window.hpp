#pragma once
#include "seop_math/math.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
namespace seop::window
{
    class Glf_window final
    {
    public:
        Glf_window();
        ~Glf_window() noexcept = default;

        auto open_window() -> bool;

        void init();
        auto shut_down() -> bool;
        void buffer_swap();

        [[nodiscard]] auto get_cursor_pos() -> math::Vec2;
        [[nodiscard]] auto get_cursor_pos_normalized() -> math::Vec2;
        [[nodiscard]] auto get_cursor_pos_ndc() -> math::Vec2;

        [[nodiscard]] auto glfw_window() const -> GLFWwindow *;
        [[nodiscard]] auto glfw_window() -> GLFWwindow *;
        [[nodiscard]] auto client_size() const -> const math::Vec2 &;
        [[nodiscard]] auto inv_client_size() const -> const math::Vec2 &;
        [[nodiscard]] auto half_client_size() const -> const math::Vec2 &;

        void set_window_size(const math::Vec2 &size);

        // temp

    private:
        GLFWwindow *window_{nullptr};

        math::Vec2 client_size_{1600.f, 900.f};
        math::Vec2 half_client_size_{client_size_ / 2};
        math::Vec2 inv_client_size_{1 / client_size_.x_, 1 / client_size_.y_};

    };

}
