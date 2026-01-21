#include "device.hpp"
#include "compute_shaders/electro_cs_shader.hpp"
#include "compute_shaders/gravity_cs_source.hpp"
#include "compute_shaders/static_magnetic_field_cs_shader.hpp"
#include "shaders/grid_shader.hpp"
#include "shaders/particle_shader.hpp"
#include "shaders/quad_shader.hpp"
#include "shaders/wire_shader.hpp"

#include "seop_entity/particle.hpp"
#include "seop_input/input.hpp"
#include "seop_input/keys.hpp"
#include "seop_math/math.hpp"
#include "seop_message/message.hpp"
#include "seop_raytrace/ray.hpp"
#include "seop_scene/camera.hpp"
#include "seop_scene/scene.hpp"
#include "seop_scene/scene_message.hpp"
#include "seop_window/glf_window.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

namespace seop::graphic
{
using namespace math;
using namespace primitive;

Device::Device()
{
}

void Device::init(Context& ctx)
{
    glewInit();
    init_draw_propeties();

    ctx.msg_queue->Register_handler<scene::Particle_change_message>(
        [this](scene::Particle_change_message& msg) -> bool { return on_message(msg); });

    add_shader_buffer(Shader_buffer_type::Particle);
    add_shader_buffer(Shader_buffer_type::Conductor_wire);

    add_compute_task(
        Shader_task_type::Compute_electromagnetic, electromagnetic_cs_source, [](uint32_t program, Context& ctx) {
            glUseProgram(program);
            glUniform1f(glGetUniformLocation(program, "u_dt"), ctx.f_dt);
            glUniform1f(glGetUniformLocation(program, "u_time_scale"),
                        ctx.scene->data().particle_properties.time_scale);
            glUniform1ui(glGetUniformLocation(program, "u_particle_count"),
                         (uint32_t)ctx.scene->data().particle_properties.count);
            glUniform1f(glGetUniformLocation(program, "u_particle_col"), ctx.scene->data().particle_properties.col);
        });
    add_compute_task(
        Shader_task_type::Compute_static_magnetic_field, static_magnetic_field_cs_source,
        [this](uint32_t program, Context& ctx) {
            glUseProgram(program);
            glUniform1ui(glGetUniformLocation(program, "u_particle_count"),
                         (uint32_t)ctx.scene->data().particle_properties.count);
            glUniform1ui(glGetUniformLocation(program, "u_wire_count"), (uint32_t)test_wires_.vb.vertices.size() / 2);
            glUniform1f(glGetUniformLocation(program, "u_dt"), ctx.f_dt);
            glUniform1f(glGetUniformLocation(program, "u_time"), static_cast<float>(ctx.time));
            glUniform1f(glGetUniformLocation(program, "u_time_scale"),
                        ctx.scene->data().particle_properties.time_scale);
            glUniform1f(glGetUniformLocation(program, "u_particle_col"), ctx.scene->data().particle_properties.col);
            glUniform1f(glGetUniformLocation(program, "u_w"), ctx.scene->data().wire_properites.w);
            glUniform1f(glGetUniformLocation(program, "u_max_i"), ctx.scene->data().wire_properites.max_i);
        });
    add_compute_task(Shader_task_type::Compute_gravity, electromagnetic_cs_source, [](uint32_t program, Context& ctx) {
        glUseProgram(program);
        glUniform3fv(glGetUniformLocation(program, "u_ray_point"), 1, &ctx.input->data().ray_point.x_);
        glUniform1f(glGetUniformLocation(program, "u_gravity"), ctx.scene->data().forces.gravity);
        glUniform1f(glGetUniformLocation(program, "u_orbit_force"),
                    ctx.scene->data().forces.gravity * ctx.scene->data().forces.vortex);
        glUniform1f(glGetUniformLocation(program, "u_damping"), ctx.scene->data().forces.damping);
        glUniform1f(glGetUniformLocation(program, "u_magentic_str"), ctx.scene->data().forces.magentic_str);
        glUniform1f(glGetUniformLocation(program, "u_dt"), ctx.f_dt);
        glUniform1f(glGetUniformLocation(program, "u_particle_col"), ctx.scene->data().particle_properties.col);
        glUniform1ui(glGetUniformLocation(program, "u_particle_count"),
                     (uint32_t)ctx.scene->data().particle_properties.count);
        glUniform1ui(glGetUniformLocation(program, "u_attractor_count"),
                     (uint32_t)ctx.scene->data().attractor_properties.attractor_count);
    });
    add_render_task(Shader_task_type::Render_screen_quad, quad_vs_source, quad_fs_source,
                    [this](uint32_t program, Context& ctx) {
                        glUseProgram(program);
                        if (uint32_t tex = frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].use_tex) {
                            glUniform1i(glGetUniformLocation(program, "use_texture"), 1);
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, tex);
                            glUniform1i(glGetUniformLocation(program, "u_screen_texture"), 0);
                        } else {
                            glUniform1i(glGetUniformLocation(program, "use_texture"), 0);
                            glUniform4f(glGetUniformLocation(program, "u_color"), 0.0f, 0.0f, 0.0f, data_.fade_scale);
                        }
                    });
    add_render_task(Shader_task_type::Render_grid, grid_vs_source, grid_fs_source, [](uint32_t program, Context& ctx) {
        glUseProgram(program);

        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,
                           (float*)&ctx.scene->camera().data().view);
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE,
                           (float*)&ctx.scene->camera().data().projection);
    });

    add_render_task(Shader_task_type::Render_particle, particle_vs_source, particle_fs_source,
                    [](uint32_t program, Context& ctx) {
                        glUseProgram(program);
                        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,
                                           (float*)&ctx.scene->camera().data().view);
                        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE,
                                           (float*)&ctx.scene->camera().data().projection);
                    });

    add_frame_buffer(Frame_buffer_type::Particle_layer, static_cast<int>(ctx.window->client_size().x_),
                     static_cast<int>(ctx.window->client_size().y_));

    add_render_task(Shader_task_type::Render_wire, wire_vs_source, wire_fs_source, [](uint32_t program, Context& ctx) {
        glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,
                           (float*)&ctx.scene->camera().data().view);
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE,
                           (float*)&ctx.scene->camera().data().projection);
        glUniform1f(glGetUniformLocation(program, "u_time"), static_cast<float>(ctx.time));
    });
    create_grid();
    create_screen_qaud();

    update_shader_buffer(shader_buffers_[to_idx(Shader_buffer_type::Particle)].id, 0,
                         sizeof(entity::Particle) * ctx.scene->data().particle_properties.count,
                         ctx.scene->data().entities.particles.data());

    // test
    create_wire();
    update_shader_buffer(shader_buffers_[to_idx(Shader_buffer_type::Conductor_wire)].id, 1,
                         sizeof(Vertex_pcs) * test_wires_.vb.vertices.size(), test_wires_.vb.vertices.data());
}

void Device::create_screen_qaud()
{
    screen_quad_.vb.vertices.push_back(
        Vertex_pu{Vec4{-1.0f, 1.0f, 0.0f, 1.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f}}); // pos, uv
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{-1.0f, -1.0f, 0.0f, 1.0f}, Vec4{0.0f, 0.0f, 0.0f, 0.0f}});
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{1.0f, -1.0f, 0.0f, 1.0f}, Vec4{1.0f, 0.0f, 0.0f, 0.0f}});
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{-1.0f, 1.0f, 0.0f, 1.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f}});
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{1.0f, -1.0f, 0.0f, 1.0f}, Vec4{1.0f, 0.0f, 0.0f, 0.0f}});
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{1.0f, 1.0f, 0.0f, 1.0f}, Vec4{1.0f, 1.0f, 0.0f, 0.0f}});

    glGenVertexArrays(1, &screen_quad_.va.id);
    glGenBuffers(1, &screen_quad_.vb.id);
    glBindVertexArray(screen_quad_.va.id);
    glBindBuffer(GL_ARRAY_BUFFER, screen_quad_.vb.id);
    glBufferData(GL_ARRAY_BUFFER, screen_quad_.vb.vertices.size() * sizeof(Vertex_pu), screen_quad_.vb.vertices.data(),
                 GL_STATIC_DRAW);

    // 위치 속성 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_pu), (void*)0);
    // 텍스처 좌표 속성 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_pu), (void*)16);

    glBindVertexArray(0); // 설정 완료 후 언바인딩
}

/*------------------
| 3               2 |
|                   |
|                   |
| 0               1 |
 ------------------*/
void Device::create_grid()
{
    grid_quad_.vb.vertices.push_back(
        Vertex_pc{Vec4{-1.0f, 0.0f, 1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}}); // pos, col
    grid_quad_.vb.vertices.push_back(Vertex_pc{Vec4{1.0f, 0.0f, 1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}});
    grid_quad_.vb.vertices.push_back(Vertex_pc{Vec4{1.0f, 0.0f, -1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}});
    grid_quad_.vb.vertices.push_back(Vertex_pc{Vec4{-1.0f, 0.0f, -1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}});

    glGenVertexArrays(1, &grid_quad_.va.id);
    glGenBuffers(1, &grid_quad_.vb.id);
    glBindVertexArray(grid_quad_.va.id);
    glBindBuffer(GL_ARRAY_BUFFER, grid_quad_.vb.id);
    glBufferData(GL_ARRAY_BUFFER, grid_quad_.vb.vertices.size() * sizeof(Vertex_pc), grid_quad_.vb.vertices.data(),
                 GL_STATIC_DRAW);

    // 위치 속성 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_pc), (void*)0);
    // 색상 속성 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_pc), (void*)16);

    glBindVertexArray(0); // 설정 완료 후 언바인딩
}

auto Device::on_message(msg::Message& msg) -> bool
{
    msg::Message_dispatcher dispatcher(msg);
    dispatcher.Dispatch<scene::Particle_change_message>([this](scene::Particle_change_message& msg) -> bool {
        update_shader_buffer(shader_buffers_[to_idx(Shader_buffer_type::Particle)].id, 0,
                             sizeof(entity::Particle) * msg.particle_count(), msg.particle_data());
        return true;
    });

    return true;
}

void Device::compute(Context& ctx)
{
    if (data_.compute_type & Compute_type::Gravity) {
        shader_tasks_[to_idx(Shader_task_type::Compute_gravity)].uniform_setter(ctx);
    }
    if (data_.compute_type & Compute_type::Electromagnetic) {
        shader_tasks_[to_idx(Shader_task_type::Compute_electromagnetic)].uniform_setter(ctx);
    }
    if (data_.compute_type & Compute_type::Time_varying_EM_field) {
        shader_tasks_[to_idx(Shader_task_type::Compute_static_magnetic_field)].uniform_setter(ctx);
    }
    // 실행
    uint32_t num_groups = static_cast<uint32_t>((ctx.scene->data().particle_properties.count + 255) / 256);
    glDispatchCompute(num_groups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Device::render(Context& ctx)
{
    // temp
    auto size = ctx.window->client_size();
    glViewport(0, 0, (int)size.x_, (int)size.y_);
    raytrace(ctx);
    // pipeline 1
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].buf_id);
    glDisable(GL_DEPTH_TEST);

    if (ctx.scene->camera().camera_state().is_move) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    } else {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].use_tex = false;
        prepare_quad(ctx); // fade_alpha가 적용된 검은 쿼드
        draw_quad();
    }

    // pipeline 2
    glEnable(GL_BLEND);                // 블렌딩 활성화
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 입자들이 겹치면 더 밝아짐 (불꽃, 네온 효과)
    prepare_particle(ctx);
    draw_particle(ctx.scene->data().particle_properties.count);

    // pipeline 3
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(data_.back_col.x_, data_.back_col.y_, data_.back_col.z_, data_.back_col.w_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_BLEND);
    prepare_grid(ctx);
    draw_grid();

#if 1
    // test
    glLineWidth(test_wires_.line_width);
    shader_tasks_[to_idx(Shader_task_type::Render_wire)].uniform_setter(ctx);
    glBindVertexArray(test_wires_.va.id);
    glDrawArrays(GL_LINES, 0, (uint32_t)test_wires_.vb.vertices.size());
    glDrawArrays(GL_POINTS, 0, (uint32_t)test_wires_.vb.vertices.size());
#endif

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].use_tex = true;
    prepare_quad(ctx);
    draw_quad();
}

void Device::prepare_grid(Context& ctx)
{
    shader_tasks_[to_idx(Shader_task_type::Render_grid)].uniform_setter(ctx);
}

void Device::draw_grid()
{
    glBindVertexArray(grid_quad_.va.id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(2);
}

void Device::prepare_particle(Context& ctx)
{
    glPointSize(ctx.scene->data().particle_properties.size);

    shader_tasks_[to_idx(Shader_task_type::Render_particle)].uniform_setter(ctx);

    glBindBuffer(GL_ARRAY_BUFFER, shader_buffers_[to_idx(Shader_buffer_type::Particle)].id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(entity::Particle), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(entity::Particle), (void*)(16));
}

void Device::prepare_quad(Context& ctx)
{
    shader_tasks_[to_idx(Shader_task_type::Render_screen_quad)].uniform_setter(ctx);
}

void Device::draw_particle(size_t cnt)
{
    // 4. 그리기
    glDrawArrays(GL_POINTS, 0, (GLsizei)cnt);

    // 5. 정리
    glDisableVertexAttribArray(0);
}

void Device::draw_quad()
{

    // 우리가 만든 VAO 바인딩 후 그리기
    glBindVertexArray(screen_quad_.va.id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}

void Device::clear_frame()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(data_.back_col.x_, data_.back_col.y_, data_.back_col.z_, data_.back_col.w_);
}

void Device::init_draw_propeties()
{
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_BLEND);                // 블렌딩 활성화
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 입자들이 겹치면 더 밝아짐 (불꽃, 네온 효과)
    glEnable(GL_PROGRAM_POINT_SIZE);   // 셰이더에서 입자 크기 조절 가능
}

uint32_t Device::create_cs(const char* src)
{
    uint32_t cs_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(cs_shader, 1, &src, NULL);
    glCompileShader(cs_shader);

#if defined(_DEBUG) || defined(DEBUG)
    GLint success;
    glGetShaderiv(cs_shader, GL_COMPILE_STATUS, &success);
#endif

    return cs_shader;
}

uint32_t Device::create_vs(const char* src)
{
    uint32_t vs_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs_shader, 1, &src, NULL);
    glCompileShader(vs_shader);
    return vs_shader;
}

uint32_t Device::create_fs(const char* src)
{
    uint32_t fs_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_shader, 1, &src, NULL);
    glCompileShader(fs_shader);
    return fs_shader;
}

void Device::add_compute_task(Shader_task_type type, const char* cs_src,
                              std::function<void(uint32_t, Context&)> uniform_setter)
{
    uint32_t program = glCreateProgram();
    uint32_t cs = create_cs(cs_src);
    glAttachShader(program, cs);
    glLinkProgram(program);

    shader_tasks_[to_idx(type)] =
        Shader_task{program, [program, uniform_setter](Context& ctx) { uniform_setter(program, ctx); }};
    glDeleteShader(cs); // 링크 후 셰이더 정리
}

void Device::create_wire()
{
    test_wires_.vb.vertices.push_back(Vertex_pcs{Vec4{-100.0f, 0.0f, 0.0f, 1.0f}, Vec4{1.0f, 1.0f, 0.0f, 1.0f}, 1.0f,
                                                 0.0f, ++primitive::vert_id, Vertex_state::STATE_NONE});
    test_wires_.vb.vertices.push_back(Vertex_pcs{Vec4{-300.0f, 300.0f, 0.0f, 1.0f}, Vec4{1.0f, 1.0f, 0.0f, 1.0f}, 1.0f,
                                                 1.0f, ++primitive::vert_id, Vertex_state::STATE_NONE});

    glGenVertexArrays(1, &test_wires_.va.id);
    glGenBuffers(1, &test_wires_.vb.id);
    glBindVertexArray(test_wires_.va.id);
    glBindBuffer(GL_ARRAY_BUFFER, test_wires_.vb.id);
    glBufferData(GL_ARRAY_BUFFER, test_wires_.vb.vertices.size() * sizeof(Vertex_pcs), test_wires_.vb.vertices.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_pcs), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_pcs), (void*)16);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex_pcs), (void*)32);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex_pcs), (void*)36);

    glBindVertexArray(0);
}

void Device::add_wire()
{
    test_wires_.vb.vertices.push_back(Vertex_pcs{Vec4{0.0f, -100.0f, 0.0f, 1.0f}, Vec4{1.0f, 1.0f, 0.0f, 1.0f}, 1.0f,
                                                 0.0f, ++primitive::vert_id, Vertex_state::STATE_NONE});
    test_wires_.vb.vertices.push_back(Vertex_pcs{Vec4{0.0f, 100.0f, 0.0f, 1.0f}, Vec4{1.0f, 1.0f, 0.0f, 1.0f}, 1.0f,
                                                 1.0f, ++primitive::vert_id, Vertex_state::STATE_NONE});

    update_shader_buffer(test_wires_.vb.id, 1, test_wires_.vb.vertices.size() * sizeof(primitive::Vertex_pcs),
                         test_wires_.vb.vertices.data());

    update_vertex_buffer(test_wires_.vb.id, sizeof(Vertex_pcs) * test_wires_.vb.vertices.size(),
                         test_wires_.vb.vertices.data());
}

auto Device::make_ray(const math::Vec2& ndc_pos, const math::Vec3& cam_pos, const math::Matrix& view,
                      const math::Matrix& projection) -> raytrace::Ray
{
    Vec4   clip_space{ndc_pos.x_, ndc_pos.y_, -1.0f, 1.0f};
    Matrix view_inv = view.Invert();
    Matrix projection_inv = projection.Invert();

    Vec4   view_space = projection_inv * clip_space;
    view_space.z_ = -1.0f; // 오른손 좌표계
    view_space.w_ = 0.0f;  // 방향 벡터이므로 w를 0으로 설정
    Vec3 ray_ori = cam_pos;
    Vec3 ray_dir = (view_inv * view_space).xyz().Normalized();

    return raytrace::Ray{ray_ori, ray_dir};
}

void Device::raytrace(Context& ctx)
{
    // ndc 좌표 변환
    if (ctx.input->get_key_down(input::Key_code::O)) {
        int a = 0;
    }
    Vec2                cursor_ndc = ctx.window->get_cursor_pos_ndc();
    scene::Camera_data& cam_data = ctx.scene->camera().data();

    raytrace::Ray       ray = make_ray(cursor_ndc, cam_data.transform.pos, cam_data.view, cam_data.projection);

    std::vector<primitive::Vertex_pcs>& verts = test_wires_.vb.vertices;

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
        std::vector<primitive::Vertex_pcs> add;
        for (auto& vert : verts) {
            if (vert.state & Vertex_state::STATE_HOVER_WIRE) {
                size_t     idx_0 = &vert - verts.data(); // 포인터 차이로 인덱스 구하기
                size_t     idx_1 = (idx_0 % 2 == 0) ? idx_0 + 1 : idx_0 - 1;
                Vertex_pcs v1{verts[idx_0].pos,     verts[idx_0].col,        1.0f, 0.0f,
                              ++primitive::vert_id, Vertex_state::STATE_HOLD};
                Vertex_pcs v2{verts[idx_1].pos,     verts[idx_1].col,        1.0f, 1.0f,
                              ++primitive::vert_id, Vertex_state::STATE_HOLD};
                add.push_back(v1);
                add.push_back(v2);
                break;
            } else if (vert.state & Vertex_state::STATE_HOVER_ONLY) { // 한 정점에서 새로운 wire 추가
                Vertex_pcs v1{vert.pos, vert.col, 1.0f, 0.0f, ++primitive::vert_id, Vertex_state::STATE_NONE};
                Vertex_pcs v2{vert.pos, vert.col, 1.0f, 1.0f, ++primitive::vert_id, Vertex_state::STATE_HOLD};
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

    // update
    update_shader_buffer(test_wires_.vb.id, 1, verts.size() * sizeof(primitive::Vertex_pcs), verts.data());
    update_vertex_buffer(test_wires_.vb.id, sizeof(Vertex_pcs) * verts.size(), verts.data());

    // end
    std::erase_if(verts, [](const Vertex_pcs& target) { return target.state & Vertex_state::STATE_DESTROYED; });
}

auto Device::get_snap_pos(float interval, const math::Vec3& ray_ori, const math::Vec3& ray_dir,
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

void Device::add_shader_buffer(Shader_buffer_type type)
{
    Shader_buffer buf;
    glGenBuffers(1, &buf.id);
    shader_buffers_[to_idx(type)] = buf;
}

void Device::add_render_task(Shader_task_type type, const char* vs_src, const char* fs_src,
                             std::function<void(uint32_t, Context&)> uniform_setter)
{
    uint32_t vs = create_vs(vs_src);
    uint32_t fs = create_fs(fs_src);
    uint32_t program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    shader_tasks_[to_idx(type)] =
        Shader_task{program, [program, uniform_setter](Context& ctx) { uniform_setter(program, ctx); }};
    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Device::add_frame_buffer(Frame_buffer_type type, int width, int height)
{
    Frame_buffer target;
    glGenFramebuffers(1, &target.buf_id);
    glBindFramebuffer(GL_FRAMEBUFFER, target.buf_id);

    // 1. 입자를 담을 텍스처 생성
    glGenTextures(1, &target.tex_id);
    glBindTexture(GL_TEXTURE_2D, target.tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 2. FBO에 텍스처 연결
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.tex_id, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    frame_buffers_[to_idx(type)] = target;
}

void Device::update_frame_buffer(Frame_buffer_type type, int width, int height)
{
    Frame_buffer& target = frame_buffers_[to_idx(type)];

    glBindTexture(GL_TEXTURE_2D, target.tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#if _DEBUG
    // 4. FBO 상태 체크 (선택 사항이지만 디버깅에 매우 유리함)
    glBindFramebuffer(GL_FRAMEBUFFER, target.buf_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.tex_id, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO Resize Failed! Status: " << status << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void Device::update_shader_buffer(uint32_t buffer_id, uint32_t binding_point, GLsizeiptr buffer_size, const void* data)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);

    glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, data, GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, buffer_id);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Device::update_vertex_buffer(uint32_t buffer_id, GLsizeiptr buffer_size, const void* data)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);

    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, data);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

auto Device::data() const -> const Device_data&
{
    return data_;
}

auto Device::data() -> Device_data&
{
    return data_;
}

void Device::set_fade_scale(float scale)
{
    data_.fade_scale = scale;
}

void Device::set_frame_rate(float rate)
{
    data_.frame_rate = rate;
}

void Device::set_back_col(const math::Vec4& col)
{
    data_.back_col = col;
}

void Device::set_compute_type(Compute_type type)
{
    data_.compute_type = type;
}

void Device::shut_down()
{
}

} // namespace seop::graphic
