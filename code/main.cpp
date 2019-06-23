//ͷ�ļ�
#include<stdio.h>
#include <stdlib.h>
#include<Windows.h>
#include<string>
#include <math.h>
#include "gl/glew.h"
#include "gl/glut.h"
#include "sunShader.h"

//����
#define BITMAP_ID 0x4D42
#define TEXTURE_PATH(name) ("texture/" name ".bmp")

//������Ϣ�ṹ
typedef struct planet {
	char *name;							//����������
	char *texture_path;					//���ǵ���ͼ�ļ�·��
	GLuint *texture_name;				//������ͼ���
	GLdouble radius;					//������԰뾶
	//GLuint display_list;				//���ǵ�չʾ����//δʵ��
	GLdouble tilt;						// ������ת��ƫ��
	GLdouble z_rotation_inverse[16];	//δ֪���󣬺�����������ƫ�ǵ�
	GLdouble period;					// �������������ת����
	struct {
	//	GLdouble inclination;			// ��תƫ��.
		GLdouble radius;				//���ǵĹ���뾶.
	//	GLuint display_list;			//��ת���չʾ�б�
		GLdouble period;				// ��ת����
	} orbit;
	//struct body_t *planets[];			//���������ָ������
} planet;

//��������
GLuint texture[10];

//һЩ����������Ϣ
const GLdouble ORBIT_RADIUS_FACTOR = 10;		//���ǹ�ת����뾶����
const GLdouble ORBIT_INNER_RADIUS = 0.02;		//���ǹ�����뾶����
const GLdouble BODY_ROTATION_FACTOR = 20;		//������ת�ǶȲ���
const GLdouble BODY_ROTATION_SPEED_FACTOR = 1;	//������ת�ٶȲ���

//����ͼ����·��
char *BACKGROUND = TEXTURE_PATH("milky_way");

//����������
GLuint *background = &texture[9];

SunShader *theSunShader;

//�������ƫ������
float fTranslate;								//�ƶ���ʼֵ
float fRotate;									//��ת��ʼֵ
float fScale = 1.0f;							//���ų�ʼֵ

//��������
GLdouble g_body_rotation_speed = 1;				//��ת�ٶ�
GLdouble g_body_rotation_phase = 0;				//��ת����
GLfloat body_rotation_self = 0;						//��ת������
GLfloat body_rotation = 0;						//��תת������

//����������������
GLUquadricObj* qobj;


//�������ز������м���û���ϣ�
bool bPersp = false;	 //͸��
bool bAnim = false;		 //��ת
bool bWire = false;		 //�߿�
bool bMix = false;		 //��Ϲ���������
bool bTex = false;		 //��ʾ����
bool bRotateSelf = false;//��ת
bool bRotate = false;    //��ת

//Ԥ����ɫ
float white[] = { 1.0f, 1.0f,1.0f, 1.0f };
float grey[] = { 0.5f,0.5f,0.5f,1.0f };
float green[] = { 0.0f,1.0f,0.0f,1.0f };

//�۲����۲�����
float eye[] = { 0, 0, 8 };
float center[] = { 0, 0, 0 };

//������Ϣ
int wWidth = 0;
int wHeight = 0;

//�ƹ���Ϣ

GLfloat LightRed[]		=	{ 1.0	,	0.0	,	0.0	,	1.0 };	//Ԥ����ɫ
GLfloat LightWhite[]	=	{ 1.0	,	1.0	,	1.0	,	1.0 };
GLfloat light_pos[]		=	{ 0.0	,	0.0	,	0.0	,	1.0 };  //�ƹ�λ��

//������Ϣ            ����		����·��					������		�뾶	��תƫ��	δ֪����		��תʱ��			��ת����뾶 ��ת����
planet SUN		=	{ "̫��"	,TEXTURE_PATH("sun")		,&texture[0],  10.9   ,7.25			,{
																								1, 0, 0, 0,
																								0, 1, 0, 0,
																								0, 0, 1, 0,
																								0, 0, 0, 1
																								  }				,7.25		,		{0			,0			}	};
planet MERCURY	=	{ "ˮ��"	,TEXTURE_PATH("mercury")	,&texture[1],  0.3829 ,0.034		,{}				,58.646		,		{0.387098	,87.9691	}	};
planet VENUS	=	{ "����"	,TEXTURE_PATH("venus")		,&texture[2],  0.9499 ,2.64			,{}				,-243.025	,		{0.723332   ,224.701	}	};
planet EARTH	=	{ "����"	,TEXTURE_PATH("earth")		,&texture[3],  1      ,23.4392811	,{}				,0.99726968	,		{1			,365.256363 }	};
planet MARS		=	{ "����"	,TEXTURE_PATH("mars")		,&texture[4],  0.5320 ,1.025957		,{}				,25.19		,		{1.523679	,686.971	}	};
planet JUPITER	=	{ "ľ��"	,TEXTURE_PATH("jupiter")	,&texture[5],  10.97  ,9.925 / 24	,{}				,3.13		,		{5.20260	,4332.59	}	};
planet SATURN	=	{ "����"	,TEXTURE_PATH("saturn")		,&texture[6],  9.140  ,10.55 / 24	,{}				,26.73		,		{9.54909	,10759.22	}	};
planet URANUS	=	{ "������"	,TEXTURE_PATH("uranus")		,&texture[7],	3.981  ,0.71833		,{}				,97.77		,		{19.2184	,30688.5	}	};
planet NEPTUNE	=	{ "������"	,TEXTURE_PATH("neptune")	,&texture[8],	3.865  ,0.6713		,{}				,28.32		,		{30.110387	,60182		} 	};

//��������
planet solarSystem[9] = { SUN,MERCURY,VENUS,EARTH,MARS,JUPITER,SATURN,URANUS,NEPTUNE };



//



//�����ļ���bit��Ϣͷ�ļ�
unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
	FILE *filePtr;						// �ļ�ָ��
	BITMAPFILEHEADER bitmapFileHeader;	// bitmap�ļ�ͷ
	unsigned char	*bitmapImage;		// bitmapͼ������
	int	imageIdx = 0;					// ͼ��λ������
	unsigned char	tempRGB;			// ��������
	// �ԡ�������+����ģʽ���ļ�filename 
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL) {
		return NULL;
	}
	// ����bitmap�ļ�ͼ
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	// ��֤�Ƿ�Ϊbitmap�ļ�
	if (bitmapFileHeader.bfType != BITMAP_ID) {
		fprintf(stderr, "Error in LoadBitmapFile: the file is not a bitmap file\n");
		return NULL;
	}
	// ����bitmap��Ϣͷ
	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	// ���ļ�ָ������bitmap����
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
	// Ϊװ��ͼ�����ݴ����㹻���ڴ�
	bitmapImage = new unsigned char[bitmapInfoHeader->biSizeImage];
	// ��֤�ڴ��Ƿ񴴽��ɹ�
	if (!bitmapImage) {
		fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
		return NULL;
	}
	// ����bitmapͼ������
	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
	// ȷ�϶���ɹ�
	if (bitmapImage == NULL) {
		fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
		return NULL;
	}
	//����bitmap�б���ĸ�ʽ��BGR�����潻��R��B��ֵ���õ�RGB��ʽ
	for (imageIdx = 0;
		imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}
	// �ر�bitmapͼ���ļ�
	fclose(filePtr);
	return bitmapImage;
}
//��������ͼƬ����������
void texload(int i, char *filename)
{
	BITMAPINFOHEADER bitmapInfoHeader;                                 // bitmap��Ϣͷ
	unsigned char*   bitmapData;                                       // ��������

	bitmapData = LoadBitmapFile(filename, &bitmapInfoHeader);
	glBindTexture(GL_TEXTURE_2D, texture[i]);
	// ָ����ǰ����ķŴ�/��С���˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0, 							//mipmap���(ͨ��Ϊ����ʾ���ϲ�) 
		GL_RGB,						//����ϣ���������к졢�̡�������
		bitmapInfoHeader.biWidth,	//��������������n�����б߿�+2 
		bitmapInfoHeader.biHeight,	//����߶ȣ�������n�����б߿�+2 
		0,							//�߿�(0=�ޱ߿�, 1=�б߿�) 
		GL_RGB,						//bitmap���ݵĸ�ʽ
		GL_UNSIGNED_BYTE,			//ÿ����ɫ���ݵ�����
		bitmapData);				//bitmap����ָ��  
}

//��ʼ������
void init()
{
	//��ʼ������
	glGenTextures(10, texture);//������������                        
	GLint i;
	for(i=0;i<9;i++){
		texload(i, (solarSystem[i].texture_path));//���ض�Ӧ��������ͼ
	}
	texload(9,BACKGROUND);//����ͼ����
	texload(10, "texture/sunshader.bmp");
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //����ε���ʾ��ʽ,ģʽ���������������������������ʽ
}
//��ת�뾶���㣺��Ϊ�޷���ȫ���ձ����������Բο��˹��ʻ�ͼ��׼�����������ǵı�����Ըı���
GLfloat RadiusTransform(GLfloat radius) {
	return radius > 1 ? ORBIT_RADIUS_FACTOR * sqrt(radius) : radius* ORBIT_RADIUS_FACTOR;
}
//��������뾶����
GLfloat RadiusTransformPlanet(GLfloat radius) {
	return radius > 1 ? sqrt(radius) : radius;
}

//������ת�Ƕ�
GLfloat get_rotation(GLfloat period){
	return (fmod(body_rotation_self,period)*g_body_rotation_speed+ g_body_rotation_phase)/period*360*BODY_ROTATION_FACTOR;
}
//���㹫ת�Ƕ�(δ���)
GLfloat get_rotation_2(GLfloat period) {
	return (fmod(body_rotation, period)*g_body_rotation_speed + g_body_rotation_phase) / period * 360 * BODY_ROTATION_FACTOR;
}

//�������
void multiply_vector_by_matrix(GLdouble vector[3], GLdouble matrix[16]) {
	GLdouble x = vector[0];
	GLdouble y = vector[1];
	GLdouble z = vector[2];
	vector[0] = x * matrix[0] + y * matrix[1] + z * matrix[ 2] + matrix[ 3];
	vector[1] = x * matrix[4] + y * matrix[5] + z * matrix[ 6] + matrix[ 7];
	vector[2] = x * matrix[8] + y * matrix[9] + z * matrix[10] + matrix[11];
}

//���Ʊ���������2��1�ı���
void draw_background() {
	glEnable(GL_TEXTURE_2D);					//������ͼ
	glBindTexture(GL_TEXTURE_2D, *background);	//����ͼ
	glBegin(GL_QUADS);							//��ʼ��������
	glTexCoord2f(0, 0);glVertex3f(-1, -0.5, 0);	//���ĸ���
	glTexCoord2f(1, 0);glVertex3f( 1, -0.5, 0);
	glTexCoord2f(1, 1);glVertex3f( 1,  0.5, 0);
	glTexCoord2f(0, 1);glVertex3f(-1,  0.5, 0);
	glEnd();									//����
	glDisable(GL_TEXTURE_2D);					//�ر���ͼ
}

//������ת��Ϣ

//������ϵ
void Draw_Galaxy() // This function draws a triangle with RGB colors
{
	qobj = gluNewQuadric();//����ռ�
	//������������
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, grey);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);

	//�������ջ��
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor3f(1.0f, 1.0f, 1.0f);
	

	load_texture()
	theSunShader->enable();
	theSunShader->update(resolution_3f(), time());

	
	double_t axis[3] = { 0, 0, 1 };//��ʼ������
	for (int i = 0; i < 9; i++) {
		if (i > 1) {
			theSunShader->disable();
		}
		glPushMatrix();
		axis[0] = 0;axis[1] = 1;axis[2] = 0;//��ת���ʼ��
		if(i>0)glRotatef(get_rotation_2(solarSystem[i].orbit.period), 0, 0, 1);
		glTranslatef(RadiusTransform(solarSystem[i].orbit.radius), 0, 0);							//�ƶ������Ƕ�Ӧ�Ĺ�ת��λ��
		multiply_vector_by_matrix(axis, solarSystem[i].z_rotation_inverse);						//��˵õ�̫��ϵ��
		glRotatef(get_rotation(solarSystem[i].period), 0, 0, 1);
		glBindTexture(GL_TEXTURE_2D, *solarSystem[i].texture_name);									//ѡ������texture[0]
		glEnable(GL_TEXTURE_2D);																	//���ö�ά����
		gluQuadricTexture(qobj, GL_TRUE);															//������
		//glRotated(get_rotation(solarSystem[i].period), axis[0], axis[1], axis[2]);				//��ת�Ƕ�ƫ��
		//glRotated(solarSystem[i].tilt, 0, 0, 1);													//��ת��ƫ��									
		//glutSolidTorus(ORBIT_INNER_RADIUS,RadiusTransform(solarSystem[i].orbit.radius),10, 100);	//�ǻ�����
		gluSphere(qobj, RadiusTransformPlanet(solarSystem[i].radius), 50, 50);						//��������qobj
		glDisable(GL_TEXTURE_2D);																	//�رն�ά����
		glPopMatrix();
	}


}


//�����Ӵ�
void updateView(int width, int height)
{
	glViewport(0,0,width,height);						// �����ӽ�

	glMatrixMode(GL_PROJECTION);						// ѡ����Ŀ����
	glLoadIdentity();									// ������Ŀ����

	float whRatio = (GLfloat)width/(GLfloat)height;		//͸�ӳ����
	gluPerspective(45.0f, whRatio,0.1f,500.0f);			//͸�Ӿ�������

	glMatrixMode(GL_MODELVIEW);							// 
}
//�����Ӵ�
void reshape(int width, int height)
{
	wWidth = width;
	wHeight = height;
	updateView(wWidth, wHeight);
}
//idle����
void idle()
{
	glutPostRedisplay();
}

//���̺���
void key(unsigned char k, int x, int y)
{
	switch(k){
		case 27:
		case 'q': {exit(0); break; }						//�˳�
		case '1': {bRotate		= !bRotate;		break;	}	//��ת
		case '2': {bRotateSelf	= !bRotateSelf; break;	}	//��ת
		case '3': {bRotateSelf = !bRotateSelf; 
				   bRotate = !bRotate;	        break;	}	//��ת+��ת
		case '4': {bPersp		= !bPersp;		break;	}	//͸��
		case '5': {bAnim		= !bAnim;		break;	}	//��ת
		case '6': {bWire		= !bWire;		break;	}	//�߿�
		case '7': {bMix			= !bMix;		break;	}	//���ջ��ģʽ
		case '8': {bTex			= !bTex;		break;	}	//����
		case 'a': {											//�ӽǱ仯
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
		case 'j': {								//�ӽǱ仯
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
	//������Ƕ�
	gluLookAt(	eye[0], eye[1], eye[2],				// ������0��0��0�����ӵ����� (0,5,50)��Y������
				center[0], center[1], center[2],
				0, 1, 0);				

	if (bWire) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);							//�ƹ�
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos	);
	glLightfv(GL_LIGHT0, GL_AMBIENT , LightWhite		);
	glEnable(GL_LIGHT0);
	
	//����ͼ����
	glPushMatrix();
	glTranslatef(0, 0, -90);
	glScalef(360, 360, 1);
	draw_background();					
	glPopMatrix();

	//glTranslatef(0.0f, 0.0f,-6.0f);			// �ƶ�����
	glRotatef(fRotate, 0, 1.0f, 0);				// ��ת
	glRotatef(-90, 1, 0, 0);					// ��ת����ϵ
	glScalef(0.2, 0.2, 0.2);					//����
   
	glEnable(GL_NORMALIZE);
	Draw_Galaxy();								// ������ϵ
	if (bAnim)	fRotate += 0.1f;
	if (bRotate)body_rotation		   += 0.01f;//��ת������
	if (bRotateSelf)body_rotation_self += 0.01f;//��ת������
	glutSwapBuffers();							
}


int main(int argc,  char *argv[])
{
	
	glutInit(&argc, argv);


	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(1024,512);
	
	int windowHandle = glutCreateWindow("�����ͼ��ѧ��ĩ��Ŀ��ZJUGalaxy");

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


