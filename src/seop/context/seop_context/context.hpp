#pragma once

// clang-format off

namespace seop{
namespace command   {class Command_list;}
namespace msg       {class Message_queue;}
namespace window    {class Glf_window;}
namespace graphic   {class Device;}
namespace imgui     {class Imgui_renderer;}
namespace scene     {class Scene;}
namespace input     {class Input;}
}

// clang-format on

namespace seop
{
class Context
{
  public:
    command::Command_list* command_list{nullptr};
    msg::Message_queue*    msg_queue{nullptr};
    window::Glf_window*    window{nullptr};
    graphic::Device*       device{nullptr};
    imgui::Imgui_renderer* imgui{nullptr};
    scene::Scene*          scene{nullptr};
    input::Input*          input{nullptr};
};
} // namespace seop
