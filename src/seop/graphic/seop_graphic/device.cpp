#include "device.hpp"
#include "shader_program.hpp"

#include "seop_input/input.hpp"
#include "seop_input/keys.hpp"
#include "seop_item/wire.hpp"
#include "seop_math/math.hpp"
#include "seop_message/message.hpp"
#include "seop_opengl/shader.hpp"
#include "seop_scene/camera.hpp"
#include "seop_scene/scene.hpp"
#include "seop_scene/scene_message.hpp"
#include "seop_util/util.hpp"
#include "seop_window/glf_window.hpp"
#if _DEBUG
#include <filesystem>
#endif

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace seop::graphic
{
using namespace math;
using namespace primitive;
using namespace util;

Device::Device()
{
}

void Device::init(Context& ctx)
{
#if _DEBUG
    std::cout << "Current Working Directory: " << std::filesystem::current_path() << std::endl;
#endif
    glewInit();
    glEnable(GL_POINT_SPRITE);       // 입자 텍스쳐 좌표
    glEnable(GL_PROGRAM_POINT_SIZE); // 셰이더에서 입자 크기 조절 가능

    init_arrow();
    init_grid();
    init_screen_qaud();
    init_wire(ctx);
    init_particle(ctx);
    set_shader_task(ctx);
    set_frame_buffer(Frame_buffer_type::Particle_layer, static_cast<int>(ctx.window->client_size().x_),
                     static_cast<int>(ctx.window->client_size().y_));

    ctx.msg_queue->Register_handler<scene::Particle_buffer_update_message>(
        [this](scene::Particle_buffer_update_message& msg) -> bool { return on_message(msg); });
    ctx.msg_queue->Register_handler<scene::Wire_buffer_update_message>(
        [this](scene::Wire_buffer_update_message& msg) -> bool { return on_message(msg); });

    ctx.scene->data().entities.particles.vb.use_shader_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW,
                                                              (uint32_t)Shader_buffer_slot::Particle);

    ctx.scene->data().wire_properites.wire_nodes.vb.use_shader_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW,
                                                                      (uint32_t)Shader_buffer_slot::Wire);

    arrow_nodes_.vb.use_shader_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, (uint32_t)Shader_buffer_slot::Arrow);
}

void Device::update(Context& ctx)
{ // hot load
    if (ctx.input->get_key_down(input::Key_code::O)) {
        hot_load_shader(ctx);
    }
}

void Device::init_screen_qaud()
{
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{-1.0f, 1.0f, 0.0f, 1.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f}});
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{-1.0f, -1.0f, 0.0f, 1.0f}, Vec4{0.0f, 0.0f, 0.0f, 0.0f}});
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{1.0f, -1.0f, 0.0f, 1.0f}, Vec4{1.0f, 0.0f, 0.0f, 0.0f}});
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{-1.0f, 1.0f, 0.0f, 1.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f}});
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{1.0f, -1.0f, 0.0f, 1.0f}, Vec4{1.0f, 0.0f, 0.0f, 0.0f}});
    screen_quad_.vb.vertices.push_back(Vertex_pu{Vec4{1.0f, 1.0f, 0.0f, 1.0f}, Vec4{1.0f, 1.0f, 0.0f, 0.0f}});

    screen_quad_.create();
    screen_quad_.bind(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
}

/*------------------
| 3               2 |
|                   |
|                   |
| 0               1 |
 ------------------*/
void Device::init_grid()
{
    grid_quad_.vb.vertices.push_back(Vertex_pc{Vec4{-1.0f, 0.0f, 1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}});
    grid_quad_.vb.vertices.push_back(Vertex_pc{Vec4{1.0f, 0.0f, 1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}});
    grid_quad_.vb.vertices.push_back(Vertex_pc{Vec4{1.0f, 0.0f, -1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}});
    grid_quad_.vb.vertices.push_back(Vertex_pc{Vec4{-1.0f, 0.0f, -1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}});

    grid_quad_.create();
    grid_quad_.bind(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
}

void Device::init_particle(Context& ctx)
{
    item::Particle& partiles = ctx.scene->data().entities.particles;
    partiles.create();
    partiles.bind(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
}

void Device::hot_load_shader(Context& ctx)
{
    set_shader_task(ctx);
}

auto Device::on_message(msg::Message& msg) -> bool
{
    msg::Message_dispatcher dispatcher(msg);
    dispatcher.Dispatch<scene::Particle_buffer_update_message>(
        [this](scene::Particle_buffer_update_message& msg) -> bool {
            update_shader_buffer(msg.buffer_id_, 0, msg.buffer_size_, msg.particle_data_);
            return true;
        });
    dispatcher.Dispatch<scene::Wire_buffer_update_message>([this](scene::Wire_buffer_update_message& msg) -> bool {
        update_shader_buffer(msg.buffer_id_, 1, msg.buffer_size_, msg.wire_nodes_data_);
        return true;
    });

    return true;
}

void Device::compute(Context& ctx)
{

    shader_programs_[to_idx(Shader_program_type::Compute_arrow_field)].set_uniform(ctx);

    shader_programs_[to_idx(Shader_program_type::Compute_arrow_field)].compute(
        static_cast<uint32_t>((arrow_nodes_.vb.vertices.size() + 255) / 256), 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    if (data_.compute_type & Compute_type::Time_varying_EM_field) {
        shader_programs_[to_idx(Shader_program_type::Compute_static_magnetic_field)].set_uniform(ctx);
    } else if (data_.compute_type & Compute_type::Lorenz_equation) {
        shader_programs_[to_idx(Shader_program_type::Compute_lorenz_equation)].set_uniform(ctx);
    }
    shader_programs_[to_idx(Shader_program_type::Compute_arrow_field)].compute(
        static_cast<uint32_t>((ctx.scene->data().entities.particles.vb.vertices.size() + 255) / 256), 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Device::render(Context& ctx)
{

    glViewport(0, 0, (int)ctx.window->client_size().x_, (int)ctx.window->client_size().y_);

    // pipeline 1 : 잔상 프레임 버퍼 텍스처 생성
    frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].bind(GL_FRAMEBUFFER);

    if (ctx.scene->camera().camera_state().is_move) {
        glClear(GL_COLOR_BUFFER_BIT);
    } else {
        prepare_screen_quad(ctx, false); // fade_alpha가 적용된 검은 쿼드
        draw_screen_quad();
    }
    prepare_particle(ctx);
    draw_particle(ctx);

    // pipeline 2
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // prepare_screen_quad(ctx, true);
    // draw_screen_quad();

    prepare_grid(ctx);
    draw_grid();

    prepare_wire(ctx);
    draw_wire(ctx);

    // temp
    prepare_arrow(ctx);
    draw_arrow();
}

void Device::prepare_arrow(Context& ctx)
{
    glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
    glDepthMask(GL_TRUE);    // 깊이 값을 기록함
    glDepthFunc(GL_LESS);    // 가까운 것이 보이게 설정
    glLineWidth(arrow_thickness);
    shader_programs_[to_idx(Shader_program_type::Render_arrow)].set_uniform(ctx);
}

void Device::draw_arrow()
{
    glBindVertexArray(arrow_nodes_.va.id_);
    glDrawArrays(GL_LINES, 0, (uint32_t)arrow_nodes_.vb.vertices.size()); // 노드
    glBindVertexArray(0);
}

void Device::prepare_grid(Context& ctx)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader_programs_[to_idx(Shader_program_type::Render_grid)].set_uniform(ctx);
}

void Device::draw_grid()
{
    glBindVertexArray(grid_quad_.va.id_);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void Device::prepare_particle(Context& ctx)
{
    glEnable(GL_BLEND); // 블렌딩 활성화
    if (data_.back_col.LengthSquared() < 2.0f) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 입자들이 겹치면 더 밝아짐 (불꽃, 네온 효과)
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    glPointSize(ctx.scene->data().particle_properties.size);
    shader_programs_[to_idx(Shader_program_type::Render_particle)].set_uniform(ctx);
}

void Device::prepare_wire(Context& ctx)
{
    glLineWidth(ctx.scene->data().wire_properites.wire_nodes.line_width);
    shader_programs_[to_idx(Shader_program_type::Render_wire)].set_uniform(ctx);
}

void Device::draw_particle(Context& ctx)
{
    glBindVertexArray(ctx.scene->data().entities.particles.va.id_);
    glDrawArrays(GL_POINTS, 0, (GLsizei)ctx.scene->data().entities.particles.vb.vertices.size());
    glBindVertexArray(0);
}

void Device::draw_wire(Context& ctx)
{
    glBindVertexArray(ctx.scene->data().wire_properites.wire_nodes.va.id_);
    glDrawArrays(GL_LINES, 0, (uint32_t)ctx.scene->data().wire_properites.wire_nodes.vb.vertices.size());  // 노드
    glDrawArrays(GL_POINTS, 0, (uint32_t)ctx.scene->data().wire_properites.wire_nodes.vb.vertices.size()); // 와이어
    glBindVertexArray(0);
}

void Device::prepare_screen_quad(Context& ctx, bool use_tex)
{
    glEnable(GL_BLEND);
    if (use_tex) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].use_tex = true;
        glBindTexture(GL_TEXTURE_2D, frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].tex_.id_);
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].use_tex = false;
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    shader_programs_[to_idx(Shader_program_type::Render_screen_quad)].set_uniform(ctx);
}

void Device::draw_screen_quad()
{

    // 우리가 만든 VAO 바인딩 후 그리기
    screen_quad_.va.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Device::clear_frame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(data_.back_col.x_, data_.back_col.y_, data_.back_col.z_, data_.back_col.w_);
}

void Device::init_arrow()
{
    arrow_nodes_.create();
    arrow_nodes_.bind(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
}

void Device::create_arrow(int interval, int x_min, int x_max, int y_min, int y_max, int z_min, int z_max)
{
    arrow_nodes_.vb.vertices.clear();
    arrow_nodes_.vb.vertices.reserve((x_max / interval - x_min / interval + 1) *
                                     (y_max / interval - y_min / interval + 1) *
                                     (z_max / interval - z_min / interval + 1) * 6);
    for (int x = x_min / interval; x <= x_max / interval; x++) {
        for (int y = y_min / interval; y <= y_max / interval; y++) {
            for (int z = z_min / interval; z <= z_max / interval; z++) {
                // 전기장
                arrow_nodes_.vb.vertices.push_back(
                    Vertex_pf{Vec4{x * interval, y * interval, z * interval, 1}, Vec4::Zero});
                arrow_nodes_.vb.vertices.push_back(
                    Vertex_pf{Vec4{x * interval, y * interval, z * interval, 1}, Vec4::Zero});
                // 자기장
                arrow_nodes_.vb.vertices.push_back(
                    Vertex_pf{Vec4{x * interval, y * interval, z * interval, 1}, Vec4::Zero});
                arrow_nodes_.vb.vertices.push_back(
                    Vertex_pf{Vec4{x * interval, y * interval, z * interval, 1}, Vec4::Zero});
                // 포인팅 벡터
                arrow_nodes_.vb.vertices.push_back(
                    Vertex_pf{Vec4{x * interval, y * interval, z * interval, 1}, Vec4::Zero});
                arrow_nodes_.vb.vertices.push_back(
                    Vertex_pf{Vec4{x * interval, y * interval, z * interval, 1}, Vec4::Zero});
            }
        }
    }
}

void Device::set_compute_task(Shader_program_type type, const std::string& cs_path,
                              std::function<void(uint32_t, Context&)> uniform_setter)
{
    Shader_program& task = shader_programs_[to_idx(type)];
    task.create();
    task.attach(opengl::Gl_shader(cs_path, GL_COMPUTE_SHADER));
    task.link();
    task.set_uniform_setter(uniform_setter);
}

void Device::set_render_task(Shader_program_type type, const std::string& vs_path, const std::string& fs_path,
                             std::function<void(uint32_t, Context&)> uniform_setter)
{
    Shader_program& task = shader_programs_[to_idx(type)];
    task.create();
    task.attach(opengl::Gl_shader(vs_path, GL_VERTEX_SHADER));
    task.attach(opengl::Gl_shader(fs_path, GL_FRAGMENT_SHADER));
    task.link();
    task.set_uniform_setter(uniform_setter);
}

void Device::init_wire(Context& ctx)
{
    item::Wire_node& nodes = ctx.scene->data().wire_properites.wire_nodes;
    nodes.create();
    nodes.bind(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
}

void Device::set_frame_buffer(Frame_buffer_type type, int width, int height)
{
    opengl::Frame_buffer& frame = frame_buffers_[to_idx(type)];
    frame.create();
    frame.bind(GL_FRAMEBUFFER);

    // 1. 입자를 담을 텍스처 생성
    frame.tex_.create();
    frame.tex_.bind(GL_TEXTURE_2D);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 2. FBO에 텍스처 연결
    frame.connect_tex(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Device::update_frame_buffer(Frame_buffer_type type, int width, int height)
{
    opengl::Frame_buffer& frame = frame_buffers_[to_idx(type)];

    frame.tex_.bind(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#if 0
    glBindFramebuffer(GL_FRAMEBUFFER, target.buf_.id_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.tex_.id_, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO Resize Failed : " << status << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void Device::update_shader_buffer(uint32_t buffer_id, uint32_t bind_slot, GLsizeiptr buffer_size, const void* data)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, data, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind_slot, buffer_id);
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
    data_.inv_frame_rate = 1 / rate;
}

void Device::set_back_col(const math::Vec4& col)
{
    data_.back_col = col;
}

void Device::set_compute_type(Compute_type type)
{
    data_.compute_type = type;
}

void Device::set_shader_task(Context& ctx)
{
    std::string shader_path = "../../../src/seop/graphic/seop_graphic/shader";
    set_compute_task(Shader_program_type::Compute_static_magnetic_field,
                     shader_path + "/comp/static_magnetic_field.comp", // FIXED : too long..
                     [this](uint32_t program_id, Context& ctx) {
                         glUseProgram(program_id);
                         set_uniform_scene_data(program_id, ctx);
                     });
    set_compute_task(Shader_program_type::Compute_lorenz_equation,
                     shader_path + "/comp/lolernz_equation.comp", // FIXED : too long..
                     [this](uint32_t program, Context& ctx) {
                         glUseProgram(program);
                         set_uniform_scene_data(program, ctx);
                         set_uniform_time_data(program, ctx);
                     });
    set_compute_task(Shader_program_type::Compute_arrow_field,
                     shader_path + "/comp/arrow_field.comp", // FIXED : too long..
                     [this](uint32_t program, Context& ctx) {
                         glUseProgram(program);
                         set_uniform_scene_data(program, ctx);
                         set_uniform_time_data(program, ctx);
                         glUniform1ui(glGetUniformLocation(program, "u_arrow_count"),
                                      (uint32_t)arrow_nodes_.vb.vertices.size() / 6);
                     });

#if 0
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
#endif
    set_render_task(Shader_program_type::Render_screen_quad,
                    shader_path + "/vert/screen_quad.vert", // FIXED : too long..
                    shader_path + "/frag/screen_quad.frag", // FIXED : too long..
                    [this](uint32_t program, Context& ctx) {
                        glUseProgram(program);
                        if (uint32_t tex = frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].use_tex) {
                            glUniform1i(glGetUniformLocation(program, "use_texture"), 1);
                            glActiveTexture(GL_TEXTURE0);
                            glUniform1i(glGetUniformLocation(program, "u_screen_texture"), 0);
                        } else {
                            glUniform1i(glGetUniformLocation(program, "use_texture"), 0);
                            glUniform4f(glGetUniformLocation(program, "u_color"), data_.back_col.x_, data_.back_col.y_,
                                        data_.back_col.z_, data_.fade_scale);
                        }
                    });
    set_render_task(Shader_program_type::Render_grid,
                    shader_path + "/vert/grid.vert", // FIXED : too long..
                    shader_path + "/frag/grid.frag", // FIXED : too long..
                    [this](uint32_t program_id, Context& ctx) {
                        glUseProgram(program_id);
                        set_uniform_scene_data(program_id, ctx);
                    });

    set_render_task(Shader_program_type::Render_particle,
                    shader_path + "/vert/particle.vert", // FIXED : too long..
                    shader_path + "/frag/particle.frag", // FIXED : too long..
                    [this](uint32_t program_id, Context& ctx) {
                        glUseProgram(program_id);
                        set_uniform_scene_data(program_id, ctx);
                    });

    set_render_task(Shader_program_type::Render_wire,
                    shader_path + "/vert/wire.vert", // FIXED : too long..
                    shader_path + "/frag/wire.frag", // FIXED : too long..
                    [this](uint32_t program_id, Context& ctx) {
                        glUseProgram(program_id);
                        set_uniform_scene_data(program_id, ctx);
                         set_uniform_time_data(program_id, ctx);
                    });

    set_render_task(
        Shader_program_type::Render_arrow,
        shader_path + "/vert/arrow.vert", // FIXED : too long..
        shader_path + "/frag/arrow.frag", // FIXED : too long..
        [this](uint32_t program_id, Context& ctx) {
            glUseProgram(program_id);
            set_uniform_scene_data(program_id, ctx);
            glUniform1i(glGetUniformLocation(program_id, "u_view_electric_field"), view_electric_field_arrow);
            glUniform1i(glGetUniformLocation(program_id, "u_view_magnetic_field"), view_magnetic_field_arrow);
            glUniform1i(glGetUniformLocation(program_id, "u_view_poynting_field"), view_poynting_field_arrow);
            glUniform1f(glGetUniformLocation(program_id, "u_arrow_scale"), arrow_scale);
        });
}

void Device::set_uniform_scene_data(uint32_t program_id, Context& ctx)
{
    glUniformMatrix4fv(glGetUniformLocation(program_id, "u_view"), 1, GL_FALSE,
                       (float*)&ctx.scene->camera().data().view);
    glUniformMatrix4fv(glGetUniformLocation(program_id, "u_projection"), 1, GL_FALSE,
                       (float*)&ctx.scene->camera().data().projection);

    glUniform1ui(glGetUniformLocation(program_id, "u_wire_count"),
                 (uint32_t)ctx.scene->data().wire_properites.wire_nodes.vb.vertices.size() / 2);
    glUniform1f(glGetUniformLocation(program_id, "u_w"), ctx.scene->data().wire_properites.w);
    glUniform1f(glGetUniformLocation(program_id, "u_max_i"), ctx.scene->data().wire_properites.max_i);
    glUniform1f(glGetUniformLocation(program_id, "u_c"), ctx.scene->data().wire_properites.c);
    glUniform1f(glGetUniformLocation(program_id, "u_c"), ctx.scene->data().wire_properites.c);

    glUniform1ui(glGetUniformLocation(program_id, "u_particle_count"),
                 (uint32_t)ctx.scene->data().entities.particles.vb.vertices.size());
    glUniform1f(glGetUniformLocation(program_id, "u_particle_col"), ctx.scene->data().particle_properties.col);
    glUniform1f(glGetUniformLocation(program_id, "u_time_scale"), ctx.scene->data().particle_properties.time_scale);
}

void Device::set_uniform_time_data(uint32_t program_id, Context& ctx)
{
    glUniform1f(glGetUniformLocation(program_id, "u_dt"), ctx.f_dt);
    glUniform1f(glGetUniformLocation(program_id, "u_time"), static_cast<float>(ctx.d_time));
}

void Device::shut_down()
{
}

} // namespace seop::graphic
