#pragma once
#include "seop_message/message.hpp"

namespace seop::scene
{
class Particle_buffer_update_message : public msg::Message
{
  public:
    Particle_buffer_update_message(uint32_t id, size_t size, const void* particle_data);
    ~Particle_buffer_update_message() override = default;

    uint32_t    buffer_id_;
    size_t      buffer_size_;
    const void* particle_data_;
};

class Wire_buffer_update_message : public msg::Message
{
  public:
    Wire_buffer_update_message(uint32_t id, size_t size, const void* wire_nodes_data);
    ~Wire_buffer_update_message() override = default;

    uint32_t    buffer_id_;
    size_t      buffer_size_;
    const void* wire_nodes_data_;
};
} // namespace seop::scene