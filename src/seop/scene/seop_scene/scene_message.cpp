#include "scene_message.hpp"

namespace seop::scene
{

Particle_buffer_update_message::Particle_buffer_update_message(uint32_t id, size_t size, const void* particle_data)
    : buffer_id_(id), buffer_size_(size), particle_data_(particle_data)
{
}

Wire_buffer_update_message::Wire_buffer_update_message(uint32_t id, size_t size, const void* wire_nodes_data)
    : buffer_id_(id), buffer_size_(size), wire_nodes_data_(wire_nodes_data)
{
}

} // namespace seop::scene