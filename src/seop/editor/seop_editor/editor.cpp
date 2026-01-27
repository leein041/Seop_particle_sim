#include "editor.hpp"
#include "seop_context/context.hpp"
#include "seop_input/input.hpp"
#include "seop_input/keys.hpp"
#include "seop_message/message.hpp"
#include "seop_primitive/vertex.hpp"
#include "seop_raytrace/ray.hpp"
#include "seop_scene/camera.hpp"
#include "seop_scene/scene.hpp"
#include "seop_scene/scene_message.hpp"
#include "seop_window/glf_window.hpp"
#include "seop_opengl/buffer.hpp"
#include "seop_item/wire.hpp"

namespace seop::editor
{
using namespace math;
using namespace primitive;

Editor::Editor(scene::Camera& cam) : active_camera_(cam)
{
}

void Editor::update(Context& ctx)
{
    ray_trace(ctx);
    scene::Camera_state&    cam_state = active_camera_.camera_state();
    scene::Camera_trasform& tr = active_camera_.data().transform;
    // NOTE : it's RH
    if (ctx.input->get_key(input::Key_code::W)) {
        tr.pos += tr.forward * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key(input::Key_code::S)) {
        tr.pos -= tr.forward * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key(input::Key_code::A)) {
        tr.pos -= tr.right * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key(input::Key_code::D)) {
        tr.pos += tr.right * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key(input::Key_code::E)) {
        tr.pos += tr.up * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key(input::Key_code::Q)) {
        tr.pos -= tr.up * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }

    if (ctx.input->get_key(input::Key_code::RButton)) {
        Vec2  delta = ctx.input->get_delta_cursor_pos();

        float dx = delta.x_ * ctx.input->mouse_sensitivity;
        float dy = delta.y_ * ctx.input->mouse_sensitivity;

        // 각도 업데이트
        tr.yaw += dx;
        tr.pitch -= dy;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key_down(input::Key_code::LButton)) {
        Vec2   pos_ndc = ctx.window->get_cursor_pos_ndc();
        Vec4   clip_space(pos_ndc.x_, pos_ndc.y_, -1.0f, 1.0f);
        Matrix projection_matirx_inv = active_camera_.data().projection.Invert();
        Matrix view_matirx_inv = active_camera_.data().view.Invert();

        Vec4   view_space = projection_matirx_inv * clip_space;
        view_space.z_ = -1.0f;
        view_space.w_ = 0.0f;
        Vec4 ray_direction = view_matirx_inv * view_space;
        Vec3 eff_ray_dir = ray_direction.xyz();
        eff_ray_dir.Normalize();
        float ray_distance = 300.0f;
        ctx.input->set_ray_point(eff_ray_dir * ray_distance + tr.pos);
    }
    // 위치가 변했으므로 행렬 다시 계산
    // test
    active_camera_.update();
}

void Editor::ray_trace(Context& ctx)
{
    // ndc 좌표 변환]
    Vec2                cursor_ndc = ctx.window->get_cursor_pos_ndc();
    scene::Camera_data& cam_data = active_camera_.data();

    raytrace::Ray ray = raytrace::make_ray(cursor_ndc, cam_data.transform.pos, cam_data.view, cam_data.projection);

    std::vector<Vertex_pcs>& verts = ctx.scene->data().wire_properites.wire_nodes.vb.vertices;
    math::Closest_points     closest_points_to_line;
    for (size_t i = 0; i < verts.size(); i += 2) {

        // find hover vert only
        if (math::closest_dist_point_to_ray(verts[i].pos.xyz(), ray.ori, ray.dir) < 100.0f) {
            verts[i].col = Vec4{0.7f, 0.7f, 0.7f, 1.0f};
            verts[i].size = 12.0f;
            verts[i].state |= Vertex_state::STATE_HOVER_ONLY;
        } else {
            verts[i].col = Vec4{1.0f, 1.0f, 0.0f, 1.0f};
            verts[i].size = 1.0f;
            verts[i].state &= ~Vertex_state::STATE_HOVER_ONLY;
        }

        if (math::closest_dist_point_to_ray(verts[i + 1].pos.xyz(), ray.ori, ray.dir) < 100.0f) {
            verts[i + 1].col = Vec4{0.7f, 0.7f, 0.7f, 1.0f};
            verts[i + 1].size = 12.0f;
            verts[i + 1].state |= Vertex_state::STATE_HOVER_ONLY;
        } else {
            verts[i + 1].col = Vec4{1.0f, 1.0f, 0.0f, 1.0f};
            verts[i + 1].size = 1.0f;
            verts[i + 1].state &= ~Vertex_state::STATE_HOVER_ONLY;
        }

        if ((verts[i].state & Vertex_state::STATE_HOVER_ONLY) ||
            (verts[i + 1].state & Vertex_state::STATE_HOVER_ONLY)) { // 버텍스 HOVER 가 우선

            verts[i].state &= ~Vertex_state::STATE_HOVER_WIRE;     // 임시
            verts[i + 1].state &= ~Vertex_state::STATE_HOVER_WIRE; // 임시
            continue;
        }

        // find hover wire
        closest_points_to_line = math::closest_points_to_line(ray.ori, ray.dir, verts[i].pos.xyz(),
                                                              verts[i + 1].pos.xyz() - verts[i].pos.xyz());
        if ((closest_points_to_line.src_point - closest_points_to_line.dst_point).LengthSquared() < 100.0f) {
            verts[i].col = Vec4{1.0f, 0.0f, 0.0f, 1.0f};
            verts[i + 1].col = Vec4{1.0f, 0.0f, 0.0f, 1.0f};
            verts[i].size = 1.0f;
            verts[i + 1].size = 1.0f;
            verts[i].state |= Vertex_state::STATE_HOVER_WIRE;
            verts[i + 1].state |= Vertex_state::STATE_HOVER_WIRE;
        } else {
            verts[i].col = Vec4{1.0f, 1.0f, 0.0f, 1.0f};
            verts[i + 1].col = Vec4{1.0f, 1.0f, 0.0f, 1.0f};
            verts[i].size = 1.0f;
            verts[i + 1].size = 1.0f;
            verts[i].state &= ~Vertex_state::STATE_HOVER_WIRE;
            verts[i + 1].state &= ~Vertex_state::STATE_HOVER_WIRE;
        }
    }
    // process keyboard ( short cut )
    if (ctx.input->get_key_down(input::Key_code::G)) {
        for (auto& vert : verts) {
            if (vert.state & Vertex_state::STATE_HOVER_WIRE) {
                size_t idx_0 = &vert - verts.data(); // 포인터 차이로 인덱스 구하기
                size_t idx_1 = (idx_0 % 2 == 0) ? idx_0 + 1 : idx_0 - 1;
                verts[idx_0].state |= Vertex_state::STATE_DESTROYED;
                verts[idx_1].state |= Vertex_state::STATE_DESTROYED;
            } else if (vert.state & Vertex_state::STATE_HOVER_ONLY) {
                size_t idx_0 = &vert - verts.data(); // 포인터 차이로 인덱스 구하기
                size_t idx_1 = (idx_0 % 2 == 0) ? idx_0 + 1 : idx_0 - 1;
                size_t first = std::min(idx_0, idx_1);
                verts[first].state |= Vertex_state::STATE_DESTROYED;
                verts[first + 1].state |= Vertex_state::STATE_DESTROYED;
            }
        }
    }
    if (ctx.input->get_key_down(input::Key_code::V)) {
        std::vector<Vertex_pcs> add;
        for (auto& vert : verts) {
            if (vert.state & Vertex_state::STATE_HOVER_WIRE && !(vert.state & Vertex_state::STATE_HOLD)) {
                size_t     idx_0 = &vert - verts.data(); // 포인터 차이로 인덱스 구하기
                size_t     idx_1 = (idx_0 % 2 == 0) ? idx_0 + 1 : idx_0 - 1;
                Vertex_pcs v1{verts[idx_0].pos, verts[idx_0].col, 1.0f, 0.0f, ++vert_id, Vertex_state::STATE_HOLD};
                Vertex_pcs v2{verts[idx_1].pos, verts[idx_1].col, 1.0f, 1.0f, ++vert_id, Vertex_state::STATE_HOLD};
                add.push_back(v1);
                add.push_back(v2);
                break;
            } else if (vert.state & Vertex_state::STATE_HOVER_ONLY && !(vert.state & Vertex_state::STATE_HOLD)) {
                Vertex_pcs v1{vert.pos, vert.col, 1.0f, 0.0f, ++vert_id, Vertex_state::STATE_NONE};
                Vertex_pcs v2{vert.pos, vert.col, 1.01f, 1.0f, ++vert_id, Vertex_state::STATE_HOLD};
                add.push_back(v1);
                add.push_back(v2);
                break;
            }
        }
        verts.insert(verts.end(), add.begin(), add.end());
    }
    if (ctx.input->get_key_down(input::Key_code::R)) {
        for (auto& vert : verts) {
            if (vert.state & Vertex_state::STATE_HOVER_WIRE) {
                size_t idx_0 = &vert - verts.data(); // 포인터 차이로 인덱스 구하기
                size_t idx_1 = (idx_0 % 2 == 0) ? idx_0 + 1 : idx_0 - 1;
                std::swap(verts[idx_0].pos, verts[idx_1].pos);
                break;
            }
        }
    }

    // snap
    Vec3  cam_fwd = ctx.scene->camera().data().transform.forward; // 카메라가 보는 방향
    float nx = std::abs(cam_fwd.x_);
    float ny = std::abs(cam_fwd.y_);
    float nz = std::abs(cam_fwd.z_);
    // 가장 절댓값이 큰 축의 평면을 사용
    if (nz >= nx && nz >= ny) {
        edit_mode_ = Edit_mode::Plane_xy;
    } else if (nx >= ny && nx >= nz) {
        edit_mode_ = Edit_mode::Plane_yz;
    } else {
        edit_mode_ = Edit_mode::Plane_zx;
    }

    // hold
    if (ctx.input->get_key_down(input::Key_code::LButton)) {
        for (auto& vert : verts) {
            if (vert.state & Vertex_state::STATE_HOVER_ONLY) {
                vert.state |= Vertex_state::STATE_HOLD;
                cur_intersection = vert.pos.xyz();
                pre_snapped_pos = get_snap_pos(snap_interval, ray.ori, ray.dir, vert.pos.xyz(), edit_mode_);
            } else if (vert.state & Vertex_state::STATE_HOVER_WIRE) {
                size_t idx_0 = &vert - verts.data(); // 포인터 차이로 인덱스 구하기
                size_t idx_1 = (idx_0 % 2 == 0) ? idx_0 + 1 : idx_0 - 1;
                size_t first = std::min(idx_0, idx_1);
                verts[first].state |= Vertex_state::STATE_HOLD;
                verts[first + 1].state |= Vertex_state::STATE_HOLD; // 다음 인덱스 정점이 짝이므로 HOLD

                cur_intersection = closest_points_to_line.dst_point;
                pre_snapped_pos = get_snap_pos(snap_interval, ray.ori, ray.dir,
                                               math::closest_points_to_line(ray.ori, ray.dir, vert.pos.xyz(),
                                                                            (verts[first + 1].pos - vert.pos).xyz())
                                                   .dst_point,
                                               edit_mode_);

                for (auto& closest_vert : verts) { // O(N^2) 성능 저하 가능성 있음
                    if ((verts[first].pos - closest_vert.pos).LengthSquared() < 1.0f) {
                        closest_vert.state |= Vertex_state::STATE_HOLD;
                    }
                    if ((verts[first + 1].pos - closest_vert.pos).LengthSquared() < 1.0f) {
                        closest_vert.state |= Vertex_state::STATE_HOLD;
                    }
                }
                break; // wire는 하나만 hold
            }
        }
    }
    // dragging
    {
        Vec3 cur_snapped_pos = get_snap_pos(snap_interval, ray.ori, ray.dir, cur_intersection, edit_mode_);
        Vec3 delet_snapped_pos = cur_snapped_pos - pre_snapped_pos;
        for (auto& vert : verts) {
            if (vert.state & Vertex_state::STATE_HOLD) {
                vert.pos += Vec4{delet_snapped_pos.x_, delet_snapped_pos.y_, delet_snapped_pos.z_, 0.0f};
            }
        }
        pre_snapped_pos = cur_snapped_pos;
    }

    // hold off
    if (ctx.input->get_key_up(input::Key_code::LButton)) {
        for (auto& vert : verts) {
            if (vert.state & Vertex_state::STATE_HOLD) {
                vert.state &= ~Vertex_state::STATE_HOLD;
            }
        }
        pre_snapped_pos = Vec3::Zero;
        cur_intersection = Vec3::Zero;
    }

    // end
    std::erase_if(verts, [](const Vertex_pcs& target) { return target.state & Vertex_state::STATE_DESTROYED; });

    // update
    // uint32_t buffer_id, size_t cnt, const void* wire_nodes_data
    ctx.msg_queue->Push_message<scene::Wire_buffer_update_message>(
        ctx.scene->data().wire_properites.wire_nodes.vb.buf_.id_,
        sizeof(Vertex_pcs) * ctx.scene->data().wire_properites.wire_nodes.vb.vertices.size(),
        ctx.scene->data().wire_properites.wire_nodes.vb.vertices.data());
}

auto Editor::get_snap_pos(float interval, const math::Vec3& ray_ori, const math::Vec3& ray_dir,
                          const math::Vec3& vert_pos, Edit_mode mode) -> math::Vec3
{
    float t{0.0f};
    Vec3  intersection;
    if (edit_mode_ == Edit_mode::Plane_xy) { // Z축이 법선 (Z = 0 평면)
        t = (vert_pos.z_ - ray_ori.z_) / ray_dir.z_;
        intersection = ray_ori + ray_dir * t;
    } else if (edit_mode_ == Edit_mode::Plane_yz) { // X축이 법선 (X = 0 평면)
        t = (vert_pos.x_ - ray_ori.x_) / ray_dir.x_;
        intersection = ray_ori + ray_dir * t;
    } else if (edit_mode_ == Edit_mode::Plane_zx) { // Y축이 법선 (Y = 0 평면)
        t = (vert_pos.y_ - ray_ori.y_) / ray_dir.y_;
        intersection = ray_ori + ray_dir * t;
    }
    // 평면 intersect 지점을 기준으로 offset 생성
    float snapped_x = std::round((intersection.x_) / interval) * interval;
    float snapped_y = std::round((intersection.y_) / interval) * interval;
    float snapped_z = std::round((intersection.z_) / interval) * interval;

    return Vec3{snapped_x, snapped_y, snapped_z};
}
} // namespace seop::editor