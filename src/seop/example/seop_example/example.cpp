#include "example.hpp"

#include "seop_entity/particle.hpp"
#include "seop_graphic/device.hpp"
#include "seop_imgui/imgui_renderer.hpp"
#include "seop_input/input.hpp"
#include "seop_math/math.hpp"
#include "seop_scene/camera.hpp"
#include "seop_scene/scene.hpp"

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
        imgui_renderer_.shut_down();
    }

    void tick()
    {
        const auto tick_end_time = std::chrono::steady_clock::now();

        // Update fixed steps
        const auto new_time = std::chrono::steady_clock::now();
        const auto duration = new_time - m_current_time;
        double frame_time = std::chrono::duration<double, std::ratio<1>>(duration).count();

        if (frame_time > 0.25) {
            frame_time = 0.25;
        }

        m_current_time = new_time;
        m_time_accumulator += frame_time;

        float inv_frame_rate = imgui_renderer_.inv_frame_rate();
        const double dt = 1.0 * inv_frame_rate;
        float dt_f = static_cast<float>(dt);

        // update
        input_.update(window_);
        scene_.update_camera(imgui_renderer_.is_hovered(), dt_f, input_, window_);

        // compute
        while (m_time_accumulator >= dt) {
            device_.compute(imgui_renderer_.is_hovered(), dt_f, scene_.data(), input_.data());
            m_time_accumulator -= dt;
            m_time += dt;
        }
        // render
        device_.prepare_grid(scene_.camera().data().view, scene_.camera().data().projection);
        device_.draw_grid();
        device_.prepare_draw(scene_.camera().data().view, scene_.camera().data().projection,
                             scene_.data().particle_properties.particle_size);
        device_.draw(scene_.data().particle_properties.particle_count);

        imgui_renderer_.render(scene_.data(), scene_.camera());
    }

    void init()
    {
        window_.init();
        scene_.init();
        device_.init();
        imgui_renderer_.init(window_);
        input_.init();
        reset();
    }

    void update()
    {
    }

    void begin_frame()
    {
        device_.clear_frame();
    }

    void end_frame()
    {
        if (scene_.data().particle_properties.particle_reset) {
            reset();
        }
        window_.buffer_swap();
    }

    void reset()
    {
        device_.clear_frame();
        scene_.create_particles(scene_.data().particle_properties.particle_count);
        scene_.create_attractors(scene_.data().attractor_properties.attractor_count);

        device_.update_shader_buffer(device_.particle_sb, 0,
                                     sizeof(entity::Particle) * scene_.data().particle_properties.particle_count,
                                     scene_.data().entities.particles.data());
        device_.update_shader_buffer(device_.ssbo_attractor, 1,
                                     sizeof(entity::Attractor) * scene_.data().attractor_properties.attractor_count,
                                     scene_.data().entities.attractors.data());

        device_.update_shader_buffer(device_.grid_sb, 2, sizeof(math::Vec4) * device_.grid_.size(),
                                     device_.grid_.data());
    }

  private:
    window::Glf_window window_;
    graphic::Device device_;
    imgui::Imgui_renderer imgui_renderer_;
    scene::Scene scene_;
    input::Input input_;

    std::chrono::steady_clock::time_point m_current_time;
    double m_time_accumulator{0.0};
    double m_time{0.0};
};

void example_run()
{
    Example example{};
    example.run();
}
} // namespace example