#include "seop_message/message.hpp"

namespace seop::scene
{
class Particle_change_message : public msg::Message
{
  public:
    Particle_change_message(size_t cnt, const void* particle_data);
    ~Particle_change_message() override = default;

    [[nodiscard]] auto particle_count() const -> size_t;
    [[nodiscard]] auto particle_data() const -> const void*;

  private:
    size_t      cnt_;
    const void* particle_data_;
};

} // namespace seop::scene