#pragma once

/* 기호 모음
 ̂


*/
namespace seop::math
{
static constexpr float π = 3.14159265f;
static constexpr float Rad = 180 / π;
static constexpr float Inv_rad = π / 180;
auto                   ToRadian(float degree) -> float;
auto                   ToDegree(float radian) -> float;

// constant
static constexpr float q = 1.602176634e-19f;
static constexpr float ε0 = 8.85e-12f;

class Vec2
{
  public:
    constexpr Vec2(float x, float y) noexcept : x_(x), y_(y)
    {
    }

    auto              operator=(const Vec2& v) noexcept -> Vec2& = default;
    auto              operator+(const Vec2& v) const noexcept -> Vec2;
    auto              operator-(const Vec2& v) const noexcept -> Vec2;
    auto              operator*(float f) const noexcept -> Vec2;
    auto              operator/(float f) const noexcept -> Vec2;

    float             x_{0.0f};
    float             y_{0.0f};

    auto              Dot(const Vec2& v) const noexcept -> float;
    auto              LengthSquared() const noexcept -> float;
    auto              Length() const noexcept -> float;
    auto              Normalized() const noexcept -> Vec2;
    void              Normalize() noexcept;

    // Constants
    static const Vec2 Zero;
    static const Vec2 One;
};

class Vec3
{
  public:
    constexpr Vec3() noexcept
    {
    }

    constexpr Vec3(float x, float y, float z) noexcept : x_{x}, y_{y}, z_{z}
    {
    }

    auto              operator-=(const Vec3& v) noexcept -> Vec3&;
    auto              operator+=(const Vec3& v) noexcept -> Vec3&;
    auto              operator==(const Vec3& v) noexcept -> bool;
    auto              operator=(const Vec3& v) noexcept -> Vec3& = default;
    auto              operator+(const Vec3& v) const noexcept -> Vec3;
    auto              operator-(const Vec3& v) const noexcept -> Vec3;
    auto              operator*(float f) const noexcept -> Vec3;
    auto              operator/(float f) const noexcept -> Vec3;

    float             x_{0.0f};
    float             y_{0.0f};
    float             z_{0.0f};

    auto              Dot(const Vec3& v) const noexcept -> float;
    auto              Cross(const Vec3& v) const noexcept -> Vec3;
    auto              LengthSquared() const noexcept -> float;
    auto              Length() const noexcept -> float;
    auto              Normalized() const noexcept -> Vec3;
    void              Normalize() noexcept;

    static const Vec3 Zero;
    static const Vec3 One;
};

class Vec4
{
  public:
    constexpr Vec4(float x, float y, float z, float w) noexcept : x_{x}, y_{y}, z_{z}, w_{w}
    {
    }

    constexpr Vec4(int x, int y, int z, int w) noexcept
        : x_{static_cast<float>(x)}, y_{static_cast<float>(y)}, z_{static_cast<float>(z)}, w_{static_cast<float>(w)}
    {
    }

    auto              operator=(const Vec4& v) noexcept -> Vec4& = default;
    auto              operator+=(const Vec4& v) noexcept -> Vec4&;
    auto              operator+(const Vec4& v) const noexcept -> Vec4;
    auto              operator-(const Vec4& v) const noexcept -> Vec4;
    auto              operator*(float f) const noexcept -> Vec4;
    auto              operator/(float f) const noexcept -> Vec4;

    float             x_{0.0f};
    float             y_{0.0f};
    float             z_{0.0f};
    float             w_{0.0f};

    auto              xyz() const noexcept -> Vec3;
    auto              xy() const noexcept -> Vec3;

    auto              Dot(const Vec4& v) const noexcept -> float;
    auto              LengthSquared() const noexcept -> float;
    auto              Length() const noexcept -> float;
    auto              Normalized() const noexcept -> Vec4;
    void              Normalize() noexcept;

    static const Vec4 Zero;
    static const Vec4 One;
};

class Matrix
{
  public:
    constexpr Matrix() noexcept
    {
    }

    constexpr Matrix(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20,
                     float m21, float m22, float m23, float m30, float m31, float m32, float m33) noexcept
        : _00(m00), _01(m01), _02(m02), _03(m03), _10(m10), _11(m11), _12(m12), _13(m13), _20(m20), _21(m21), _22(m22),
          _23(m23), _30(m30), _31(m31), _32(m32), _33(m33)
    {
    }

    static Matrix CreateTranslation(float x, float y, float z) noexcept;
    static Matrix CreateScale(float xs, float ys, float zs) noexcept;
    static Matrix CreatePerspective(float fov, float aspect, float nearZ, float farZ) noexcept;
    static Matrix CreateRotationX(float angle) noexcept;
    static Matrix CreateRotationY(float angle) noexcept;
    static Matrix CreateRotationZ(float angle) noexcept;
    static Matrix CreateLookAt(const Vec3& eye, const Vec3& target, const Vec3& up) noexcept;

    auto          operator=(const Matrix& m) noexcept -> Matrix& = default;
    auto          operator+(const Matrix& m) const noexcept -> Matrix;
    auto          operator-(const Matrix& m) const noexcept -> Matrix;
    auto          operator*(const Matrix& m) const noexcept -> Matrix;
    auto          operator*(const Vec4& v) const noexcept -> Vec4;

    void          Transpose() noexcept;
    auto          Transposed() const noexcept -> Matrix;
    void          Inverse() noexcept;
    auto          Invert() const noexcept -> Matrix;

    auto          Determinant() const -> float;
    float _00{0.0f}, _01{0.0f}, _02{0.0f}, _03{0.0f}, _10{0.0f}, _11{0.0f}, _12{0.0f}, _13{0.0f}, _20{0.0f}, _21{0.0f},
        _22{0.0f}, _23{0.0f}, _30{0.0f}, _31{0.0f}, _32{0.0f}, _33{0.0f};

    static const Matrix Zero;
    static const Matrix Identity;

  private:
    auto Determinant3x3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21,
                        float m22) const -> float;
};

inline constexpr Vec2   Vec2::Zero = {0.f, 0.f};
inline constexpr Vec2   Vec2::One = {1.f, 1.f};

inline constexpr Vec3   Vec3::Zero = {0.f, 0.f, 0.f};
inline constexpr Vec3   Vec3::One = {1.f, 1.f, 1.f};

inline constexpr Vec4   Vec4::Zero = {0.f, 0.f, 0.f, 0.f};
inline constexpr Vec4   Vec4::One = {1.f, 1.f, 1.f, 1.f};

inline constexpr Matrix Matrix::Zero = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

inline constexpr Matrix Matrix::Identity = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
                                            0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};

struct Closest_points
{
    Vec3 src_point{Vec3::Zero};
    Vec3 dst_point{Vec3::Zero};
};

auto closest_dist_point_to_ray(const Vec3& vert, const Vec3& ori, const Vec3& dir) -> float;

auto dist_line_to_line(const Vec3& src_ori, const Vec3& src_dir, const Vec3& dst_ori, const Vec3& dst_dir) -> float;
auto closest_points_to_line(const Vec3& src_ori, const Vec3& src_dir, const Vec3& dst_ori, const Vec3& dst_dir)
    -> Closest_points;

} // namespace seop::math
