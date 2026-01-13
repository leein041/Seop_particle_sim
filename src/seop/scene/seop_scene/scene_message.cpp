#include "scene_message.hpp"

namespace seop::scene
{
Particle_change_message::Particle_change_message(size_t cnt, const void* particle_data)
    : cnt_(cnt), particle_data_(particle_data)
{
}

auto Particle_change_message::particle_count() const -> size_t
{
    return cnt_;
}

auto Particle_change_message::particle_data() const -> const void*
{
    return particle_data_;
}
} // namespace seop::scene