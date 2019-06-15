#pragma once
#include "common.h"

// hide the shader names
namespace impl {
    class BlackholeShader : public CGLShader {
        GLint _resolution, _time, _tex_disc, _tex_previous;
    public:
        BlackholeShader(const char* frag_shader) : CGLShader(nullptr, frag_shader) {
            _resolution = glGetUniformLocation(programObject, "resolution");
            _time = glGetUniformLocation(programObject, "time_total");
            _tex_disc = glGetUniformLocation(programObject, "tex_disc");
            _tex_previous = glGetUniformLocation(programObject, "tex_previous");

        };
        void update(vec3 resolution, float time, GLuint tex_disc, GLuint tex_previous) {
            glUniform3f(_resolution, resolution.x, resolution.y, resolution.z);
            glUniform1f(_time, time);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, tex_disc);
            glUniform1i(_tex_disc, 2);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, tex_previous);
            glUniform1i(_tex_previous, 1);
        }
    };
}

// 绘制黑洞
class BlackholeEffect : public Effect {
    GLuint blackhole_framebuffer;
    GLuint blackhole_framebuffer_tex;
    GLuint blackhole_tex_disc;
    impl::BlackholeShader* blackhole_shader = nullptr;
    void blackhole_framebuffer_init() {
        create_framebuffer(&blackhole_framebuffer, &blackhole_framebuffer_tex, resolution_2i().x, resolution_2i().y);
    }
public:
    BlackholeEffect() {
        blackhole_framebuffer_init();
        load_texture(&blackhole_tex_disc, "texture/adisk2.bmp");
        blackhole_shader = new impl::BlackholeShader("shader/blackhole_adisc.glslf");
    }

    // 窗口改变大小时调用
    // 重新创建帧缓冲区
    void resize() {
        blackhole_framebuffer_init();
    }

    // 将黑洞绘制到纹理上
    // 返回纹理id
    GLuint render_texture() {
        glBindFramebuffer(GL_FRAMEBUFFER, blackhole_framebuffer);
        auto shader = use_shader(blackhole_shader);
        shader->update(resolution_3f(), time(), blackhole_tex_disc, blackhole_framebuffer_tex);
        glRectf(-1.0, -1.0, 1.0, 1.0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return blackhole_framebuffer_tex;
    }
};