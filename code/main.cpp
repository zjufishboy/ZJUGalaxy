//头文件
#include<stdio.h>
#include <stdlib.h>
#include<Windows.h>
#include<string>
#include <math.h>
#include "gl/glew.h"
#include "gl/glut.h"
#include "sunShader.h"

//定义
#define BITMAP_ID 0x4D42
#define TEXTURE_PATH(name) ("texture/" name ".bmp")

//行星信息结构
typedef struct planet {
	char *name;							//行星中文名
	char *texture_path;					//行星的贴图文件路径
	GLuint *texture_name;				//行星贴图编号
	GLdouble radius;					//行星相对半径
	//GLuint display_list;				//行星的展示序列//未实现
	GLdouble tilt;						// 行星自转轴偏角
	GLdouble z_rotation_inverse[16];	//未知矩阵，好像是用来算偏角的
	GLdouble period;					// 按天算的行星自转周期
	struct {
	//	GLdouble inclination;			// 公转偏角.
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

//背景图纹理路径
char *BACKGROUND = TEXTURE_PATH("milky_way");

//背景纹理编号
GLuint *background = &texture[9];

SunShader *theSunShader;

//整体相对偏移数据
float fTranslate;								//移动初始值
float fRotate;									//旋转初始值
float fScale = 1.0f;							//缩放初始值

//其他变量
GLdouble g_body_rotation_speed = 1;				//旋转速度
GLdouble g_body_rotation_phase = 0;				//旋转进度
GLfloat body_rotation_self = 0;						//自转衡量度
GLfloat body_rotation = 0;						//公转转衡量度

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

GLfloat LightRed[]		=	{ 1.0	,	0.0	,	0.0	,	1.0 };	//预设颜色
GLfloat LightWhite[]	=	{ 1.0	,	1.0	,	1.0	,	1.0 };
GLfloat light_pos[]		=	{ 0.0	,	0.0	,	0.0	,	1.0 };  //灯光位置

//行星信息            名字		纹理路径					纹理编号		半径	自转偏角	未知矩阵		自转时间			公转轨道半径 公转周期
planet SUN		=	{ "太阳"	,TEXTURE_PATH("sun")		,&texture[0],  10.9   ,7.25			,{
																								1, 0, 0, 0,
																								0, 1, 0, 0,
																								0, 0, 1, 0,
																								0, 0, 0, 1
																								  }				,7.25		,		{0			,0			}	};
planet MERCURY	=	{ "水星"	,TEXTURE_PATH("mercury")	,&texture[1],  0.3829 ,0.034		,{}				,58.646		,		{0.387098	,87.9691	}	};
planet VENUS	=	{ "金星"	,TEXTURE_PATH("venus")		,&texture[2],  0.9499 ,2.64			,{}				,-243.025	,		{0.723332   ,224.701	}	};
planet EARTH	=	{ "地球"	,TEXTURE_PATH("earth")		,&texture[3],  1      ,23.4392811	,{}				,0.99726968	,		{1			,365.256363 }	};
planet MARS		=	{ "火星"	,TEXTURE_PATH("mars")		,&texture[4],  0.5320 ,1.025957		,{}				,25.19		,		{1.523679	,686.971	}	};
planet JUPITER	=	{ "木星"	,TEXTURE_PATH("jupiter")	,&texture[5],  10.97  ,9.925 / 24	,{}				,3.13		,		{5.20260	,4332.59	}	};
planet SATURN	=	{ "土星"	,TEXTURE_PATH("saturn")		,&texture[6],  9.140  ,10.55 / 24	,{}				,26.73		,		{9.54909	,10759.22	}	};
planet URANUS	=	{ "天王星"	,TEXTURE_PATH("uranus")		,&texture[7],	3.981  ,0.71833		,{}				,97.77		,		{19.2184	,30688.5	}	};
planet NEPTUNE	=	{ "海王星"	,TEXTURE_PATH("neptune")	,&texture[8],	3.865  ,0.6713		,{}				,28.32		,		{30.110387	,60182		} 	};

//行星数组
planet solarSystem[9] = { SUN,MERCURY,VENUS,EARTH,MARS,JUPITER,SATURN,URANUS,NEPTUNE };



//



//加载文件到bit信息头文件
unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
	FILE *filePtr;						// 文件指针
	BITMAPFILEHEADER bitmapFileHeader;	// bitmap文件头
	unsigned char	*bitmapImage;		// bitmap图像数据
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
void texload(int i, char *filename)
{
	BITMAPINFOHEADER bitmapInfoHeader;                                 // bitmap信息头
	unsigned char*   bitmapData;                                       // 纹理数据

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
	GLint i;
	for(i=0;i<9;i++){
		texload(i, (solarSystem[i].texture_path));//加载对应的星球贴图
	}
	texload(9,BACKGROUND);//背景图纹理
	texload(10, "texture/sunshader.bmp");
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //多边形的显示方式,模式将适用于物体的所有面采用填充形式
}
//公转半径计算：因为无法完全按照比例来，所以参考了国际画图标准，将地内行星的比例相对改变了
GLfloat RadiusTransform(GLfloat radius) {
	return radius > 1 ? ORBIT_RADIUS_FACTOR * sqrt(radius) : radius* ORBIT_RADIUS_FACTOR;
}
//星球自身半径换算
GLfloat RadiusTransformPlanet(GLfloat radius) {
	return radius > 1 ? sqrt(radius) : radius;
}

//计算自转角度
GLfloat get_rotation(GLfloat period){
	return (fmod(body_rotation_self,period)*g_body_rotation_speed+ g_body_rotation_phase)/period*360*BODY_ROTATION_FACTOR;
}
//计算公转角度(未完成)
GLfloat get_rotation_2(GLfloat period) {
	return (fmod(body_rotation, period)*g_body_rotation_speed + g_body_rotation_phase) / period * 360 * BODY_ROTATION_FACTOR;
}

//矩阵相乘
void multiply_vector_by_matrix(GLdouble vector[3], GLdouble matrix[16]) {
	GLdouble x = vector[0];
	GLdouble y = vector[1];
	GLdouble z = vector[2];
	vector[0] = x * matrix[0] + y * matrix[1] + z * matrix[ 2] + matrix[ 3];
	vector[1] = x * matrix[4] + y * matrix[5] + z * matrix[ 6] + matrix[ 7];
	vector[2] = x * matrix[8] + y * matrix[9] + z * matrix[10] + matrix[11];
}

//绘制背景，保持2：1的比例
void draw_background() {
	glEnable(GL_TEXTURE_2D);					//开启贴图
	glBindTexture(GL_TEXTURE_2D, *background);	//绑定贴图
	glBegin(GL_QUADS);							//开始画正方形
	glTexCoord2f(0, 0);glVertex3f(-1, -0.5, 0);	//绑定四个点
	glTexCoord2f(1, 0);glVertex3f( 1, -0.5, 0);
	glTexCoord2f(1, 1);glVertex3f( 1,  0.5, 0);
	glTexCoord2f(0, 1);glVertex3f(-1,  0.5, 0);
	glEnd();									//结束
	glDisable(GL_TEXTURE_2D);					//关闭贴图
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
	

	load_texture()
	theSunShader->enable();
	theSunShader->update(resolution_3f(), time());

	
	double_t axis[3] = { 0, 0, 1 };//初始坐标轴
	for (int i = 0; i < 9; i++) {
		if (i > 1) {
			theSunShader->disable();
		}
		glPushMatrix();
		axis[0] = 0;axis[1] = 1;axis[2] = 0;//自转轴初始化
		if(i>0)glRotatef(get_rotation_2(solarSystem[i].orbit.period), 0, 0, 1);
		glTranslatef(RadiusTransform(solarSystem[i].orbit.radius), 0, 0);							//移动到行星对应的公转轴位置
		multiply_vector_by_matrix(axis, solarSystem[i].z_rotation_inverse);						//相乘得到太阳系轴
		glRotatef(get_rotation(solarSystem[i].period), 0, 0, 1);
		glBindTexture(GL_TEXTURE_2D, *solarSystem[i].texture_name);									//选择纹理texture[0]
		glEnable(GL_TEXTURE_2D);																	//启用二维纹理
		gluQuadricTexture(qobj, GL_TRUE);															//纹理函数
		//glRotated(get_rotation(solarSystem[i].period), axis[0], axis[1], axis[2]);				//自转角度偏移
		//glRotated(solarSystem[i].tilt, 0, 0, 1);													//自转轴偏移									
		//glutSolidTorus(ORBIT_INNER_RADIUS,RadiusTransform(solarSystem[i].orbit.radius),10, 100);	//星环绘制
		gluSphere(qobj, RadiusTransformPlanet(solarSystem[i].radius), 50, 50);						//二次曲面qobj
		glDisable(GL_TEXTURE_2D);																	//关闭二维纹理
		glPopMatrix();
	}


}


//更新视窗
void updateView(int width, int height)
{
	glViewport(0,0,width,height);						// 更新视角

	glMatrixMode(GL_PROJECTION);						// 选择项目矩阵
	glLoadIdentity();									// 重设项目矩阵

	float whRatio = (GLfloat)width/(GLfloat)height;		//透视长宽比
	gluPerspective(45.0f, whRatio,0.1f,500.0f);			//透视具体设置

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
	switch(k){
		case 27:
		case 'q': {exit(0); break; }						//退出
		case '1': {bRotate		= !bRotate;		break;	}	//公转
		case '2': {bRotateSelf	= !bRotateSelf; break;	}	//自转
		case '3': {bRotateSelf = !bRotateSelf; 
				   bRotate = !bRotate;	        break;	}	//自转+公转
		case '4': {bPersp		= !bPersp;		break;	}	//透视
		case '5': {bAnim		= !bAnim;		break;	}	//旋转
		case '6': {bWire		= !bWire;		break;	}	//线框
		case '7': {bMix			= !bMix;		break;	}	//光照混合模式
		case '8': {bTex			= !bTex;		break;	}	//纹理
		case 'a': {											//视角变化
			eye[0] -= 0.2f;
			//center[0] -= 0.2f;
			break;
				  }
		case 'd': {
			eye[0] += 0.2f;
			//center[0] += 0.2f;
			break;
				  }
		case 'w': {
			eye[1] -= 0.2f;
			//center[1] -= 0.2f;
			break;
				  }
		case 's': {
			eye[1] += 0.2f;
			//center[1] += 0.2f;
			break;
				  }
		case 'z': {
			eye[2] -= 0.2f;
			//center[2] -= 0.2f;
			break;
				  }
		case 'x': {
			eye[2] += 0.2f;
			//center[2] += 0.2f;
			break;
				  }
		case 'j': {								//视角变化
			//eye[0] -= 0.2f;
			center[0] -= 0.2f;
			break;
		}
		case 'l': {
			//eye[0] += 0.2f;
			center[0] += 0.2f;
			break;
		}
		case 'i': {
			//eye[1] -= 0.2f;
			center[1] -= 0.2f;
			break;
		}
		case 'k': {
			//eye[1] += 0.2f;
			center[1] += 0.2f;
			break;
		}
		case 'n': {
			//eye[2] -= 0.2f;
			center[2] -= 0.2f;
			break;
		}
		case 'm': {
			//eye[2] += 0.2f;
			center[2] += 0.2f;
			break;
		}
	}
	updateView(wWidth, wHeight);
}


void Display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();									
	//摄像机角度
	gluLookAt(	eye[0], eye[1], eye[2],				// 场景（0，0，0）的视点中心 (0,5,50)，Y轴向上
				center[0], center[1], center[2],
				0, 1, 0);				

	if (bWire) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);							//灯光
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos	);
	glLightfv(GL_LIGHT0, GL_AMBIENT , LightWhite		);
	glEnable(GL_LIGHT0);
	
	//背景图绘制
	glPushMatrix();
	glTranslatef(0, 0, -90);
	glScalef(360, 360, 1);
	draw_background();					
	glPopMatrix();

	//glTranslatef(0.0f, 0.0f,-6.0f);			// 移动整体
	glRotatef(fRotate, 0, 1.0f, 0);				// 旋转
	glRotatef(-90, 1, 0, 0);					// 旋转坐标系
	glScalef(0.2, 0.2, 0.2);					//缩放
   
	glEnable(GL_NORMALIZE);
	Draw_Galaxy();								// 绘制星系
	if (bAnim)	fRotate += 0.1f;
	if (bRotate)body_rotation		   += 0.01f;//公转衡量度
	if (bRotateSelf)body_rotation_self += 0.01f;//自转衡量度
	glutSwapBuffers();							
}


int main(int argc,  char *argv[])
{
	
	glutInit(&argc, argv);


	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(1024,512);
	
	int windowHandle = glutCreateWindow("计算机图形学期末项目：ZJUGalaxy");

	init();

	glutDisplayFunc(Display);
	glutReshapeFunc(reshape);	
	glutKeyboardFunc(key);
	glutIdleFunc(idle);

	glewInit();
	theSunShader = new SunShader;
	glutMainLoop();
	
	return 0;
}


