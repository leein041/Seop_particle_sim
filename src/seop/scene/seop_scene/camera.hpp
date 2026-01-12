#pragma once

#include "seop_math/math.hpp"

namespace seop::scene
{
enum class Projection_type
{
    Perspective,  // 원근 카메라
    Orthographic, // 직교 카메라
    end,
};

class Camera_trasform
{
  public:
    math::Vec3 pos{0.0f, 0.0f, 2.0f};
    math::Vec3 forward{0.0f, 0.0f, -1.0f};
    math::Vec3 up{0.0f, 1.0f, 0.0f};
    math::Vec3 right{1.0f, 0.0f, 0.0f};
    
    float speed_scale{500.0f};
    float yaw{0.0f};
    float pitch{0.0f};
};

class Camera_frustum

{
  public:
    float fov{0.785f};             // 약 45도
    float aspect{1600.f / 900.0f}; //
    float near{0.1f};
    float far{10000.0f};
};

class Camera_data
{
  public:
    Camera_frustum frustum;
    Camera_trasform transform;
    math::Matrix view;
    math::Matrix projection;
    Projection_type projection_type{Projection_type::Perspective};
};

class Camera

{
  public:
    Camera();
    ~Camera() = default;
    void init();
    void update();
    void make_view();
    void make_projection();

    [[nodiscard]] auto data() const -> const Camera_data &;
    [[nodiscard]] auto data() -> Camera_data &;
    void set_transform(const Camera_trasform& tr);

    // temp
    void update_transform();



  private:
    Camera_data data_;
};
} // namespace seop::scene