#pragma once
#include "scene.hpp"
#include "seop_command/command.hpp"

namespace seop::scene
{
class Particle_change_command : public command::Command
{
  public:
    Particle_change_command(size_t cnt, Scene &scene) : scene_(scene)
    {
    }

    ~Particle_change_command() noexcept override = default;

    void set_particle_count(size_t cnt)
    {
        cnt_ = cnt;
    }

    bool execute() override
    {
        scene_.create_particles_cube_group(cnt_);
        return true;
    }

  private:
    size_t cnt_;
    Scene &scene_;
};
} // namespace  seop::scene
