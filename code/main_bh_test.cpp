#include "common.h"
#include "bloom.h"
#include "draw_texture.h"
#include "blackhole.h"
#include <vector>

// 所有效果
// 便于窗口改变大小时调用resize()
vector<Effect*> effects;

BlackholeEffect* ef_blackhole;
BloomEffect* ef_bloom;
DrawTextureEffect* ef_draw_tex;

void display() {
    static int frames, last_time;
    auto ticks = clock();
    if (frames == 0) last_time = ticks;
    frames++;
    if (ticks - last_time >= 500)
    {
        printf("FPS: %.2f\n", 1000.0 * frames / (ticks - last_time));
        frames = 0;
    }

    GLuint tex_bh = ef_blackhole->render_texture();
    GLuint tex_bh_bloom = ef_bloom->render_texture(tex_bh);
    ef_draw_tex->draw(tex_bh_bloom);

    glFlush();
}

void redisplay(int value)
{
    glutPostRedisplay();
    glutTimerFunc(value, redisplay, value);
}

static void reshape(int w, int h) {
    _set_geometry(w, h, 0, 0);
    glViewport(0, 0, w, h);
    for (Effect* ef : effects) {
        ef->resize();
    }
}

int main(int argc, char* argv[])
{

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_SINGLE);
    glutInitWindowSize(800, 600);

    int windowHandle = glutCreateWindow("计算机图形学期末项目：ZJUGalaxy");
    _set_time_start();

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        printf("Error: %s\n", glewGetErrorString(err));
    }

    // 初始化效果
    effects.push_back(ef_blackhole = new BlackholeEffect());
    effects.push_back(ef_bloom = new BloomEffect());
    effects.push_back(ef_draw_tex = new DrawTextureEffect());

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    redisplay(10);

    glutMainLoop();

    return 0;
}