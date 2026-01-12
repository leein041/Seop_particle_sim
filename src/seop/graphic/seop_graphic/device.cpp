#include "device.hpp"
#include "seop_entity/particle.hpp"
#include "seop_math/math.hpp"
#include "seop_scene/camera.hpp"
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

void Device::init()
{
    glewInit();

    gravity_cs = make_cs(gravity_cs_source);
    ampere_cs = make_cs(ampere_cs_source);
    particle_vs = make_vs(particle_vs_source);
    particle_fs = make_fs(particle_fs_source);
    grid_vs = make_vs(grid_vs_source);
    grid_fs = make_fs(grid_fs_source);

	// test
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);
	
    math::Vec4 _0{-1.0f, 0.0f, -1.0f, 0.0f};
    math::Vec4 _1{1.0f, 0.0f, -1.0f, 0.0f};
    math::Vec4 _2{1.0f, 0.0f, 1.0f, 0.0f};
    math::Vec4 _3{-1.0f, 0.0f, 1.0f, 0.0f};
	grid_.push_back(_0);
	grid_.push_back(_1);
	grid_.push_back(_2);
	grid_.push_back(_3);

    gen_buffer();
    gen_shader_program();
}

void Device::compute(bool is_ui_hovered, float dt_f, scene::Scene_data &scene_data, input::Input_data &input_data)
{
    if (compute_type_ & Compute_type::Gravity) {
        // SSBO 생성 및 데이터 전송
        glUseProgram(gravity_cs_program);
        // Uniform 설정
        glUniform3fv(glGetUniformLocation(gravity_cs_program, "u_ray_point"), 1,
                     (is_ui_hovered) ? &Vec3::Zero.x_ : &input_data.ray_point.x_);
        glUniform1f(glGetUniformLocation(gravity_cs_program, "u_gravity"), scene_data.forces.gravity);
        glUniform1f(glGetUniformLocation(gravity_cs_program, "u_orbit_force"),
                    scene_data.forces.gravity * scene_data.forces.vortex);
        glUniform1f(glGetUniformLocation(gravity_cs_program, "u_damping"), scene_data.forces.damping);
        glUniform1f(glGetUniformLocation(gravity_cs_program, "u_magentic_str"), scene_data.forces.magentic_str);
        glUniform1f(glGetUniformLocation(gravity_cs_program, "u_dt"), dt_f);
        glUniform1ui(glGetUniformLocation(gravity_cs_program, "u_particle_count"),
                     (GLuint)scene_data.particle_properties.particle_count);
        glUniform1f(glGetUniformLocation(gravity_cs_program, "u_particle_col"),
                    scene_data.particle_properties.particle_col);
        glUniform1ui(glGetUniformLocation(gravity_cs_program, "u_attractor_count"),
                     (GLuint)scene_data.attractor_properties.attractor_count);
    }
    if (compute_type_ & Compute_type::Ampere) {
        // SSBO 생성 및 데이터 전송
        glUseProgram(ampere_cs_program);
        // Uniform 설정
        glUniform2f(glGetUniformLocation(ampere_cs_program, "u_cursor_pos"), 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(ampere_cs_program, "u_damping"), scene_data.forces.damping);
        glUniform1f(glGetUniformLocation(ampere_cs_program, "u_current"), scene_data.electronical_properites.I);
        glUniform1f(glGetUniformLocation(ampere_cs_program, "u_dt"), dt_f);
        glUniform1ui(glGetUniformLocation(ampere_cs_program, "u_particle_count"),
                     (GLuint)scene_data.particle_properties.particle_count);
        glUniform1f(glGetUniformLocation(ampere_cs_program, "u_particle_col"),
                    scene_data.particle_properties.particle_col);
    }

    // 실행
    GLuint num_groups = static_cast<GLuint>((scene_data.particle_properties.particle_count + 255) / 256);
    glDispatchCompute(num_groups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Device::prepare_grid(const Matrix &view, const Matrix &projection)
{
    glEnable(GL_BLEND);                                // 블렌딩 활성화
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 입자들이 겹치면 더 밝아짐 (불꽃, 네온 효과)

    glUseProgram(grid_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(grid_shader_program, "view"), 1, GL_FALSE, (float *)&view);
    glUniformMatrix4fv(glGetUniformLocation(grid_shader_program, "projection"), 1, GL_FALSE, (float *)&projection);

    glBindBuffer(GL_ARRAY_BUFFER, grid_sb);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(math::Vec4), 0);
}

void Device::draw_grid()
{
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(2);
}

void Device::prepare_draw(const Matrix &view, const Matrix &projection, float particle_size)
{

    glPointSize(particle_size);

    glUseProgram(render_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(render_shader_program, "view"), 1, GL_FALSE, (float *)&view);
    glUniformMatrix4fv(glGetUniformLocation(render_shader_program, "projection"), 1, GL_FALSE, (float *)&projection);

    glBindBuffer(GL_ARRAY_BUFFER, particle_sb);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(entity::Particle), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(entity::Particle), (void *)(16));
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

void Device::make_shader()
{
}

void Device::gen_buffer()
{
    glGenBuffers(1, &grid_sb);
    glGenBuffers(1, &particle_sb);
    glGenBuffers(1, &ssbo_attractor);
}

void Device::gen_shader_program()
{
    gravity_cs_program = glCreateProgram();
    glAttachShader(gravity_cs_program, gravity_cs);
    glLinkProgram(gravity_cs_program);

    ampere_cs_program = glCreateProgram();
    glAttachShader(ampere_cs_program, ampere_cs);
    glLinkProgram(ampere_cs_program);

    render_shader_program = glCreateProgram();
    glAttachShader(render_shader_program, particle_vs);
    glAttachShader(render_shader_program, particle_fs);
    glLinkProgram(render_shader_program);

    grid_shader_program = glCreateProgram();
    glAttachShader(grid_shader_program, grid_vs);
    glAttachShader(grid_shader_program, grid_fs);
    glLinkProgram(grid_shader_program);
}

GLuint Device::make_cs(const char *src)
{
    GLuint cs_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(cs_shader, 1, &src, NULL);
    glCompileShader(cs_shader);
    return cs_shader;
}

GLuint Device::make_vs(const char *src)
{
    GLuint vs_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs_shader, 1, &src, NULL);
    glCompileShader(vs_shader);
    return vs_shader;
}

GLuint Device::make_fs(const char *src)
{
    GLuint fs_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_shader, 1, &src, NULL);
    glCompileShader(fs_shader);
    return fs_shader;
}

void Device::update_shader_buffer(GLuint buffer_id, GLuint binding_point, GLsizeiptr buffer_size, const void *data)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);

    glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, data, GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, buffer_id);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Device::update_vertex_buffer(GLuint buffer_id, GLsizeiptr buffer_size, const void *data)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);

    glBufferData(GL_ARRAY_BUFFER, buffer_size, data, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

auto Device::compute_type() const -> uint32_t
{
    return compute_type_;
}

auto Device::shut_down() -> bool
{
    glfwTerminate();
    glDeleteShader(gravity_cs);
    return true;
}
} // namespace seop::graphic
