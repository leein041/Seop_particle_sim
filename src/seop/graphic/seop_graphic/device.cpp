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

    ctx.msg_queue->Register_handler<scene::Particle_change_message>(
        [this](scene::Particle_change_message& msg) -> bool { return on_message(msg); });

    set_draw_propeties();

    create_shader();
    create_shader_buffer();
    create_shader_program();
    create_frame_buffer(particle_layer_buffer, particle_layer_tex, static_cast<int>(ctx.window->window_size().x_),
                        static_cast<int>(ctx.window->window_size().y_));

    glBindFramebuffer(GL_FRAMEBUFFER, particle_layer_buffer);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 완전히 투명한 검정색으로 초기화
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // temp
    quad_vb.vertices_.push_back(Vertex_pu{Vec4{-1.0f, 1.0f, 0.0f, 1.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f}}); // pos, uv
    quad_vb.vertices_.push_back(Vertex_pu{Vec4{-1.0f, -1.0f, 0.0f, 1.0f}, Vec4{0.0f, 0.0f, 0.0f, 0.0f}});
    quad_vb.vertices_.push_back(Vertex_pu{Vec4{1.0f, -1.0f, 0.0f, 1.0f}, Vec4{1.0f, 0.0f, 0.0f, 0.0f}});
    quad_vb.vertices_.push_back(Vertex_pu{Vec4{-1.0f, 1.0f, 0.0f, 1.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f}});
    quad_vb.vertices_.push_back(Vertex_pu{Vec4{1.0f, -1.0f, 0.0f, 1.0f}, Vec4{1.0f, 0.0f, 0.0f, 0.0f}});
    quad_vb.vertices_.push_back(Vertex_pu{Vec4{1.0f, 1.0f, 0.0f, 1.0f}, Vec4{1.0f, 1.0f, 0.0f, 0.0f}});

    glGenVertexArrays(1, &quad_va.id_);
    glGenBuffers(1, &quad_vb.id_);
    glBindVertexArray(quad_va.id_);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vb.id_);
    glBufferData(GL_ARRAY_BUFFER, quad_vb.vertices_.size() * sizeof(Vertex_pu), quad_vb.vertices_.data(),
                 GL_STATIC_DRAW);

    // 위치 속성 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_pu), (void*)0);
    // 텍스처 좌표 속성 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_pu), (void*)16);

    glBindVertexArray(0); // 설정 완료 후 언바인딩
}

auto Device::on_message(msg::Message& msg) -> bool
{
    msg::Message_dispatcher dispatcher(msg);
    dispatcher.Dispatch<scene::Particle_change_message>([this](scene::Particle_change_message& msg) -> bool {
        bind_shader_buffer(shader_buffer_data_.particle_sb, 0, sizeof(entity::Particle) * msg.particle_count(),
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
        glUseProgram(shader_program_data_.electro_cs_program);
        // Uniform 설정
        glUniform1f(glGetUniformLocation(shader_program_data_.electro_cs_program, "u_damping"),
                    ctx.scene->data().forces.damping);
        glUniform1f(glGetUniformLocation(shader_program_data_.electro_cs_program, "u_dt"), dt_f);
        glUniform1f(glGetUniformLocation(shader_program_data_.electro_cs_program, "u_time_scale"),
                    ctx.scene->data().particle_properties.time_scale);
        glUniform1ui(glGetUniformLocation(shader_program_data_.electro_cs_program, "u_particle_count"),
                     (GLuint)ctx.scene->data().particle_properties.count);
        glUniform1f(glGetUniformLocation(shader_program_data_.electro_cs_program, "u_particle_col"),
                    ctx.scene->data().particle_properties.col);
    }

    // 실행
    GLuint num_groups = static_cast<GLuint>((ctx.scene->data().particle_properties.count + 255) / 256);
    glDispatchCompute(num_groups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Device::render(Context& ctx)
{
#if 1
    // draw particle on particle laye
    glBindFramebuffer(GL_FRAMEBUFFER, particle_layer_buffer);
    glDisable(GL_DEPTH_TEST);

    if (ctx.scene->camera().camera_state().is_move) {
        // 카메라가 움직이면 잔상을 남기지 않고 완전히 투명하게 지움
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    } else {
        // 가만히 있을 때는 기존처럼 검은색 쿼드를 살짝 덮어 잔상 생성
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        prepare_quad(ctx, 0); // fade_alpha가 적용된 검은 쿼드
        draw_quad();
    }

    glEnable(GL_BLEND);                // 블렌딩 활성화
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 입자들이 겹치면 더 밝아짐 (불꽃, 네온 효과)
    prepare_particle(ctx);
    draw_particle(ctx.scene->data().particle_properties.count);
#endif
    // draw main buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(back_col_.x_, back_col_.y_, back_col_.z_, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_BLEND);
    prepare_grid(ctx);
    draw_grid();
#if 1
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    prepare_quad(ctx, particle_layer_tex);
    draw_quad();
#endif

    // test
    pre_view = ctx.scene->camera().data().view;
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

void Device::prepare_particle(Context& ctx)
{
    glPointSize(ctx.scene->data().particle_properties.size);

    glUseProgram(shader_program_data_.particle_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program_data_.particle_shader_program, "view"), 1, GL_FALSE,
                       (float*)&ctx.scene->camera().data().view);
    glUniformMatrix4fv(glGetUniformLocation(shader_program_data_.particle_shader_program, "projection"), 1, GL_FALSE,
                       (float*)&ctx.scene->camera().data().projection);

    glBindBuffer(GL_ARRAY_BUFFER, shader_buffer_data_.particle_sb);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(entity::Particle), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(entity::Particle), (void*)(16));
}

void Device::prepare_quad(Context& ctx, GLuint tex_id)
{
    glUseProgram(shader_program_data_.quad_shader_program);
    GLint loc_use = glGetUniformLocation(shader_program_data_.quad_shader_program, "use_texture");
    GLint loc_tex = glGetUniformLocation(shader_program_data_.quad_shader_program, "u_screen_texture");
    GLint loc_col = glGetUniformLocation(shader_program_data_.quad_shader_program, "u_color");
    GLint loc_reproj_matrix =
        glGetUniformLocation(shader_program_data_.quad_shader_program, "u_reproject_matrix"); // 추가
    if (tex_id) {
        glUniform1i(loc_use, 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glUniform1i(loc_tex, 0);
    } else {
        glUniform1i(loc_use, 0);
        glUniform4f(loc_col, 0.0f, 0.0f, 0.0f, fade_scale);
    }
}

void Device::draw_particle(size_t cnt)
{
    // 4. 그리기
    glDrawArrays(GL_POINTS, 0, (GLsizei)cnt);

    // 5. 정리
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Device::draw_quad()
{

    // 우리가 만든 VAO 바인딩 후 그리기
    glBindVertexArray(quad_va.id_);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
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
    shader_data_.quad_vs = make_vs(quad_vs_source);
    shader_data_.quad_fs = make_fs(quad_fs_source);
}

void Device::create_shader_buffer()
{
    glGenBuffers(1, &shader_buffer_data_.particle_sb);
}

void Device::create_shader_program()
{
    shader_program_data_.gravity_cs_program = glCreateProgram();
    glAttachShader(shader_program_data_.gravity_cs_program, shader_data_.gravity_cs);
    glLinkProgram(shader_program_data_.gravity_cs_program);

    shader_program_data_.electro_cs_program = glCreateProgram();
    glAttachShader(shader_program_data_.electro_cs_program, shader_data_.ampere_cs);
    glLinkProgram(shader_program_data_.electro_cs_program);

    shader_program_data_.particle_shader_program = glCreateProgram();
    glAttachShader(shader_program_data_.particle_shader_program, shader_data_.particle_vs);
    glAttachShader(shader_program_data_.particle_shader_program, shader_data_.particle_fs);
    glLinkProgram(shader_program_data_.particle_shader_program);

    shader_program_data_.grid_shader_program = glCreateProgram();
    glAttachShader(shader_program_data_.grid_shader_program, shader_data_.grid_vs);
    glAttachShader(shader_program_data_.grid_shader_program, shader_data_.grid_fs);
    glLinkProgram(shader_program_data_.grid_shader_program);

    shader_program_data_.quad_shader_program = glCreateProgram();
    glAttachShader(shader_program_data_.quad_shader_program, shader_data_.quad_vs);
    glAttachShader(shader_program_data_.quad_shader_program, shader_data_.quad_fs);
    glLinkProgram(shader_program_data_.quad_shader_program);

    GLint success;
    glGetProgramiv(shader_program_data_.quad_shader_program, GL_LINK_STATUS, &success);
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

void Device::create_frame_buffer(GLuint& fbo, GLuint& tex, int width, int height)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 1. 입자를 담을 텍스처 생성
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 2. FBO에 텍스처 연결
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "FBO Error!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Device::bind_shader_buffer(GLuint buffer_id, GLuint binding_point, GLsizeiptr buffer_size, const void* data)
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

auto Device::grid() const -> const Screen_quad&
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

void Device::create_vertex_buffer()
{
}

void Device::create_vertex_array()
{
}

auto Device::shut_down() -> bool
{
    glfwTerminate();
    glDeleteShader(shader_data_.gravity_cs);
    return true;
}
} // namespace seop::graphic
