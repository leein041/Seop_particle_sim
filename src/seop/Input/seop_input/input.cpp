#include "input.hpp"

#include "seop_context/context.hpp"
#include "seop_window/glf_window.hpp"
#include <GLFW/glfw3.h>

#include <algorithm>

namespace seop::input {
Input::Input() {}

void Input::update(Context& ctx) {
    update_cursor_pos(ctx);
    update_mouse(ctx);
    update_keys(ctx);
}

void Input::reset() {
    Input_data new_data;
    data_ = new_data;
}

auto Input::data() const -> const Input_data& { return data_; }

auto Input::data() -> Input_data& { return data_; }

void Input::set_ray_point(const math::Vec3& ray_point) { data_.ray_point = ray_point; }

auto Input::get_key_down(Key_code code) -> bool { return keys_[static_cast<int>(code)].state == Key_state::Down; }

auto Input::get_key_up(Key_code code) -> bool { return keys_[static_cast<int>(code)].state == Key_state::Up; }

auto Input::get_key(Key_code code) -> bool { return keys_[static_cast<int>(code)].state == Key_state::Pressed; }

auto Input::get_delta_cursor_pos() const -> math::Vec2 { return data_.cur_cursor_pos - data_.pre_cursor_pos; }

void Input::init() { create_key(); }

void Input::update_cursor_pos(Context& ctx) {
    data_.pre_cursor_pos = data_.cur_cursor_pos;
    data_.cur_cursor_pos = ctx.window->get_cursor_pos_normalized();
}

void Input::update_mouse(Context& ctx) {
    update_mouse_button(ctx, GLFW_MOUSE_BUTTON_LEFT, Key_code::LButton);
    update_mouse_button(ctx, GLFW_MOUSE_BUTTON_RIGHT, Key_code::RButton);
    update_mouse_button(ctx, GLFW_MOUSE_BUTTON_MIDDLE, Key_code::MButton);
}

void Input::update_mouse_button(Context& ctx, int glfw_button, Key_code code) {
    if (glfwGetWindowAttrib(ctx.window->glfw_window(), GLFW_FOCUSED)) {
        if (glfwGetMouseButton(ctx.window->glfw_window(), glfw_button)) {
            update_key_down(keys_[static_cast<int>(code)]);
        } else {
            update_key_up(keys_[static_cast<int>(code)]);
        }
    } else {
        clear_key();
    }
}

void Input::update_keys(Context& ctx) {
    // Note : keyboard start at 0x08
    for (int vk = 0x08; vk < 0xFF; vk++) {
        update_key(keys_[vk], ctx);
    }
    std::for_each(keys_.begin(), keys_.end(), [&](Key& key) -> void {});
}

void Input::update_key(Key& key, Context& ctx) {
    if (glfwGetWindowAttrib(ctx.window->glfw_window(), GLFW_FOCUSED)) {
        if (glfwGetKey(ctx.window->glfw_window(), static_cast<int>(key.key_code))) {
            update_key_down(key);
        } else {
            update_key_up(key);
        }
    } else {
        clear_key();
    }
}

void Input::update_key_down(Key& key) {
    if (key.pressed == true) {
        key.state = Key_state::Pressed; // 키를 누르는 중
    } else {
        key.state = Key_state::Down; // 키를 누름
    }
    key.pressed = true;
}

void Input::update_key_up(Key& key) {
    if (key.pressed == true) {
        key.state = Key_state::Up; // 키를 누르지 않음
    } else {
        key.state = Key_state::None; // 키를 안눌렀음
    }
    key.pressed = false;
}

void Input::create_key() {
    for (int vk = 0x00; vk <= 0xFF; vk++) {
        Key key = {};
        key.pressed = false;
        key.state = Key_state::None;
        key.key_code = static_cast<Key_code>(vk);
        key.vk_key_code = vk;

        keys_.push_back(key);
    }
}

void Input::clear_key() {
    for (Key& key : keys_) {
        if (key.state == Key_state::Down || key.state == Key_state::Pressed) {
            key.state = Key_state::Up;
        } else if (key.state == Key_state::Up) {
            key.state = Key_state::None;
        }
        key.pressed = false;
    }
}
} // namespace seop::input