//头文件
#include<stdio.h>
#include <stdlib.h>
#include<Windows.h>
#include<string>
#include<math.h>
#include "glut.h"
#include "snap.h"
#include "blackhole_scene.h"

//定义
#define BITMAP_ID 0x4D42
#define TEXTURE_PATH(name) ("texture/" name ".bmp")
#define PI 3.14159265

//行星信息结构
typedef struct planet {
    char* name;							//行星中文名
    char* texture_path;					//行星的贴图文件路径
    GLuint* texture_name;				//行星贴图编号
    GLdouble radius;					//行星相对半径
    //GLuint display_list;				//行星的展示序列//未实现
    GLdouble tilt;						// 行星自转轴偏角
    GLdouble z_rotation_inverse[16];	//未知矩阵，好像是用来算偏角的
    GLdouble period;					// 按天算的行星自转周期
    struct {
        GLdouble inclination;			// 公转偏角.
        GLdouble radius;				//行星的轨道半径.
    //	GLuint display_list;			//公转轴的展示列表
        GLdouble period;				// 公转周期
    } orbit;
    //struct body_t *planets[];			//星球的卫星指针数组
} planet;

//纹理数组
GLuint texture[10];

//一些其他常量信息
const GLdouble ORBIT_RADIUS_FACTOR = 10;		//行星公转轨道半径参数
const GLdouble ORBIT_INNER_RADIUS = 0.02;		//行星轨道环半径参数
const GLdouble BODY_ROTATION_FACTOR = 20;		//行星自转角度参数
const GLdouble BODY_ROTATION_SPEED_FACTOR = 1;	//行星自转速度参数

const int screenHeight = 512;					//屏幕高度
const int screenWidth = 1024;					//屏幕宽度


//背景图纹理路径
char* BACKGROUND = TEXTURE_PATH("milky_way");
char* backgrounds_path[6] = { TEXTURE_PATH("front"),TEXTURE_PATH("back"),TEXTURE_PATH("bottom"),TEXTURE_PATH("top"),TEXTURE_PATH("left"),TEXTURE_PATH("right") };
//背景纹理编号
GLuint* background = &texture[9];
GLuint backgrounds[6];

//整体相对偏移数据
float fTranslate;								//移动初始值
float fRotate;									//旋转初始值
float fScale = 1.0f;							//缩放初始值

//视角偏移参数
float R1 = 0;									//水平绕轴偏角
float R2 = 0;									//垂直绕轴偏角
float L1 = 20;									//极坐标长度

//其他变量
GLdouble g_body_rotation_speed = 1;				//旋转速度
GLdouble g_body_rotation_phase = 0;				//旋转进度
GLfloat body_rotation_self = 0;						//自转衡量度
GLfloat body_rotation = 0;						//公转转衡量度
GLfloat body_rotation__self_speed = 1.0;		//自转衡量度速度
GLfloat body_rotation_speed = 1.0;				//公转转衡量度速度

//二次曲面声明类型
GLUquadricObj* qobj;

//其他开关参数（有几个没用上）
bool bPersp = false;	 //透视
bool bAnim = false;		 //旋转
bool bWire = false;		 //线框
bool bMix = false;		 //混合光照与纹理
bool bTex = false;		 //显示纹理
bool bRotateSelf = false;//自转
bool bRotate = false;    //公转
bool bStarRow = true;    //星环

int speed = 0;			//公转速度挡位
int speed_self = 0;		//自转速度挡位
int choices = 1;

//预设颜色
float white[] = { 1.0f, 1.0f,1.0f, 1.0f };
float grey[] = { 0.5f,0.5f,0.5f,1.0f };
float green[] = { 0.0f,1.0f,0.0f,1.0f };

//观察点与观察中心
float eye[] = { 0, 0, 8 };
float center[] = { 0, 0, 0 };

//窗口信息
int wWidth = 0;
int wHeight = 0;

//灯光信息

GLfloat LightRed[] = { 1.0	,	0.0	,	0.0	,	1.0 };	//预设颜色
GLfloat LightWhite[] = { 1.0	,	1.0	,	1.0	,	1.0 };
GLfloat light_pos[] = { 0.0	,	0.0	,	0.0	,	1.0 };  //灯光位置

//行星信息            名字		纹理路径					纹理编号		半径	自转偏角	未知矩阵		自转时间			公转轨道半径 公转周期
planet SUN = { "太阳"	,TEXTURE_PATH("sun")		,&texture[0],  10.9   ,7.25			,{
                                                                                                1, 0, 0, 0,
                                                                                                0, 1, 0, 0,
                                                                                                0, 0, 1, 0,
                                                                                                0, 0, 0, 1
                                                                                                  }				,7.25		,		{0		,0			,0			} };
planet MERCURY = { "水星"	,TEXTURE_PATH("mercury")	,&texture[1],  0.3829 ,0.034		,{}				,58.646		,		{7.005	,0.387098	,87.9691	} };
planet VENUS = { "金星"	,TEXTURE_PATH("venus")		,&texture[2],  0.9499 ,177.4		,{}				,-243.025	,		{3.395	,0.723332   ,224.701	} };
planet EARTH = { "地球"	,TEXTURE_PATH("earth")		,&texture[3],  1      ,23.4392811	,{}				,0.99726968	,		{0		,1			,365.256363 } };
planet MARS = { "火星"	,TEXTURE_PATH("mars")		,&texture[4],  0.5320 ,25.19		,{}				,25.19		,		{1.85	,1.523679	,686.971	} };
planet JUPITER = { "木星"	,TEXTURE_PATH("jupiter")	,&texture[5],  10.97  ,3.08			,{}				,3.13		,		{1.303	,5.20260	,4332.59	} };
planet SATURN = { "土星"	,TEXTURE_PATH("saturn")		,&texture[6],  9.140  ,26.7			,{}				,26.73		,		{2.489	,9.54909	,10759.22	} };
planet URANUS = { "天王星"	,TEXTURE_PATH("uranus")		,&texture[7],  3.981  ,97.9			,{}				,97.77		,		{0.773	,19.2184	,30688.5	} };
planet NEPTUNE = { "海王星"	,TEXTURE_PATH("neptune")	,&texture[8],  3.865  ,27.8			,{}				,28.32		,		{1.770	,30.110387	,60182		} };
//planet MOON     =   {"月亮"     ,TEXTURE_PATH("moon")		,&texture[9],   	  ,				,{}				,			,		{		,			,			}	};

//行星数组
planet solarSystem[9] = { SUN,MERCURY,VENUS,EARTH,MARS,JUPITER,SATURN,URANUS,NEPTUNE };



//



//加载文件到bit信息头文件
unsigned char* LoadBitmapFile(char* filename, BITMAPINFOHEADER* bitmapInfoHeader)
{
    FILE* filePtr;						// 文件指针
    BITMAPFILEHEADER bitmapFileHeader;	// bitmap文件头
    unsigned char* bitmapImage;		// bitmap图像数据
    int	imageIdx = 0;					// 图像位置索引
    unsigned char	tempRGB;			// 交换变量
    // 以“二进制+读”模式打开文件filename 
    filePtr = fopen(filename, "rb");
    if (filePtr == NULL) {
        return NULL;
    }
    // 读入bitmap文件图
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
    // 验证是否为bitmap文件
    if (bitmapFileHeader.bfType != BITMAP_ID) {
        fprintf(stderr, "Error in LoadBitmapFile: the file is not a bitmap file\n");
        return NULL;
    }
    // 读入bitmap信息头
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
    // 将文件指针移至bitmap数据
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
    // 为装载图像数据创建足够的内存
    bitmapImage = new unsigned char[bitmapInfoHeader->biSizeImage];
    // 验证内存是否创建成功
    if (!bitmapImage) {
        fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
        return NULL;
    }
    // 读入bitmap图像数据
    fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
    // 确认读入成功
    if (bitmapImage == NULL) {
        fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
        return NULL;
    }
    //由于bitmap中保存的格式是BGR，下面交换R和B的值，得到RGB格式
    for (imageIdx = 0;
        imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }
    // 关闭bitmap图像文件
    fclose(filePtr);
    return bitmapImage;
}
//加载纹理图片到纹理数组
void texload(int i, char* filename, GLuint* texture)
{
    BITMAPINFOHEADER bitmapInfoHeader;                                 // bitmap信息头
    unsigned char* bitmapData;                                       // 纹理数据

    bitmapData = LoadBitmapFile(filename, &bitmapInfoHeader);
    glBindTexture(GL_TEXTURE_2D, texture[i]);
    // 指定当前纹理的放大/缩小过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
        0, 							//mipmap层次(通常为，表示最上层) 
        GL_RGB,						//我们希望该纹理有红、绿、蓝数据
        bitmapInfoHeader.biWidth,	//纹理宽带，必须是n，若有边框+2 
        bitmapInfoHeader.biHeight,	//纹理高度，必须是n，若有边框+2 
        0,							//边框(0=无边框, 1=有边框) 
        GL_RGB,						//bitmap数据的格式
        GL_UNSIGNED_BYTE,			//每个颜色数据的类型
        bitmapData);				//bitmap数据指针  
}

//初始化函数
void init()
{
    //初始化纹理
    glGenTextures(10, texture);//设置纹理数组                        
    glGenTextures(6, backgrounds);
    GLint i;
    for (i = 0; i < 9; i++) {
        texload(i, (solarSystem[i].texture_path), texture);//加载对应的星球贴图
    }
    texload(9, BACKGROUND, texture);//背景图纹理
    for (i = 0; i < 6; i++) {
        texload(i, (backgrounds_path[i]), backgrounds);//加载对应的背景贴图
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //多边形的显示方式,模式将适用于物体的所有面采用填充形式
}
//公转半径计算：因为无法完全按照比例来，所以参考了国际画图标准，将地内行星的比例相对改变了
GLfloat RadiusTransform(GLfloat radius) {
    return radius > 1 ? ORBIT_RADIUS_FACTOR * sqrt(radius) : radius * ORBIT_RADIUS_FACTOR;
}
//星球自身半径换算
GLfloat RadiusTransformPlanet(GLfloat radius) {
    return radius > 1 ? sqrt(radius) : radius;
}

//计算自转角度
GLfloat get_rotation(GLfloat period) {
    return (fmod(body_rotation_self, period) * g_body_rotation_speed + g_body_rotation_phase) / period * 360 * BODY_ROTATION_FACTOR;
}
//计算公转角度
GLfloat get_rotation_2(GLfloat period) {
    return (fmod(body_rotation, period) * g_body_rotation_speed + g_body_rotation_phase) / period * 360 * BODY_ROTATION_FACTOR;
}

//矩阵相乘（没用）
void multiply_vector_by_matrix(GLdouble vector[3], GLdouble matrix[16]) {
    GLdouble x = vector[0];
    GLdouble y = vector[1];
    GLdouble z = vector[2];
    vector[0] = x * matrix[0] + y * matrix[1] + z * matrix[2] + matrix[3];
    vector[1] = x * matrix[4] + y * matrix[5] + z * matrix[6] + matrix[7];
    vector[2] = x * matrix[8] + y * matrix[9] + z * matrix[10] + matrix[11];
}


//绘制背景，保持2：1的比例
void draw_background() {
    GLfloat background_coord2f[6][4][3] = {
        {
            { 1,-1, 1},
            {-1,-1, 1},
            {-1, 1, 1},
            { 1, 1, 1}
        },
        {
            {-1,-1,-1},
            { 1,-1,-1},
            { 1, 1,-1},
            {-1, 1,-1}
        },
        {
            { 1,-1,-1},
            {-1,-1,-1},
            {-1,-1, 1},
            { 1,-1, 1}


        },
        {
            { 1, 1, 1},
            {-1, 1, 1},
            {-1, 1,-1},
            { 1, 1,-1}
        },
        {
            {-1,-1, 1},
            {-1,-1,-1},
            {-1, 1,-1},
            {-1, 1, 1}
        },
        {
            { 1,-1,-1},
            { 1,-1, 1},
            { 1, 1, 1},
            { 1, 1,-1}

        }
    };
    GLfloat TexCoord[4][2] = {
        {0.0f,0.0f},{1.0f,0.0f},{1.0f,1.0f},{0.0f,1.0f}
    };
    glEnable(GL_TEXTURE_2D);					//开启贴图
    for (int i = 0; i < 6; i++) {
        glBindTexture(GL_TEXTURE_2D, backgrounds[i]);	//绑定贴图
        glEnable(GL_TEXTURE_2D);																	//启用二维纹理
        glBegin(GL_QUADS);

        /** 指定纹理坐标和顶点坐标 */
        for (int j = 0; j < 4; j++) {
            glTexCoord2f(TexCoord[j][0], TexCoord[j][1]); glVertex3f(background_coord2f[i][j][0], background_coord2f[i][j][1], background_coord2f[i][j][2]);
        }
        glEnd();
        glDisable(GL_TEXTURE_2D);					//关闭贴图
    }
}


//更新旋转信息

//绘制星系
void Draw_Galaxy() // This function draws a triangle with RGB colors
{
    qobj = gluNewQuadric();//申请空间
    //开启光照纹理
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, grey);
    //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);

    //开启光照混合
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor3f(1.0f, 1.0f, 1.0f);


    double_t axis[3] = { 0, 0, 1 };//初始坐标轴
    for (int i = 0; i < 9; i++) {
        glPushMatrix();
        axis[0] = 0; axis[1] = 1; axis[2] = 0;//自转轴初始化
        glRotated(90.0 - solarSystem[i].orbit.inclination, 1, 0, 0);
        if (bStarRow)glutSolidTorus(ORBIT_INNER_RADIUS, RadiusTransform(solarSystem[i].orbit.radius), 10, 100);	//星环绘制
        if (i > 0)glRotatef(get_rotation_2(solarSystem[i].orbit.period), 0, 0, 1);
        glTranslatef(RadiusTransform(solarSystem[i].orbit.radius), 0, 0);							//移动到行星对应的公转轴位置
        //multiply_vector_by_matrix(axis, solarSystem[i].z_rotation_inverse);						//相乘得到太阳系轴
        glRotatef(get_rotation(solarSystem[i].period), 0, 0, 1);
        glBindTexture(GL_TEXTURE_2D, *solarSystem[i].texture_name);									//选择纹理texture[0]
        glEnable(GL_TEXTURE_2D);																	//启用二维纹理
        gluQuadricTexture(qobj, GL_TRUE);															//纹理函数
        //glRotated(get_rotation(solarSystem[i].period), axis[0], axis[1], axis[2]);				//自转角度偏移
        glRotated(solarSystem[i].tilt, 0, 0, 1);													//自转轴偏移								
        gluSphere(qobj, RadiusTransformPlanet(solarSystem[i].radius), 50, 50);						//二次曲面qobj
        glDisable(GL_TEXTURE_2D);																	//关闭二维纹理
        glPopMatrix();
    }
}


//更新视窗
void updateView(int width, int height)
{
    glViewport(0, 0, width, height);						// 更新视角

    glMatrixMode(GL_PROJECTION);						// 选择项目矩阵
    glLoadIdentity();									// 重设项目矩阵

    float whRatio = (GLfloat)width / (GLfloat)height;		//透视长宽比
    gluPerspective(45.0f, whRatio, 0.1f, 500.0f);			//透视具体设置

    glMatrixMode(GL_MODELVIEW);							// 
}
//更新视窗
void reshape(int width, int height)
{
    wWidth = width;
    wHeight = height;
    updateView(wWidth, wHeight);
}
//idle函数
void idle()
{
    glutPostRedisplay();
}

//键盘函数
void key(unsigned char k, int x, int y)
{
    switch (k) {
        case 27:
        case 'q': {exit(0); break; }						//退出
        case '1': {bRotate = !bRotate;		break;	}	//公转
        case '2': {bRotateSelf = !bRotateSelf; break;	}	//自转
        case '3': {bRotateSelf = !bRotateSelf;
            bRotate = !bRotate;	        break;	}	//自转+公转
        case '4': {bPersp = !bPersp;		break;	}	//透视
        case '5': {bAnim = !bAnim;		break;	}	//旋转
        case '6': {bWire = !bWire;		break;	}	//线框
        case '7': {bMix = !bMix;		break;	}	//光照混合模式
        case '8': {bTex = !bTex;		break;	}	//纹理
        case 'd': {											//视角变化
            R1 += 1.0;
            if (R1 > 180)R1 -= 360;
            break;
        }
        case 'a': {
            R1 -= 1.0;
            if (R1 < -180)R1 += 360;

            break;
        }
        case 'w': {
            if (R2 >= -85)R2 -= 1.0;
            break;
        }
        case 's': {
            if (R2 <= 85)R2 += 1.0;
            break;
        }
        case 'z': {
            L1 += 1;
            break;
        }
        case 'x': {
            L1 -= 1;
            break;
        }
        case 'j': {								//视角变化
            center[0] -= 0.2f;
            break;
        }
        case 'l': {
            center[0] += 0.2f;
            break;
        }
        case 'i': {
            center[1] -= 0.2f;
            break;
        }
        case 'k': {
            center[1] += 0.2f;
            break;
        }
        case 'n': {
            center[2] -= 0.2f;
            break;
        }
        case 'm': {
            center[2] += 0.2f;
            break;
        }
        case 'b': {
            speed = (speed + 1) % 3;
            body_rotation_speed = 1.0 + speed * 5.0;
            break;
        }
        case 'v': {
            speed_self = (speed_self + 1) % 3;
            body_rotation__self_speed = 1.0 + speed_self * 5.0;
            break;
        }
        case 'p': {
            snapScreen(1024, 512, "screenshot.bmp");
            break;
        }
    }
    updateView(wWidth, wHeight);
}


void Display_Galaxy()
{
    eye[1] = L1 * sin(R2 * PI / 180);
    eye[0] = L1 * cos(R2 * PI / 180) * sin(R1 * PI / 180);
    eye[2] = L1 * cos(R2 * PI / 180) * cos(R1 * PI / 180);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    //摄像机角度
    gluLookAt(eye[0], eye[1], eye[2],				// 场景（0，0，0）的视点中心 (0,5,50)，Y轴向上
        center[0], center[1], center[2],
        0, 1, 0);

    //if (bWire) {
    //	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //}
    //else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //}

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);							//灯光
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, LightWhite);
    glEnable(GL_LIGHT0);

    //背景图绘制
    glPushMatrix();
    //glTranslatef(0, 0, -90);
    glScalef(100, 100, 100);
    draw_background();
    glPopMatrix();

    //glTranslatef(0.0f, 0.0f,-6.0f);			// 移动整体
    glRotatef(fRotate, 0, 1.0f, 0);				// 旋转
    //glRotatef(-90, 1, 0, 0);					// 旋转坐标系
    glScalef(0.2, 0.2, 0.2);					//缩放

    glEnable(GL_NORMALIZE);
    Draw_Galaxy();								// 绘制星系
    if (bAnim)	fRotate += 0.1f;
    if (bRotate)body_rotation += 0.01f * body_rotation__self_speed;//公转衡量度
    if (bRotateSelf)body_rotation_self += 0.0001f * body_rotation_speed;//自转衡量度
    glutSwapBuffers();
}
void display() {
    switch (choices) {
        case 1:
            Display_Galaxy();
            break;
        case 2:
            printf("you choose the black hole\n");
            break;
        default:
            printf("what do you want to see!\n");
            break;
    }

}
void myMenu(int value)	//	glutCreateMenu需要回调的函数
{
    switch (value)	//	根据value的值来决定要执行的绘制的方法
    {
        case 0:
            exit(0);
            break;
        case 1:
            choices = 1;
            break;
        case 2:
            bRotateSelf = true;
            break;
        case 3:
            bRotate = true;
            break;
        case 4:
            speed = (speed + 1) % 3;
            body_rotation_speed = 1.0 + speed * 5.0;
            break;
        case 5:
            speed_self = (speed_self + 1) % 3;
            body_rotation__self_speed = 1.0 + speed_self * 5.0;
            break;
        case 6:
            bStarRow = !bStarRow;
            break;
        case 7:
            choices = 2;
            break;

        case 8:
            printf("black hole setting 1:");
            break;

        case 9:
            printf("black hole setting 2:");
            break;
    }
}
int MouseX;
int MouseY;
bool mouseLeftDown = false;
void myMouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) {
            MouseX = x;
            MouseY = y;
            mouseLeftDown = true;
        }
    }
    else
        mouseLeftDown = false;
}
void mouseMotionCB(int x, int y)
{

    if (mouseLeftDown)
    {
        if (y - MouseY > 0) {
            R2 += (y - MouseY) * 0.1f;
            if (R2 > 85)R2 = 85;
        }
        else {
            R2 -= (MouseY - y) * 0.1f;
            if (R2 < -85)R2 = -85;
        }
        if (x - MouseX < 0) {
            R1 += (MouseX - x) * 0.1f;
            if (R1 > 180)R1 -= 360;
        }
        else {
            R1 -= (x - MouseX) * 0.1f;
            if (R1 < -180)R1 += 360;
        }
        MouseX = x;
        MouseY = y;
    }
    //glutPostRedisplay();
}
int main(int argc, char* argv[])
{
select:
    printf("请选择场景：\n");
    printf("1. 太阳系：\n");
    printf("2. 黑洞：\n");
    int scene;
    scanf("%d", &scene);
    if (scene == 1) {

        glutInit(&argc, argv);

        glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
        glutInitWindowSize(screenWidth, screenHeight);

        int windowHandle = glutCreateWindow("计算机图形学期末项目：ZJUGalaxy");

        init();

        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutKeyboardFunc(key);
        glutIdleFunc(idle);
        //绑定菜单界面
        int MainMenu;
        int GalaxyMenu;
        int BHMenu;
        int ExitMenu;
        //二级菜单
        GalaxyMenu = glutCreateMenu(myMenu);	//	注册菜单回调函数
        glutAddMenuEntry("显示太阳系", 1);	//	添加菜单项
        glutAddMenuEntry("开始自转", 2);
        glutAddMenuEntry("开始公转", 3);
        glutAddMenuEntry("自转加速", 4);
        glutAddMenuEntry("公转加速", 5);
        glutAddMenuEntry("显示/隐藏轨迹", 6);
        BHMenu = glutCreateMenu(myMenu);
        glutAddMenuEntry("显示黑洞", 7);	//	添加菜单项
        glutAddMenuEntry("黑洞设置1", 8);
        glutAddMenuEntry("黑洞设置2", 9);
        ExitMenu = glutCreateMenu(myMenu);
        glutAddMenuEntry("退出", 0);
        //一级菜单
        MainMenu = glutCreateMenu(myMenu);
        glutAddSubMenu("太阳系设置", GalaxyMenu);
        glutAddSubMenu("黑洞设置", BHMenu);
        glutAddSubMenu("退出", ExitMenu);
        //菜单注册
        glutAttachMenu(GLUT_RIGHT_BUTTON);	//	把当前菜单注册到指定的鼠标键

        glutMouseFunc(myMouse);
        glutMotionFunc(mouseMotionCB);

        glutMainLoop();
    }
    else if (scene == 2) {
        run_blackhole_scene();
    }
    else {
        goto select;
    }

    return 0;
}


