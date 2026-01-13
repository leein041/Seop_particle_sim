#include "device.hpp"
#include "compute_shaders/electro_cs_shader.hpp"
#include "compute_shaders/gravity_cs_source.hpp"

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

    ctx.msg_queue->Register_handler<scene::Particle_change_message>(
        [this](scene::Particle_change_message& msg) -> bool { return on_message(msg); });

    set_draw_propeties();

    create_shader();
    create_shader_buffer();
    create_shader_program();
}

auto Device::on_message(msg::Message& msg) -> bool
{
    msg::Message_dispatcher dispatcher(msg);
    dispatcher.Dispatch<scene::Particle_change_message>([this](scene::Particle_change_message& msg) -> bool {
        update_shader_buffer(shader_buffer_data_.particle_sb, 0, sizeof(entity::Particle) * msg.particle_count(),
                             msg.particle_data());
        return true;
    });

    return true;
}

void Device::compute(float dt_f, Context& ctx)
{
    if (compute_type_ & Compute_type::Gravity) {
        // SSBO 생성 및 데이터 전송
        glUseProgram(shader_program_data_.gravity_cs_program);
        // Uniform 설정
        glUniform3fv(glGetUniformLocation(shader_program_data_.gravity_cs_program, "u_ray_point"), 1,
                     &ctx.input->data().ray_point.x_);
        glUniform1f(glGetUniformLocation(shader_program_data_.gravity_cs_program, "u_gravity"),
                    ctx.scene->data().forces.gravity);
        glUniform1f(glGetUniformLocation(shader_program_data_.gravity_cs_program, "u_orbit_force"),
                    ctx.scene->data().forces.gravity * ctx.scene->data().forces.vortex);
        glUniform1f(glGetUniformLocation(shader_program_data_.gravity_cs_program, "u_damping"),
                    ctx.scene->data().forces.damping);
        glUniform1f(glGetUniformLocation(shader_program_data_.gravity_cs_program, "u_magentic_str"),
                    ctx.scene->data().forces.magentic_str);
        glUniform1f(glGetUniformLocation(shader_program_data_.gravity_cs_program, "u_dt"), dt_f);
        glUniform1ui(glGetUniformLocation(shader_program_data_.gravity_cs_program, "u_particle_count"),
                     (GLuint)ctx.scene->data().particle_properties.count);
        glUniform1f(glGetUniformLocation(shader_program_data_.gravity_cs_program, "u_particle_col"),
                    ctx.scene->data().particle_properties.col);
        glUniform1ui(glGetUniformLocation(shader_program_data_.gravity_cs_program, "u_attractor_count"),
                     (GLuint)ctx.scene->data().attractor_properties.attractor_count);
    }
    if (compute_type_ & Compute_type::Electromagnetic) {
        // SSBO 생성 및 데이터 전송
        glUseProgram(shader_program_data_.em_cs_program);
        // Uniform 설정
        glUniform1f(glGetUniformLocation(shader_program_data_.em_cs_program, "u_damping"),
                    ctx.scene->data().forces.damping);
        glUniform1f(glGetUniformLocation(shader_program_data_.em_cs_program, "u_dt"), dt_f);
        glUniform1f(glGetUniformLocation(shader_program_data_.em_cs_program, "u_time_scale"), ctx.scene->data().particle_properties.time_scale);
        glUniform1ui(glGetUniformLocation(shader_program_data_.em_cs_program, "u_particle_count"),
                     (GLuint)ctx.scene->data().particle_properties.count);
        glUniform1f(glGetUniformLocation(shader_program_data_.em_cs_program, "u_particle_col"),
                    ctx.scene->data().particle_properties.col);
    }

    // 실행
    GLuint num_groups = static_cast<GLuint>((ctx.scene->data().particle_properties.count + 255) / 256);
    glDispatchCompute(num_groups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Device::prepare_grid(Context& ctx)
{

    glUseProgram(shader_program_data_.grid_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program_data_.grid_shader_program, "view"), 1, GL_FALSE,
                       (float*)&ctx.scene->camera().data().view);
    glUniformMatrix4fv(glGetUniformLocation(shader_program_data_.grid_shader_program, "projection"), 1, GL_FALSE,
                       (float*)&ctx.scene->camera().data().projection);

    glBindBuffer(GL_ARRAY_BUFFER, shader_buffer_data_.grid_sb);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(math::Vec4), 0);
}

void Device::draw_grid()
{
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(2);
}

void Device::prepare_draw(Context& ctx)
{
    glPointSize(ctx.scene->data().particle_properties.size);

    glUseProgram(shader_program_data_.render_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program_data_.render_shader_program, "view"), 1, GL_FALSE,
                       (float*)&ctx.scene->camera().data().view);
    glUniformMatrix4fv(glGetUniformLocation(shader_program_data_.render_shader_program, "projection"), 1, GL_FALSE,
                       (float*)&ctx.scene->camera().data().projection);

    glBindBuffer(GL_ARRAY_BUFFER, shader_buffer_data_.particle_sb);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(entity::Particle), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(entity::Particle), (void*)(16));
}

void Device::draw(size_t cnt)
{
    // 4. 그리기
    glDrawArrays(GL_POINTS, 0, (GLsizei)cnt);

    // 5. 정리
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Device::clear_frame()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(back_col_.x_, back_col_.y_, back_col_.z_, back_col_.w_);
}

void Device::create_shader()
{
    shader_data_.gravity_cs = make_cs(gravity_cs_source);
    shader_data_.ampere_cs = make_cs(ampere_cs_source);
    shader_data_.particle_vs = make_vs(particle_vs_source);
    shader_data_.particle_fs = make_fs(particle_fs_source);
    shader_data_.grid_vs = make_vs(grid_vs_source);
    shader_data_.grid_fs = make_fs(grid_fs_source);
}

void Device::create_shader_buffer()
{
    glGenBuffers(1, &shader_buffer_data_.particle_sb);
    glGenBuffers(1, &shader_buffer_data_.particle_sb);
    glGenBuffers(1, &shader_buffer_data_.attractor_sb);
}

void Device::create_shader_program()
{
    shader_program_data_.gravity_cs_program = glCreateProgram();
    glAttachShader(shader_program_data_.gravity_cs_program, shader_data_.gravity_cs);
    glLinkProgram(shader_program_data_.gravity_cs_program);

    shader_program_data_.em_cs_program = glCreateProgram();
    glAttachShader(shader_program_data_.em_cs_program, shader_data_.ampere_cs);
    glLinkProgram(shader_program_data_.em_cs_program);

    shader_program_data_.render_shader_program = glCreateProgram();
    glAttachShader(shader_program_data_.render_shader_program, shader_data_.particle_vs);
    glAttachShader(shader_program_data_.render_shader_program, shader_data_.particle_fs);
    glLinkProgram(shader_program_data_.render_shader_program);

    shader_program_data_.grid_shader_program = glCreateProgram();
    glAttachShader(shader_program_data_.grid_shader_program, shader_data_.grid_vs);
    glAttachShader(shader_program_data_.grid_shader_program, shader_data_.grid_fs);
    glLinkProgram(shader_program_data_.grid_shader_program);
}

void Device::set_draw_propeties()
{
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_BLEND);                // 블렌딩 활성화
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 입자들이 겹치면 더 밝아짐 (불꽃, 네온 효과)
}

GLuint Device::make_cs(const char* src)
{
    GLuint cs_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(cs_shader, 1, &src, NULL);
    glCompileShader(cs_shader);
    return cs_shader;
}

GLuint Device::make_vs(const char* src)
{
    GLuint vs_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs_shader, 1, &src, NULL);
    glCompileShader(vs_shader);
    return vs_shader;
}

GLuint Device::make_fs(const char* src)
{
    GLuint fs_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_shader, 1, &src, NULL);
    glCompileShader(fs_shader);
    return fs_shader;
}

void Device::update_shader_buffer(GLuint buffer_id, GLuint binding_point, GLsizeiptr buffer_size, const void* data)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);

    glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, data, GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, buffer_id);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

auto Device::grid() const -> const Grid&
{
    return grid_;
}

auto Device::compute_type() const -> uint32_t
{
    return compute_type_;
}

auto Device::shader_data() const -> const Shader_data&
{
    return shader_data_;
}

auto Device::shader_buffer_data() const -> const Shader_buffer_data
{
    return shader_buffer_data_;
}

auto Device::shader_program_data() const -> const Shader_program_data
{
    return shader_program_data_;
}

void Device::set_compute_type(Compute_type type)
{
    compute_type_ = type;
}

auto Device::shut_down() -> bool
{
    glfwTerminate();
    glDeleteShader(shader_data_.gravity_cs);
    return true;
}
} // namespace seop::graphic
