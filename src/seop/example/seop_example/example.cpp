#include "example.hpp"

#include "seop_command/command.hpp"
#include "seop_context/context.hpp"
#include "seop_editor/editor.hpp"
#include "seop_entity/attractor.hpp"
#include "seop_entity/particle.hpp"
#include "seop_graphic/device.hpp"
#include "seop_imgui/imgui_core.hpp"
#include "seop_input/input.hpp"
#include "seop_math/math.hpp"
#include "seop_message/message.hpp"
#include "seop_scene/camera.hpp"
#include "seop_scene/scene.hpp"
#include "seop_window/glf_window.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace example
{   
using namespace seop;

class Example
{
  public:
    Example() : editor_(scene_.camera())
    {
    }

    ~Example()
    {
    }

    void run()
    {
        init();
        while (!window_.shut_down()) {
            begin_frame();
            tick();
            end_frame();
        }
        imgui_core_.shut_down();
        glfwTerminate();
    }

    void tick()
    {
        const auto tick_end_time = std::chrono::steady_clock::now();
        const auto new_time = std::chrono::steady_clock::now();
        const auto duration = new_time - m_current_time;
        double     frame_time = std::chrono::duration<double, std::ratio<1>>(duration).count();

        if (frame_time > 0.25) {
            frame_time = 0.25;
        }

        m_current_time = new_time;
        m_time_accumulator += frame_time;

        const double dt = 1.0 * device_.data().inv_frame_rate;
        ctx_.f_dt = static_cast<float>(dt);

        // update
        update();

        // compute
        while (m_time_accumulator >= dt) {
            ctx_.d_time += dt;
            device_.compute(ctx_);
            m_time_accumulator -= dt;
        }
        // render
        device_.render(ctx_);
        imgui_core_.render(ctx_);
    }

    void register_command()
    {
        scene_.register_commnad(ctx_);
    }

    void init()
    {
        ctx_.command_list = &command_list_;
        ctx_.msg_queue = &msg_queue_;
        ctx_.window = &window_;
        ctx_.scene = &scene_;
        ctx_.device = &device_;
        ctx_.imgui = &imgui_core_;
        ctx_.input = &input_;
        ctx_.editor = &editor_;

        window_.init();
        scene_.init();
        device_.init(ctx_);
        imgui_core_.init(ctx_);
        input_.init();

        register_command();
    }

    void update()
    {
        device_.update(ctx_);
        imgui_core_.update();
        input_.update(ctx_);
        scene_.update(ctx_);
        if (!imgui_core_.state().is_ui_hovered)
            editor_.update(ctx_);
    }

    void begin_frame()
    {
        device_.clear_frame();
    }

    void end_frame()
    {
        msg_queue_.Process();
        imgui_core_.end_frame();
        scene_.end_frame();
        window_.buffer_swap();
    }

  private:
    Context                               ctx_;
    msg::Message_queue                    msg_queue_;
    command::Command_list                 command_list_;
    window::Glf_window                    window_;
    graphic::Device                       device_;
    imgui::Imgui_core                     imgui_core_;
    scene::Scene                          scene_;
    input::Input                          input_;
    editor::Editor                        editor_;

    std::chrono::steady_clock::time_point m_current_time;
    double                                m_time_accumulator{0.0};
};

void example_run()
{

    Example example{};
    example.run();
}
} // namespace example