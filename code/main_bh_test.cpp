#include "common.h"
#include "bloom.h"
#include "draw_texture.h"
#include "blackhole.h"
#include "input.h"
#include <vector>
#include <unordered_map>

// 所有效果
// 便于窗口改变大小时调用resize()
vector<Effect*> effects;

BlackholeEffect* ef_blackhole;
BloomEffect* ef_bloom;
DrawTextureEffect* ef_draw_tex;


Input input;

mat4 cam_mat;
vec3 cam_pos = vec3(0, 4.5, -58);
vec2 cam_rot; // yaw and pitch
vec3 cam_pos_v;
vec2 cam_rot_v;
float cam_pos_v_init = 2.f;
float cam_rot_v_init = 0.25f;
float cam_v_decay = 0.1;

void set_cam_pos_velocity(vec3 dir) {
    cam_pos_v += vec3(cam_mat * vec4(dir, 0) * cam_pos_v_init);
}

void set_cam_rot_velocity(vec2 rot) {
    cam_rot_v += rot * cam_rot_v_init;
}

void update(float time_elapsed) {
    cam_pos += cam_pos_v * time_elapsed;
    cam_rot += cam_rot_v * time_elapsed;

    cam_pos_v *= 1 - cam_v_decay * max(1.f, time_elapsed);
    cam_rot_v *= 1 - cam_v_decay * max(1.f, time_elapsed);

    if (input.get_key('a')) {											//视角变化
        set_cam_pos_velocity(vec3(-1, 0, 0));
    }
    if (input.get_key('d')) {
        set_cam_pos_velocity(vec3(1, 0, 0));
    }
    if (input.get_key('w')) {
        set_cam_pos_velocity(vec3(0, 1, 0));
    }
    if (input.get_key('s')) {
        set_cam_pos_velocity(vec3(0, -1, 0));
    }
    if (input.get_key('z')) {
        set_cam_pos_velocity(vec3(0, 0, -1));
    }
    if (input.get_key('x')) {
        set_cam_pos_velocity(vec3(0, 0, 1));
    }
    if (input.get_key('j')) {								//视角变化
        set_cam_rot_velocity(vec2(-1, 0));
    }
    if (input.get_key('l')) {
        set_cam_rot_velocity(vec2(1, 0));
    }
    if (input.get_key('i')) {
        set_cam_rot_velocity(vec2(0, -1));
    }
    if (input.get_key('k')) {
        set_cam_rot_velocity(vec2(0, 1));
    }

    cam_mat = glm::identity<mat4>();
    cam_mat = glm::translate(cam_mat, cam_pos);
    cam_mat = glm::rotate(cam_mat, cam_rot[0], vec3(0, 1, 0));
    cam_mat = glm::rotate(cam_mat, cam_rot[1], vec3(1, 0, 0));
}

void render() {
    GLuint tex_bh = ef_blackhole->render_texture(cam_mat);
    GLuint tex_bh_bloom = ef_bloom->render_texture(tex_bh);
    ef_draw_tex->draw(tex_bh_bloom);
    
}

float time_previous = -1;
void display() {
    {
        static int frames, last_time;
        auto ticks = clock();
        if (frames == 0) last_time = ticks;
        frames++;
        if (ticks - last_time >= 500)
        {
            printf("FPS: %.2f\n", 1000.0 * frames / (ticks - last_time));
            frames = 0;
        }
    }

    if (time_previous < 0) time_previous = time();
    float time_elapsed = time() - time_previous;
    time_previous = time();

    update(time_elapsed);

    render();

    glFlush();
}

static void redisplay(int value)
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

static void key_up(unsigned char k, int x, int y)
{
    input.on_key_up(k);
}

static void key_down(unsigned char k, int x, int y)
{
    input.on_key_down(k);
}

int main(int argc, char* argv[])
{

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_SINGLE);
    glutInitWindowSize(800, 600);

    int windowHandle = glutCreateWindow("黑洞测试 WASDZX=相机位置 IJKL=相机方向");
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
    glutKeyboardFunc(key_down);
    glutKeyboardUpFunc(key_up);
    redisplay(10);

    glutMainLoop();

    return 0;
}