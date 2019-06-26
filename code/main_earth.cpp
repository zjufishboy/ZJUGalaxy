#include <stdlib.h>
#include "glut.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include "simplexnoise.h"
#include "main_earth.h"

#define PI 3.1415926535;
//����glut�в�û����ά�����������Լ�������һ��
class vector3
{
public:
	float x;
	float y;
	float z;
	vector3(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}
	vector3(const vector3 &a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
	}
	vector3()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	void operator=(const vector3 &a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
	}
	vector3 operator*(const float &a)
	{
		vector3 b;
		b.x = a * x;
		b.y = a * y;
		b.z = a * z;
		return b;
	}

	vector3 operator+(const vector3 &a)
	{
		vector3 b;
		b.x = a.x + x;
		b.y = a.y + y;
		b.z = a.z + z;
		return b;
	}
	float operator*(const vector3 &a)
	{
		float b;
		b = a.x * x + a.y * y + a.z * z;
		return b;
	}
	vector3 operator^(const vector3 &a)
	{
		vector3 b;
		b.x = y * a.z - z * a.y;
		b.y = z * a.x - x * a.z;
		b.z = x * a.y - y * a.x;
		return b;
	}
	vector3 unitize()
	{
		float s = (1 / size());
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	float size()
	{
		return sqrt(x * x + y * y + z * z);
	}
};
float max(float a, float b)
{
	return a > b ? a : b;
}
//������������֮��ļнǣ����ػ�����
float calang(vector3 a, vector3 b)
{
	float t;
	t = (a * b) / a.size() / b.size();
	return acos(t);
}

//һЩ����
static float eye[] = {5, 0, 0};
static float center[] = {0, 0, 0};
static float up[] = {0, 0, 1};

GLfloat white[] = {1, 1, 1, 1};
GLfloat black[] = {0, 0, 0, 1};
static GLfloat light_pos[] = {0, 20, 10, 0};

static const float HEIGHT_MAX = 0.0550f; //��������ķ�Χ
static const float HEIGHT_MIN = -0.0575f;

bool isrotate = false;
float frotate = 0.02;
//z����ת�Ƕ�
float zanrg = 0;
//y����ת�Ƕ�
float yanrg = 0;
//�۲����z����ת�Ƕ�
float anrg = 0;
//z��
vector3 zaxis = vector3(0, 0, 1);
//y��
vector3 yaxis = vector3(0, 0, 1);

//������
static bool mouseLeftDown;
static bool mouseRightDown;
static bool mouseMiddleDown;
float mouseX, mouseY;
//�۲�����������
float distance = 1.0;

void map(vector3 &position);
float calang(vector3 a, vector3 b);
vector3 calcolor(float a);
vector3 calatomcolor(vector3 v);
float calheight(vector3 &position);
vector3 rotate(vector3 a, vector3 b, float ang);

//����glut�Դ��������޸����꣬�����Լ���������ӳ��ʵ����һ����
class sphere
{
public:
	int cut;
	float rad;
	float arad;
	vector3 *vexy1; //�����������������꣬ÿ����һ������
	vector3 *vexy2;
	vector3 *veyz1;
	vector3 *veyz2;
	vector3 *vexz1;
	vector3 *vexz2;
	vector3 *cxy1; //����������������ɫֵ
	vector3 *cxy2;
	vector3 *cyz1;
	vector3 *cyz2;
	vector3 *cxz1;
	vector3 *cxz2;
	vector3 *avexy1; //����������������꣬ÿ����һ������
	vector3 *avexy2;
	vector3 *aveyz1;
	vector3 *aveyz2;
	vector3 *avexz1;
	vector3 *avexz2;
	vector3 *acxy1; //���������������ɫֵ
	vector3 *acxy2;
	vector3 *acyz1;
	vector3 *acyz2;
	vector3 *acxz1;
	vector3 *acxz2;

	sphere(int n, float r)
	{
		rad = r;
		cut = n;
		arad = r * 1.3;
		vexy1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		vexy2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		veyz1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		veyz2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		vexz1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		vexz2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		cxy1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		cxy2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		cyz1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		cyz2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		cxz1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		cxz2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		avexy1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		avexy2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		aveyz1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		aveyz2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		avexz1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		avexz2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		acxy1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		acxy2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		acyz1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		acyz2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		acxz1 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		acxz2 = (vector3 *)malloc(sizeof(vector3) * cut * cut);
		//set_seed(1);
	}
	void cubecal();
	void cubedraw();
	void facedraw(vector3 *a, vector3 *c);
};
//��ʼ����
sphere s = sphere(100, 1);

//���������������ӳ��������������
void map(vector3 &position)
{
	float x2 = position.x * position.x;
	float y2 = position.y * position.y;
	float z2 = position.z * position.z;
	position.x = position.x * sqrt(1.0f - (y2 * 0.5f) - (z2 * 0.5f) + ((y2 * z2) / 3.0f));
	position.y = position.y * sqrt(1.0f - (z2 * 0.5f) - (x2 * 0.5f) + ((z2 * x2) / 3.0f));
	position.z = position.z * sqrt(1.0f - (x2 * 0.5f) - (y2 * 0.5f) + ((x2 * y2) / 3.0f));
}
//����ø߶ȱ仯��Ӧ�ñ�ʾ����ɫ
vector3 calcolor(float a)
{
	vector3 color;
	if (a < 0.0)
	{ //С��0Ϊ��������ֵԽ����ɫԽ��
		a = fabs(a);
		color.x = color.y = 0.1;
		float i = (1 - a / fabs(HEIGHT_MIN));
		if (i < 0.05)
		{ //��ֹ��ɫ����
			i = 0.05;
		}
		color.z = i * 0.5; //��С��ɫ��Χ������Ч��
	}
	else
	{ //����0Ϊ½�أ�����ֵԽ����ɫԽ��
		a = fabs(a);
		color.x = color.z = 0.1;
		float i = (1 - a / fabs(HEIGHT_MAX));
		if (i < 0.05)
		{
			i = 0.05;
		}
		color.y = i * 0.3;
	}
	return color;
}

vector3 calatomcolor(vector3 v)
{ //�������Ӧ�ñ�ʾ����ɫ
	vector3 color;
	vector3 light = vector3(light_pos[0], light_pos[1], light_pos[2]);
	vector3 view = vector3(eye[0] - center[0], eye[1] - center[1], eye[2] - center[2]);

	float diffuse1 = 1.0, diffuse2 = 1.0;
	diffuse1 = pow(max((light * v) / light.size() / v.size(), 0), 1.5); //���Դ�нǴ�����˥��
	diffuse2 = pow(max(sin(calang(view, v)), 0), 3.5);					//�����߼нǴ�����˥��
	color.x = 1 * diffuse1 * diffuse2;
	color.y = 1 * diffuse1 * diffuse2;
	color.z = 1 * diffuse1 * diffuse2;
	return color;
}

float calheight(vector3 &position)
{ //ͨ�������������ɸ߶��Ŷ�

	vector3 vNormalFromCenter = position;

	static const float NOISE_PERSISTENCE = 0.6f;
	static const float NOISE_OCTAVES = 8.0f;
	static const float NOISE_SCALE = 1.0f;

	float fNoise = scaled_octave_noise_3d(NOISE_OCTAVES, NOISE_PERSISTENCE, NOISE_SCALE, HEIGHT_MIN, HEIGHT_MAX, position.x, position.y, position.z);

	if (fNoise >= 0)
	{
		position = position + vNormalFromCenter * fNoise;
	}

	return fNoise;
}

vector3 rotate(vector3 a, vector3 b, float ang)
{ //��ά�����ƶ�����ת
	vector3 c;
	c = a * (float)cos(ang) + (b ^ a) * (float)sin(ang) + b * (float)(1 - cos(ang)) * (a * b);
	return c;
}

void sphere::cubecal()
{ //����������������ɫֵ,�Լ��������������ɫ
	vector3 minv(-1.0, -1.0, -1.0);
	for (int i = 0; i < cut; i++)
	{
		for (int j = 0; j < cut; j++)
		{
			vector3 position = minv;
			position.x += (float)j / (float)(cut - 1) * 2.0f;
			position.y += (float)i / (float)(cut - 1) * 2.0f;
			map(position);
			position = position * rad;
			cxy1[i * cut + j] = calcolor(calheight(position));
			vexy1[i * cut + j] = position;

			vector3 aposition = position.unitize() * arad;
			avexy1[i * cut + j] = aposition;
			acxy1[i * cut + j] = calatomcolor(aposition);
		}
	}
	minv = vector3(-1.0, -1.0, 1.0);
	for (int y = 0; y < cut; y++)
	{
		for (int x = 0; x < cut; x++)
		{
			vector3 position = minv;
			position.x += (float)x / (float)(cut - 1) * 2.0f;
			position.y += (float)y / (float)(cut - 1) * 2.0f;
			map(position);
			position = position * rad;
			cxy2[y * cut + x] = calcolor(calheight(position));
			vexy2[y * cut + x] = position;

			vector3 aposition = position.unitize() * arad;
			avexy2[y * cut + x] = aposition;
			acxy2[y * cut + x] = calatomcolor(aposition);
		}
	}
	minv = vector3(-1.0, -1.0, -1.0);
	for (int i = 0; i < cut; i++)
	{
		for (int j = 0; j < cut; j++)
		{
			vector3 position = minv;
			position.y += (float)j / (float)(cut - 1) * 2.0f;
			position.z += (float)i / (float)(cut - 1) * 2.0f;
			map(position);
			position = position * rad;
			cyz1[i * cut + j] = calcolor(calheight(position));
			veyz1[i * cut + j] = position;

			vector3 aposition = position.unitize() * arad;
			aveyz1[i * cut + j] = aposition;
			acyz1[i * cut + j] = calatomcolor(aposition);
		}
	}
	minv = vector3(1.0, -1.0, -1.0);
	for (int i = 0; i < cut; i++)
	{
		for (int j = 0; j < cut; j++)
		{
			vector3 position = minv;
			position.y += (float)j / (float)(cut - 1) * 2.0f;
			position.z += (float)i / (float)(cut - 1) * 2.0f;
			map(position);
			position = position * rad;
			cyz2[i * cut + j] = calcolor(calheight(position));
			veyz2[i * cut + j] = position;

			vector3 aposition = position.unitize() * arad;
			aveyz2[i * cut + j] = aposition;
			acyz2[i * cut + j] = calatomcolor(aposition);
		}
	}
	minv = vector3(-1.0, -1.0, -1.0);
	for (int i = 0; i < cut; i++)
	{
		for (int j = 0; j < cut; j++)
		{
			vector3 position = minv;
			position.x += (float)j / (float)(cut - 1) * 2.0f;
			position.z += (float)i / (float)(cut - 1) * 2.0f;
			map(position);
			position = position * rad;
			cxz1[i * cut + j] = calcolor(calheight(position));
			vexz1[i * cut + j] = position;

			vector3 aposition = position.unitize() * arad;
			avexz1[i * cut + j] = aposition;
			acxz1[i * cut + j] = calatomcolor(aposition);
		}
	}
	minv = vector3(-1.0, 1.0, -1.0);
	for (int i = 0; i < cut; i++)
	{
		for (int j = 0; j < cut; j++)
		{
			vector3 position = minv;
			position.x += (float)j / (float)(cut - 1) * 2.0f;
			position.z += (float)i / (float)(cut - 1) * 2.0f;
			map(position);
			position = position * rad;
			cxz2[i * cut + j] = calcolor(calheight(position));
			vexz2[i * cut + j] = position;

			vector3 aposition = position.unitize() * arad;
			avexz2[i * cut + j] = aposition;
			acxz2[i * cut + j] = calatomcolor(aposition);
		}
	}
}

void sphere::cubedraw()
{ //������
	facedraw(vexy1, cxy1);
	facedraw(vexy2, cxy2);
	facedraw(veyz1, cyz1);
	facedraw(veyz2, cyz2);
	facedraw(vexz1, cxz1);
	facedraw(vexz2, cxz2);
}

void sphere::facedraw(vector3 *a, vector3 *c)
{ //��ÿ����
	vector3 *vex = a;
	vector3 v1;
	vector3 v2;
	vector3 v3;
	vector3 v4;

	float color[4] = {0, 0, 0, 1};
	float sky[4] = {1, 1, 1, 0.1};
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	for (int y = 0; y < cut - 1; y++)
	{
		for (int x = 0; x < cut - 1; x++)
		{

			v1 = rotate(vex[y * cut + x], zaxis, zanrg); //���㶥����y���z���ϵ���ת
			v1 = rotate(v1, yaxis, yanrg);
			v2 = rotate(vex[y * cut + (x + 1) % cut], zaxis, zanrg);
			v2 = rotate(v2, yaxis, yanrg);
			v3 = rotate(vex[(y + 1) % cut * cut + (x + 1) % cut], zaxis, zanrg);
			v3 = rotate(v3, yaxis, yanrg);
			v4 = rotate(vex[(y + 1) % cut * cut + x % cut], zaxis, zanrg);
			v4 = rotate(v4, yaxis, yanrg);

			color[0] = c[y * cut + x].x;
			color[1] = c[y * cut + x].y;
			color[2] = c[y * cut + x].z;
			color[3] = 1;

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);

			glMaterialf(GL_FRONT, GL_SHININESS, 10.0);

			glBegin(GL_QUADS);
			glNormal3f(v1.x, v1.y, v1.z);
			glVertex3f(v1.x, v1.y, v1.z);
			glNormal3f(v2.x, v2.y, v2.z);
			glVertex3f(v2.x, v2.y, v2.z);
			glNormal3f(v3.x, v3.y, v3.z);
			glVertex3f(v3.x, v3.y, v3.z);
			glNormal3f(v4.x, v4.y, v4.z);
			glVertex3f(v4.x, v4.y, v4.z);
			glEnd();
			
			vector3 light = vector3(light_pos[0], light_pos[1], light_pos[2]);
			vector3 view = vector3(eye[0] - center[0], eye[1] - center[1], eye[2] - center[2]);
			v1 = v1.unitize() * 1.3; //�����뾶��������
			v2 = v2.unitize() * 1.3;
			v3 = v3.unitize() * 1.3;
			v4 = v4.unitize() * 1.3;

			float diffuse1 = 1.0, diffuse2 = 1.0;
			diffuse1 = pow(max((light * v1) / light.size() / v1.size(), 0), 1.5); //���Դ�нǴ�����˥��
			diffuse2 = pow(max(sin(calang(view, v1)), 0), 3.5);					  //�����߼нǴ�����˥��
			sky[0] = 1 * diffuse1 * diffuse2;
			sky[1] = 1 * diffuse1 * diffuse2;
			sky[2] = 1 * diffuse1 * diffuse2;
			sky[3] = 0.1;

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, sky);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, sky);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,sky);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, sky);
			//glMaterialf(GL_FRONT, GL_SHININESS, 1.0);
			glBegin(GL_QUADS);
			glVertex3f(v1.x, v1.y, v1.z);
			glVertex3f(v2.x, v2.y, v2.z);
			glVertex3f(v3.x, v3.y, v3.z);
			glVertex3f(v4.x, v4.y, v4.z);
			glEnd();
			
		}
	}
}

void draw()
{ //�Դ������壬δʹ��

	GLUquadric *qobj = gluNewQuadric();
	gluQuadricTexture(qobj, GL_TRUE);
	gluQuadricDrawStyle(qobj, GLU_FILL);
	gluQuadricNormals(qobj, GLU_SMOOTH);
	glPushMatrix();
	glRotatef(.0, 0, 0, 0);
	gluSphere(qobj, 1, 500, 1000);
	glPopMatrix();
	gluDeleteQuadric(qobj);
}

static void reshape(int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float whRatio = (GLfloat)width / (GLfloat)height;
	gluPerspective(45, whRatio, 1, 1000);

	glMatrixMode(GL_MODELVIEW);
}

static void idle()
{
	glutPostRedisplay();
}

static void key(unsigned char k, int x, int y)
{
	switch (k)
	{
	case 27:
	case ' ':
		isrotate = !isrotate;
		break;
	default:
		break;
	}
}

static void mouse(int button, int state, int x, int y)
{
	mouseX = x;
	mouseY = y;

	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			mouseLeftDown = true;
		}
		else if (state == GLUT_UP)
			mouseLeftDown = false;
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			mouseRightDown = true;
		}
		else if (state == GLUT_UP)
			mouseRightDown = false;
	}
}

void mouseMotion(int x, int y)
{
	if (mouseLeftDown)
	{
		anrg -= (x - mouseX) * 0.005;
		float r = eye[0] * eye[0] + eye[1] * eye[1] + eye[2] * eye[2];
		eye[0] = sqrt(r) * cos(anrg);
		eye[1] = sqrt(r) * sin(anrg);
		mouseX = x;
		mouseY = y;
	}
	if (mouseRightDown)
	{
		distance = 1 - (x - mouseX) * 0.002f;
		eye[0] = eye[0] * distance;
		eye[1] = eye[1] * distance;
		eye[2] = eye[2] * distance;
		mouseY = y;
		mouseX = x;
	}
	glutPostRedisplay();
}

static void redraw()
{

	static int frames, last_time;
	auto ticks = clock();
	if (frames == 0)
		last_time = ticks;
	frames++;
	if (ticks - last_time >= 500)
	{
		printf("FPS: %.2f\n", 1000.0 * frames / (ticks - last_time));
		frames = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);
	glLoadIdentity();

	gluLookAt(eye[0], eye[1], eye[2],
			  center[0], center[1], center[2],
			  up[0], up[1], up[2]);

	//glEnable(GLUT_MULTISAMPLE);//��������ݣ�����û��
	/*
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	*/
	/*
	glEnable(GL_FOG);//������Ҳ�ܴﵽ���Ƶ�����Ч��
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR,white);
	glFogf(GL_FOG_DENSITY, 0.5);
	glHint(GL_FOG_HINT, GL_DONT_CARE);
	glFogf(GL_FOG_START, eye[0]-1);
	glFogf(GL_FOG_END, eye[0]+1);
	*/

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	float ambient[] = {0.05, 0.05, 0.05, 1};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glEnable(GL_LIGHT0);

	if (isrotate)
	{
		zanrg += frotate;
		if (zanrg > 2 * 3.1415926535)
			zanrg -= 2 * PI;
	}
	s.cubedraw();
	glutSwapBuffers();
}

static void init()
{
	s.cubecal();
}

void main_earth()
{
	char* argv = "";
    int argc = 0;
	glutInit(&argc, &argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(720, 720);
	int windowHandle = glutCreateWindow("test");
	init();
	glutDisplayFunc(redraw);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutIdleFunc(idle);
	glutMainLoop();
}
