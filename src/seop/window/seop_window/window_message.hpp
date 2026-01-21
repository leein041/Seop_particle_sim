#pragma once
#include "seop_message/message.hpp"

namespace seop::window
{
class Window_resize_message : public msg::Message
{
  public:
    Window_resize_message(float width, float height);
    ~Window_resize_message() override = default;

    [[nodiscard]] auto width() const -> float;
    [[nodiscard]] auto height() const -> float;

  private:
    float width_;
    float height_;
};
} // namespace seop::window
