#pragma once
#include "seop_math/math.hpp"

#include <cstdint>

namespace seop
{
class Context;
} // namespace seop

namespace seop::scene
{
class Camera;
} // namespace seop::scene

namespace seop::editor
{

enum class Edit_mode {
    Plane_xy,
    Plane_yz,
    Plane_zx,
    End,
};

enum Edit_state : std::uint32_t {
    EDIT_NONE = 0,
    EDIT_HOLD_WIRE = 1 << 0,
};

class Editor
{
  public:
    Editor(scene::Camera& cam);
    ~Editor() = default;

    void update(Context& ctx);
    void ray_trace(Context& ctx);
    auto get_snap_pos(float interval, const math::Vec3& ray_ori, const math::Vec3& ray_dir, const math::Vec3& vert_pos,
                      Edit_mode mode) -> math::Vec3;

  private:
    scene::Camera& active_camera_;

    float          snap_interval{50.0f};
    Edit_mode      edit_mode_{Edit_mode::Plane_xy};

    math::Vec3     pre_snapped_pos{math::Vec3::Zero};
    math::Vec3     cur_intersection{math::Vec3::Zero};
};
} // namespace seop::editor