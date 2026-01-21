#include "window_message.hpp"

namespace seop::window
{
Window_resize_message::Window_resize_message(float width, float height) : width_(width), height_(height)
{
}

auto Window_resize_message::width() const -> float
{
    return width_;
}

auto Window_resize_message::height() const -> float
{
    return height_;
}
} // namespace seop::window