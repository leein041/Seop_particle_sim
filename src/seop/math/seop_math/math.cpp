#include "math.hpp"

#include <algorithm>
#include <cmath>

namespace seop::math
{
auto Vec2::operator+(const Vec2& v) const noexcept -> Vec2
{
    return Vec2{x_ + v.x_, y_ + v.y_};
}

auto Vec2::operator-(const Vec2& v) const noexcept -> Vec2
{
    return Vec2{x_ - v.x_, y_ - v.y_};
}

auto Vec2::operator*(float f) const noexcept -> Vec2
{
    return Vec2(x_ * f, y_ * f);
}

auto Vec2::operator/(float f) const noexcept -> Vec2
{
    return Vec2(x_ / f, y_ / f);
}

auto Vec2::Dot(const Vec2& v) const noexcept -> float
{
    return x_ * v.x_ + y_ * v.y_;
}

auto Vec2::LengthSquared() const noexcept -> float
{
    return x_ * x_ + y_ * y_;
}

auto Vec2::Length() const noexcept -> float
{
    return std::sqrt(LengthSquared());
}

auto Vec2::Normalized() const noexcept -> Vec2
{
    float len = Length();
    if (len > 1e-6f) {
        float inv_len = 1 / len;
        return Vec2{x_ * inv_len, y_ * inv_len};
    } else {
        return Vec2{0.f, 0.f};
    }
}

void Vec2::Normalize() noexcept
{
    float len = Length();
    if (len > 1e-6f) {
        *this = Normalized();
    }
}

auto Vec3::operator-=(const Vec3& v) noexcept -> Vec3&
{
    this->x_ -= v.x_;
    this->y_ -= v.y_;
    this->z_ -= v.z_;
    return *this;
}

auto Vec3::operator+=(const Vec3& v) noexcept -> Vec3&
{
    this->x_ += v.x_;
    this->y_ += v.y_;
    this->z_ += v.z_;
    return *this;
}

auto Vec3::operator==(const Vec3& v) noexcept -> bool
{
    return (x_ == v.x_) && (y_ == v.y_) && (z_ == v.z_);
}

auto Vec3::operator+(const Vec3& v) const noexcept -> Vec3
{
    return Vec3{x_ + v.x_, y_ + v.y_, z_ + v.z_};
}

auto Vec3::operator-(const Vec3& v) const noexcept -> Vec3
{
    return Vec3{x_ - v.x_, y_ - v.y_, z_ - v.z_};
}

auto Vec3::operator*(float f) const noexcept -> Vec3
{
    return Vec3(x_ * f, y_ * f, z_ * f);
}

auto Vec3::operator/(float f) const noexcept -> Vec3
{
    return Vec3(x_ / f, y_ / f, z_ / f);
}

auto Vec3::Dot(const Vec3& v) const noexcept -> float
{
    return x_ * v.x_ + y_ * v.y_ + z_ * v.z_;
}

auto Vec3::Cross(const Vec3& v) const noexcept -> Vec3
{
    return Vec3(y_ * v.z_ - z_ * v.y_, z_ * v.x_ - x_ * v.z_, x_ * v.y_ - y_ * v.x_);
}

auto Vec3::LengthSquared() const noexcept -> float
{
    return x_ * x_ + y_ * y_ + z_ * z_;
}

auto Vec3::Length() const noexcept -> float
{
    return std::sqrt(LengthSquared());
}

auto Vec3::Normalized() const noexcept -> Vec3
{
    float len = Length();
    if (len > 1e-6f) {
        return Vec3{x_ / len, y_ / len, z_ / len};
    } else {
        return Vec3{0.f, 0.f, 0.f};
    }
}

void Vec3::Normalize() noexcept
{
    float len = Length();
    if (len > 1e-6f) {
        *this = Normalized();
    }
}

auto Vec4::operator+=(const Vec4& v) noexcept -> Vec4&
{
    this->x_ += v.x_;
    this->y_ += v.y_;
    this->z_ += v.z_;
    this->w_ += v.w_;
    return *this;
}

auto Vec4::operator+(const Vec4& v) const noexcept -> Vec4
{
    return Vec4{x_ + v.x_, y_ + v.y_, z_ + v.z_, w_ + v.w_};
}

auto Vec4::operator-(const Vec4& v) const noexcept -> Vec4
{
    return Vec4{x_ - v.x_, y_ - v.y_, z_ - v.z_, w_ - v.w_};
}

auto Vec4::operator*(float f) const noexcept -> Vec4
{
    return Vec4(x_ * f, y_ * f, z_ * f, w_ * f);
}

auto Vec4::operator/(float f) const noexcept -> Vec4
{
    return Vec4(x_ / f, y_ / f, z_ / f, w_ / f);
}

auto Vec4::xyz() const noexcept -> Vec3
{
    return Vec3(x_, y_, z_);
}

auto Vec4::Dot(const Vec4& v) const noexcept -> float
{
    return x_ * v.x_ + y_ * v.y_ + z_ * v.z_ + w_ * v.w_;
}

auto Vec4::LengthSquared() const noexcept -> float
{
    return x_ * x_ + y_ * y_ + z_ * z_ + w_ * w_;
}

auto Vec4::Length() const noexcept -> float
{
    return std::sqrt(LengthSquared());
}

auto Vec4::Normalized() const noexcept -> Vec4
{
    float len = Length();
    if (len > 1e-6f) {
        return Vec4{x_ / len, y_ / len, z_ / len, w_ / len};
    } else {
        return Vec4{0.f, 0.f, 0.f, 0.f};
    }
}

void Vec4::Normalize() noexcept
{
    float len = Length();
    if (len > 1e-6f) {
        *this = Normalized();
    }
}

Matrix Matrix::CreateTranslation(float x, float y, float z) noexcept
{
    return Matrix{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1};
}

Matrix Matrix::CreateScale(float xs, float ys, float zs) noexcept
{
    return Matrix();
}

Matrix Matrix::CreatePerspective(float fov, float aspect, float nearZ, float farZ) noexcept
{
    float h = 1.0f / tanf(fov * 0.5f);
    float w = h / aspect;
    float a = -(farZ + nearZ) / (farZ - nearZ); // OpenGL은 -1 ~ 1 매핑
    float b = -(2.0f * farZ * nearZ) / (farZ - nearZ);

    return Matrix(w, 0, 0, 0, 0, h, 0, 0, 0, 0, a, -1, 0, 0, b, 0);
}

Matrix Matrix::CreateRotationX(float angle) noexcept
{
    float s = std::sin(angle);
    float c = std::cos(angle);
    return Matrix(1, 0, 0, 0, 0, c, s, 0, 0, -s, c, 0, 0, 0, 0, 1);
}

Matrix Matrix::CreateRotationY(float angle) noexcept
{
    float s = std::sin(angle);
    float c = std::cos(angle);
    return Matrix(c, 0, s, 0, 0, 1, 0, 0, -s, 0, c, 0, 0, 0, 0, 1);
}

Matrix Matrix::CreateRotationZ(float angle) noexcept
{
    float s = std::sin(angle);
    float c = std::cos(angle);
    return Matrix(c, -s, 0, 0, s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

Matrix Matrix::CreateLookAt(const Vec3& eye, const Vec3& target, const Vec3& up) noexcept
{
    Vec3 zAxis = (eye - target).Normalized(); // 오른손: 카메라 뒤를 향함
    Vec3 xAxis = up.Cross(zAxis).Normalized();
    Vec3 yAxis = zAxis.Cross(xAxis);

    return Matrix(xAxis.x_, yAxis.x_, zAxis.x_, 0.0f,                     // 1열 (_00, _01, _02, _03)
                  xAxis.y_, yAxis.y_, zAxis.y_, 0.0f,                     // 2열 (_10, _11, _12, _13)
                  xAxis.z_, yAxis.z_, zAxis.z_, 0.0f,                     // 3열 (_20, _21, _22, _23)
                  -xAxis.Dot(eye), -yAxis.Dot(eye), -zAxis.Dot(eye), 1.0f // 4행
    );
}

auto Matrix::operator+(const Matrix& m) const noexcept -> Matrix
{
    return Matrix(_00 + m._00, _01 + m._01, _02 + m._02, _03 + m._03, _10 + m._10, _11 + m._11, _12 + m._12,
                  _13 + m._13, _20 + m._20, _21 + m._21, _22 + m._22, _23 + m._23, _30 + m._30, _31 + m._31,
                  _32 + m._32, _33 + m._33);
}

auto Matrix::operator-(const Matrix& m) const noexcept -> Matrix
{
    return Matrix(_00 - m._00, _01 - m._01, _02 - m._02, _03 - m._03, _10 - m._10, _11 - m._11, _12 - m._12,
                  _13 - m._13, _20 - m._20, _21 - m._21, _22 - m._22, _23 - m._23, _30 - m._30, _31 - m._31,
                  _32 - m._32, _33 - m._33);
}

auto Matrix::operator*(const Matrix& m) const noexcept -> Matrix
{
    return Matrix(
        _00 * m._00 + _10 * m._01 + _20 * m._02 + _30 * m._03, _01 * m._00 + _11 * m._01 + _21 * m._02 + _31 * m._03,
        _02 * m._00 + _12 * m._01 + _22 * m._02 + _32 * m._03, _03 * m._00 + _13 * m._01 + _23 * m._02 + _33 * m._03,

        _00 * m._10 + _10 * m._11 + _20 * m._12 + _30 * m._13, _01 * m._10 + _11 * m._11 + _21 * m._12 + _31 * m._13,
        _02 * m._10 + _12 * m._11 + _22 * m._12 + _32 * m._13, _03 * m._10 + _13 * m._11 + _23 * m._12 + _33 * m._13,

        _00 * m._20 + _10 * m._21 + _20 * m._22 + _30 * m._23, _01 * m._20 + _11 * m._21 + _21 * m._22 + _31 * m._23,
        _02 * m._20 + _12 * m._21 + _22 * m._22 + _32 * m._23, _03 * m._20 + _13 * m._21 + _23 * m._22 + _33 * m._23,

        _00 * m._30 + _10 * m._31 + _20 * m._32 + _30 * m._33, _01 * m._30 + _11 * m._31 + _21 * m._32 + _31 * m._33,
        _02 * m._30 + _12 * m._31 + _22 * m._32 + _32 * m._33, _03 * m._30 + _13 * m._31 + _23 * m._32 + _33 * m._33);
}

auto Matrix::operator*(const Vec4& v) const noexcept -> Vec4
{
    return Vec4{_00 * v.x_ + _10 * v.y_ + _20 * v.z_ + _30 * v.w_, _01 * v.x_ + _11 * v.y_ + _21 * v.z_ + _31 * v.w_,
                _02 * v.x_ + _12 * v.y_ + _22 * v.z_ + _32 * v.w_, _03 * v.x_ + _13 * v.y_ + _23 * v.z_ + _33 * v.w_};
}

void Matrix::Transpose() noexcept
{
    *this = Transposed();
}

auto Matrix::Transposed() const noexcept -> Matrix
{
    return Matrix{_00, _10, _20, _30, _01, _11, _21, _31, _02, _12, _22, _32, _03, _13, _23, _33};
}

void Matrix::Inverse() noexcept
{
    *this = Invert();
}

auto Matrix::Invert() const noexcept -> Matrix
{
    float det = Determinant();
    if (std::abs(det) < 1e-6f)
        return Matrix::Zero; // 역행렬이 없는 경우

    float  invDet = 1.0f / det;
    Matrix res;

    // |_00  _01  _02 _03|
    // |_10  _11  _12 _13|
    // |_20  _21  _22 _23|
    // |_30  _31  _32 _33|
    return Matrix{Determinant3x3(_11, _21, _31, _12, _22, _32, _13, _23, _33) * invDet,
                  -Determinant3x3(_01, _21, _31, _02, _22, _32, _03, _23, _33) * invDet,
                  Determinant3x3(_01, _11, _31, _02, _12, _32, _03, _13, _33) * invDet,
                  -Determinant3x3(_01, _11, _21, _02, _12, _22, _03, _13, _23) * invDet,

                  // 2열 (res._10, res._11, res._12, res._13)
                  -Determinant3x3(_10, _20, _30, _12, _22, _32, _13, _23, _33) * invDet,
                  Determinant3x3(_00, _20, _30, _02, _22, _32, _03, _23, _33) * invDet,
                  -Determinant3x3(_00, _10, _30, _02, _12, _32, _03, _13, _33) * invDet,
                  Determinant3x3(_00, _10, _20, _02, _12, _22, _03, _13, _23) * invDet,

                  // 3열 (res._20, res._21, res._22, res._23)
                  Determinant3x3(_10, _20, _30, _11, _21, _31, _13, _23, _33) * invDet,
                  -Determinant3x3(_00, _20, _30, _01, _21, _31, _03, _23, _33) * invDet,
                  Determinant3x3(_00, _10, _30, _01, _11, _31, _03, _13, _33) * invDet,
                  -Determinant3x3(_00, _10, _20, _01, _11, _21, _03, _13, _23) * invDet,

                  // 4열 (res._30, res._31, res._32, res._33)
                  -Determinant3x3(_10, _20, _30, _11, _21, _31, _12, _22, _32) * invDet,
                  Determinant3x3(_00, _20, _30, _01, _21, _31, _02, _22, _32) * invDet,
                  -Determinant3x3(_00, _10, _30, _01, _11, _31, _02, _12, _32) * invDet,
                  Determinant3x3(_00, _10, _20, _01, _11, _21, _02, _12, _22) * invDet};
}

auto Matrix::Determinant() const -> float
{
    return _00 * Determinant3x3(_11, _21, _31, _12, _22, _32, _13, _23, _33) -
           _01 * Determinant3x3(_10, _20, _30, _12, _22, _32, _13, _23, _33) +
           _02 * Determinant3x3(_10, _20, _30, _11, _21, _31, _13, _23, _33) -
           _03 * Determinant3x3(_10, _20, _30, _11, _21, _31, _12, _22, _32);
}

auto Matrix::Determinant3x3(float m00, float m10, float m20, float m01, float m11, float m21, float m02, float m12,
                            float m22) const -> float
{
    // |_00  _01  _02 |
    // |_10  _11  _12 |
    // |_20  _21  _22 |
    return m00 * (m11 * m22 - m12 * m21) - m01 * (m10 * m22 - m12 * m20) + m02 * (m10 * m21 - m11 * m20);
}

auto ToRadian(float degree) -> float
{
    return degree * Inv_rad;
}

auto ToDegree(float radian) -> float
{
    return radian * Rad;
}

auto closest_dist_point_to_ray(const Vec3& vert, const Vec3& ori, const Vec3& dir) -> float
{
    Vec3  A = ori;
    Vec3  L = dir;
    Vec3  AP = vert - A;
    float t = AP.Dot(L) / L.Dot(L);
    Vec3  H = A + L * t; // H = A + tv
    Vec3  HP = vert - H;

    return HP.LengthSquared();
}

auto dist_line_to_line(const Vec3& src_ori, const Vec3& src_dir, const Vec3& dst_ori, const Vec3& dst_dir) -> float
{
    auto& points = closest_points_to_line(src_ori, src_dir, dst_ori, dst_dir);
    return (points.dst_point - points.src_point).LengthSquared();
}

auto closest_points_to_line(const Vec3& src_ori, const Vec3& src_dir, const Vec3& dst_ori, const Vec3& dst_dir)
    -> Closest_points
{
    Vec3  w0 = dst_ori - src_ori;
    float a = dst_dir.Dot(dst_dir);
    float b = dst_dir.Dot(src_dir);
    float c = src_dir.Dot(src_dir);
    float d = dst_dir.Dot(w0);
    float e = src_dir.Dot(w0);
    float denom = a * c - b * b;

    float s, t;

    // 1. 매개변수 s 계산
    if (denom < 1e-6) { // 평행할 때
        s = 0.0f;
        t = (b > c ? d / b : e / c);
    } else {
        s = (b * e - c * d) / denom;
        t = (a * e - b * d) / denom;
    }

    // 2. 선분의 범위(0~1)로 클램핑
    s = std::clamp(s, 0.0f, 1.0f);
    t = std::max(0.0f, t);

    // 3. 최종 거리 계산
    Closest_points points{src_ori + src_dir * t, dst_ori + dst_dir * s};
    return points;
}


} // namespace seop::math
