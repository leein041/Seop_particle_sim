#pragma once
#include "keys.hpp"
#include "seop_math/math.hpp"
#include <vector>

namespace seop {
class Context;
}

namespace seop::input {
class Key {
  public:
    Key_code  key_code{Key_code::End};
    Key_state state{Key_state::Released};
    bool      pressed{false};
    int       vk_key_code{0};
};

class Input_data {
  public:
    math::Vec2 pre_cursor_pos{0.0f, 0.0f};
    math::Vec2 cur_cursor_pos{0.0f, 0.0f};
    math::Vec3 ray_point{0.0f, 0.0f, 0.0f};
};

class Input {
  public:
    Input();
    ~Input() = default;

    void               init();
    void               update(Context& ctx);
    void               reset();
    [[nodiscard]] auto get_delta_cursor_pos() const -> math::Vec2;
    [[nodiscard]] auto get_key_down(Key_code code) -> bool;
    [[nodiscard]] auto get_key_up(Key_code code) -> bool;
    [[nodiscard]] auto get_key(Key_code code) -> bool;
    [[nodiscard]] auto data() const -> const Input_data&;
    [[nodiscard]] auto data() -> Input_data&;
    void               set_ray_point(const math::Vec3& ray_point);

    float mouse_sensitivity = 0.1f;

  private:
    void update_cursor_pos(Context& ctx);
    void update_mouse(Context& ctx);
    void update_mouse_button(Context& ctx, int glfw_button, Key_code code);
    void update_keys(Context& ctx);
    void update_key(Key& key, Context& ctx);
    void update_key_down(Key& key);
    void update_key_up(Key& key);
    void create_key();
    void clear_key();

  private:
    std::vector<Key> keys_;
    Input_data       data_;
};
} // namespace seop::input