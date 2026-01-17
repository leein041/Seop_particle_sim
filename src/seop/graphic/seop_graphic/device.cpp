#include "device.hpp"
#include "compute_shaders/electro_cs_shader.hpp"
#include "compute_shaders/gravity_cs_source.hpp"
#include "quad_shader.hpp"
#include "shader.hpp"

#include "seop_entity/particle.hpp"
#include "seop_math/math.hpp"
#include "seop_message/message.hpp"
#include "seop_scene/camera.hpp"
#include "seop_scene/scene.hpp"
#include "seop_scene/scene_message.hpp"
#include "seop_window/glf_window.hpp"

#include <iostream>
#include <memory>
#include <vector>

namespace seop::graphic
{
using namespace math;

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
    update_shader_buffer(shader_buffers_[to_idx(Shader_buffer_type::Particle)].sb, 0,
                         sizeof(entity::Particle) * ctx.scene->data().particle_properties.count,
                         ctx.scene->data().entities.particles.data());

    add_compute_task(
        Shader_task_type::Compute_electromagnetic, electromagnetic_cs_source, [](GLuint program, Context& ctx) {
            glUseProgram(program);
            // Uniform 설정
            glUniform1f(glGetUniformLocation(program, "u_damping"), ctx.scene->data().forces.damping);
            glUniform1f(glGetUniformLocation(program, "u_dt"), ctx.f_dt);
            glUniform1f(glGetUniformLocation(program, "u_time_scale"),
                        ctx.scene->data().particle_properties.time_scale);
            glUniform1ui(glGetUniformLocation(program, "u_particle_count"),
                         (GLuint)ctx.scene->data().particle_properties.count);
            glUniform1f(glGetUniformLocation(program, "u_particle_col"), ctx.scene->data().particle_properties.col);
        });
    add_compute_task(Shader_task_type::Compute_gravity, electromagnetic_cs_source, [](GLuint program, Context& ctx) {
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
                     (GLuint)ctx.scene->data().particle_properties.count);
        glUniform1ui(glGetUniformLocation(program, "u_attractor_count"),
                     (GLuint)ctx.scene->data().attractor_properties.attractor_count);
    });
    add_render_task(Shader_task_type::Render_screen_quad, quad_vs_source, quad_fs_source,
                    [this](GLuint program, Context& ctx) {
                        glUseProgram(program);
                        if (GLuint tex = frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].use_tex) {
                            glUniform1i(glGetUniformLocation(program, "use_texture"), 1);
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, tex);
                            glUniform1i(glGetUniformLocation(program, "u_screen_texture"), 0);
                        } else {
                            glUniform1i(glGetUniformLocation(program, "use_texture"), 0);
                            glUniform4f(glGetUniformLocation(program, "u_color"), 0.0f, 0.0f, 0.0f, data_.fade_scale);
                        }
                    });
    add_render_task(Shader_task_type::Render_grid, grid_vs_source, grid_fs_source, [](GLuint program, Context& ctx) {
        glUseProgram(program);

        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,
                           (float*)&ctx.scene->camera().data().view);
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE,
                           (float*)&ctx.scene->camera().data().projection);
    });

    add_render_task(Shader_task_type::Render_particle, particle_vs_source, particle_fs_source,
                    [](GLuint program, Context& ctx) {
                        glUseProgram(program);
                        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,
                                           (float*)&ctx.scene->camera().data().view);
                        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE,
                                           (float*)&ctx.scene->camera().data().projection);
                    });

    add_frame_buffer(Frame_buffer_type::Particle_layer, static_cast<int>(ctx.window->window_size().x_),
                     static_cast<int>(ctx.window->window_size().y_));

    create_grid();
    create_screen_qaud();
}

void Device::create_screen_qaud()
{
    screen_quad_.vertices.push_back(Vertex_pu{Vec4{-1.0f, 1.0f, 0.0f, 1.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f}}); // pos, uv
    screen_quad_.vertices.push_back(Vertex_pu{Vec4{-1.0f, -1.0f, 0.0f, 1.0f}, Vec4{0.0f, 0.0f, 0.0f, 0.0f}});
    screen_quad_.vertices.push_back(Vertex_pu{Vec4{1.0f, -1.0f, 0.0f, 1.0f}, Vec4{1.0f, 0.0f, 0.0f, 0.0f}});
    screen_quad_.vertices.push_back(Vertex_pu{Vec4{-1.0f, 1.0f, 0.0f, 1.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f}});
    screen_quad_.vertices.push_back(Vertex_pu{Vec4{1.0f, -1.0f, 0.0f, 1.0f}, Vec4{1.0f, 0.0f, 0.0f, 0.0f}});
    screen_quad_.vertices.push_back(Vertex_pu{Vec4{1.0f, 1.0f, 0.0f, 1.0f}, Vec4{1.0f, 1.0f, 0.0f, 0.0f}});

    glGenVertexArrays(1, &screen_quad_.va);
    glGenBuffers(1, &screen_quad_.vb);
    glBindVertexArray(screen_quad_.va);
    glBindBuffer(GL_ARRAY_BUFFER, screen_quad_.vb);
    glBufferData(GL_ARRAY_BUFFER, screen_quad_.vertices.size() * sizeof(Vertex_pu), screen_quad_.vertices.data(),
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
    grid_quad_.vertices.push_back(Vertex_pc{Vec4{-1.0f, 0.0f, 1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}}); // pos, col
    grid_quad_.vertices.push_back(Vertex_pc{Vec4{1.0f, 0.0f, 1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}});
    grid_quad_.vertices.push_back(Vertex_pc{Vec4{1.0f, 0.0f, -1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}});
    grid_quad_.vertices.push_back(Vertex_pc{Vec4{-1.0f, 0.0f, -1.0f, 1.0f}, Vec4{1.0f, 1.0f, 1.0f, 1.0f}});

    glGenVertexArrays(1, &grid_quad_.va);
    glGenBuffers(1, &grid_quad_.vb);
    glBindVertexArray(grid_quad_.va);
    glBindBuffer(GL_ARRAY_BUFFER, grid_quad_.vb);
    glBufferData(GL_ARRAY_BUFFER, grid_quad_.vertices.size() * sizeof(Vertex_pc), grid_quad_.vertices.data(),
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
        update_shader_buffer(shader_buffers_[to_idx(Shader_buffer_type::Particle)].sb, 0,
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

    // 실행
    GLuint num_groups = static_cast<GLuint>((ctx.scene->data().particle_properties.count + 255) / 256);
    glDispatchCompute(num_groups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Device::render(Context& ctx)
{
#if 1
    // pipeline 1
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].buffer);
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
#endif
    // pipeline 3
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(data_.back_col.x_, data_.back_col.y_, data_.back_col.z_, data_.back_col.w_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_BLEND);
    prepare_grid(ctx);
    draw_grid();
#if 1
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    frame_buffers_[to_idx(Frame_buffer_type::Particle_layer)].use_tex = true;
    prepare_quad(ctx);
    draw_quad();
#endif
}

void Device::prepare_grid(Context& ctx)
{
    shader_tasks_[to_idx(Shader_task_type::Render_grid)].uniform_setter(ctx);
}

void Device::draw_grid()
{
    glBindVertexArray(grid_quad_.va);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(2);
}

void Device::prepare_particle(Context& ctx)
{
    glPointSize(ctx.scene->data().particle_properties.size);

    shader_tasks_[to_idx(Shader_task_type::Render_particle)].uniform_setter(ctx);

    glBindBuffer(GL_ARRAY_BUFFER, shader_buffers_[to_idx(Shader_buffer_type::Particle)].sb);
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
    glBindVertexArray(screen_quad_.va);
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
}

GLuint Device::create_cs(const char* src)
{
    GLuint cs_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(cs_shader, 1, &src, NULL);
    glCompileShader(cs_shader);
    return cs_shader;
}

GLuint Device::create_vs(const char* src)
{
    GLuint vs_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs_shader, 1, &src, NULL);
    glCompileShader(vs_shader);
    return vs_shader;
}

GLuint Device::create_fs(const char* src)
{
    GLuint fs_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_shader, 1, &src, NULL);
    glCompileShader(fs_shader);
    return fs_shader;
}

void Device::add_compute_task(Shader_task_type type, const char* cs_src,
                              std::function<void(GLuint, Context&)> uniform_setter)
{
    GLuint program = glCreateProgram();
    GLuint cs = create_cs(cs_src);
    glAttachShader(program, cs);
    glLinkProgram(program);

    shader_tasks_[to_idx(type)] =
        Shader_task{program, [program, uniform_setter](Context& ctx) { uniform_setter(program, ctx); }};
    glDeleteShader(cs); // 링크 후 셰이더 정리
}

void Device::add_shader_buffer(Shader_buffer_type type)
{
    Shader_buffer buf;
    glGenBuffers(1, &buf.sb);
    shader_buffers_[to_idx(type)] = buf;
}

void Device::add_render_task(Shader_task_type type, const char* vs_src, const char* fs_src,
                             std::function<void(GLuint, Context&)> uniform_setter)
{
    GLuint vs = create_vs(vs_src);
    GLuint fs = create_fs(fs_src);
    GLuint program = glCreateProgram();
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
    glGenFramebuffers(1, &target.buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, target.buffer);

    // 1. 입자를 담을 텍스처 생성
    glGenTextures(1, &target.texture);
    glBindTexture(GL_TEXTURE_2D, target.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 2. FBO에 텍스처 연결
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.texture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    frame_buffers_[to_idx(type)] = target;
}

void Device::update_shader_buffer(GLuint buffer_id, GLuint binding_point, GLsizeiptr buffer_size, const void* data)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);

    glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, data, GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, buffer_id);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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
    glfwTerminate();
}

} // namespace seop::graphic
