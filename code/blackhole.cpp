#include "blackhole.h"

//GLuint blackhole_framebuffer;
//GLuint blackhole_framebuffer_tex;
//GLuint blackhole_tex_disc;
//BlackholeShader* blackhole_shader = nullptr;
//
//void blackhole_framebuffer_init() {
//    create_framebuffer(&blackhole_framebuffer, &blackhole_framebuffer_tex, resolution_2i().x, resolution_2i().y);
//}
//
//void blackhole_init() {
//    blackhole_framebuffer_init();
//    load_texture(&blackhole_tex_disc, "texture/adisk2.bmp");
//    blackhole_shader = new BlackholeShader("shader/blackhole_adisc.glslf");
//}
//
//GLuint draw_blackhole_texture() {
//    glBindFramebuffer(GL_FRAMEBUFFER, blackhole_framebuffer);
//    blackhole_shader->enable();
//    blackhole_shader->update(resolution_3f(), time(), blackhole_tex_disc, blackhole_framebuffer_tex);
//    glRectf(-1.0, -1.0, 1.0, 1.0);
//    blackhole_shader->disable();
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    return blackhole_framebuffer_tex;
//}