#include "example.hpp"

#include "seop_command/command.hpp"
#include "seop_context/context.hpp"
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
    Example() : device_{}
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
        device_.shut_down();
        imgui_core_.shut_down();
    }

    void tick()
    {
        const auto tick_end_time = std::chrono::steady_clock::now();

        // Update fixed steps
        const auto new_time = std::chrono::steady_clock::now();
        const auto duration = new_time - m_current_time;
        double     frame_time = std::chrono::duration<double, std::ratio<1>>(duration).count();

        if (frame_time > 0.25) {
            frame_time = 0.25;
        }

        m_current_time = new_time;
        m_time_accumulator += frame_time;

        float        inv_frame_rate = device_.data().inv_frame_rate;
        const double dt = 1.0 * inv_frame_rate;
        float        dt_f = static_cast<float>(dt);

        // update
        update(dt_f);

        // compute
        while (m_time_accumulator >= dt) {
            device_.compute(dt_f, context_);
            m_time_accumulator -= dt;
            m_time += dt;
        }
        // render
        device_.render(context_);
        imgui_core_.render(context_);

        // reset
        if (imgui_core_.state().is_reset) {
            reset();
        }
    }

    void register_command()
    {
        scene_.register_commnad(context_);
    }

    void init()
    {
        context_.command_list = &command_list_;
        context_.msg_queue = &msg_queue_;
        context_.window = &window_;
        context_.scene = &scene_;
        context_.device = &device_;
        context_.imgui = &imgui_core_;
        context_.input = &input_;

        window_.init();
        scene_.init();
        device_.init(context_);
        imgui_core_.init(context_);
        input_.init();

        reset();

        register_command();
    }

    void update(float dt_f)
    {
        imgui_core_.update();
        input_.update(context_);
        if (!imgui_core_.state().is_ui_hovered)
            scene_.update_camera(dt_f, context_);
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

    void reset()
    {
        input_.reset();
        scene_.reset();

        device_.bind_shader_buffer(device_.shader_buffer_data().particle_sb, 0,
                                   sizeof(entity::Particle) * scene_.data().particle_properties.count,
                                   scene_.data().entities.particles.data());

        device_.bind_shader_buffer(device_.shader_buffer_data().grid_sb, 2, sizeof(graphic::Screen_quad),
                                   &device_.grid()._0);
    }

  private:
    Context                               context_;
    msg::Message_queue                    msg_queue_;
    command::Command_list                 command_list_;
    window::Glf_window                    window_;
    graphic::Device                       device_;
    imgui::Imgui_core                     imgui_core_;
    scene::Scene                          scene_;
    input::Input                          input_;

    std::chrono::steady_clock::time_point m_current_time;
    double                                m_time_accumulator{0.0};
    double                                m_time{0.0};
};

void example_run()
{
    Example example{};

    example.run();
}
} // namespace example