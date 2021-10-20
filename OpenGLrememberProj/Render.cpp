#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

void normalvec(double* A, double* B, double* C)
{
	double a[] = { 0,0,0 };
	double b[] = { 0,0,0 };
	double c[] = { 0,0,0 };
	a[0] = B[0] - A[0];
	a[1] = B[1] - A[1];
	a[2] = B[2] - A[2];
	b[0] = C[0] - A[0];
	b[1] = C[1] - A[1];
	b[2] = C[2] - A[2];
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
	double lenght = sqrt(pow(c[0], 2) + pow(c[1], 2) + pow(c[2], 2));
	c[0] = c[0] / lenght;
	c[1] = c[1] / lenght;
	c[2] = c[2] / lenght;
	glNormal3d(c[0], c[1], c[2]);
}

void Prism() {
	double x1 = 9;
	double y1 = 10;
	double x2 = 16;
	double y2 = 7;
	double x3;
	double y3;
	x3 = (x1 + x2) / 2;
	y3 = (y1 + y2) / 2;
	double radius;
	radius = sqrt(pow(x1 - x3, 2) + pow(y1 - y3, 2));
	float pi = 3.14;
	float step = 5.0;
	double A1[] = { 0,0,0 };
	double B1[] = { 0,6,0 };
	double C1[] = { 7,6,0 };
	double D1[] = { x1,y1,0 };
	double E1[] = { x2,y2,0 };
	double F1[] = { 16,2,0 };
	double G1[] = { 9,3,0 };
	double H1[] = { 8,-1,0 };
	double A2[] = { 0,0,7 };
	double B2[] = { 0,6,7 };
	double C2[] = { 7,6,7 };
	double D2[] = { x1,y1,7 };
	double E2[] = { x2,y2,7 };
	double F2[] = { 16,2,7 };
	double G2[] = { 9,3,7 };
	double H2[] = { 8,-1,7 };
	normalvec(A1, B1, G1);
	glBegin(GL_QUADS);
	glColor3d(0.580, 0.000, 0.827);
	glVertex3dv(A1);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(B1);
	glVertex3dv(G1);
	glColor3d(0.580, 0.000, 0.827);
	glVertex3dv(H1);
	glEnd();
	glBegin(GL_QUADS);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(F1);
	glVertex3dv(G1);
	glEnd();
	glBegin(GL_QUADS);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(C1);
	glColor3d(0.133, 0.545, 0.133);
	glVertex3dv(D1);
	glVertex3dv(E1);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(F1);
	glEnd();
	normalvec(B2, A2, G2);
	glBegin(GL_QUADS);
	glColor3d(0.580, 0.000, 0.827);
	glVertex3dv(A2);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(B2);
	glVertex3dv(G2);
	glColor3d(0.580, 0.000, 0.827);
	glVertex3dv(H2);
	glEnd();
	glBegin(GL_QUADS);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(B2);
	glVertex3dv(C2);
	glVertex3dv(F2);
	glVertex3dv(G2);
	glEnd();
	glBegin(GL_QUADS);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(C2);
	glColor3d(0.133, 0.545, 0.133);
	glVertex3dv(D2);
	glVertex3dv(E2);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(F2);
	glEnd();
	normalvec(B1, A1, B2);
	glBegin(GL_QUADS);
	glColor3d(0.580, 0.000, 0.827);
	glVertex3dv(A1);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(B1);
	glVertex3dv(B2);
	glColor3d(0.580, 0.000, 0.827);
	glVertex3dv(A2);
	glEnd();
	normalvec(C1, B1, C2);
	glBegin(GL_QUADS);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(C2);
	glVertex3dv(B2);
	glEnd();
	normalvec(D1, C1, D2);
	glBegin(GL_QUADS);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(C1);
	glColor3d(0.133, 0.545, 0.133);
	glVertex3dv(D1);
	glVertex3dv(D2);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(C2);
	glEnd();
	normalvec(F1, E1, F2);
	glBegin(GL_QUADS);
	glColor3d(0.133, 0.545, 0.133);
	glVertex3dv(E1);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(F1);
	glVertex3dv(F2);
	glColor3d(0.133, 0.545, 0.133);
	glVertex3dv(E2);
	glEnd();
	normalvec(G1, F1, G2);
	glBegin(GL_QUADS);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(F1);
	glVertex3dv(G1);
	glVertex3dv(G2);
	glVertex3dv(F2);
	glEnd();
	normalvec(H1, G1, H2);
	glBegin(GL_QUADS);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(G1);
	glColor3d(0.580, 0.000, 0.827);
	glVertex3dv(H1);
	glVertex3dv(H2);
	glColor3d(1.000, 0.894, 0.710);
	glVertex3dv(G2);
	glEnd();
	normalvec(A1, H1, A2);
	glBegin(GL_QUADS);
	glColor3d(0.580, 0.000, 0.827);
	glVertex3dv(H1);
	glVertex3dv(A1);
	glVertex3dv(A2);
	glVertex3dv(H2);
	glEnd();
}

void Semicircle() {
	double t = 7;
	double n = 20;
	double pi = 3.14159;
	double x, y;
	double x1 = 9, y1 = 10, x2 = 16, y2 = 7;
	double x3, y3;
	x3 = (x1 + x2) / 2;
	y3 = (y1 + y2) / 2;
	double radius;
	radius = sqrt(pow(x1 - x3, 2) + pow(y1 - y3, 2));
	double a = 23.2;
	double f = (180 + a)*180/pi;
	double xNext[20];
	double yNext[20];
	for (int i = 0;i < n;i++)
	{
		xNext[i] = cos(f - 6 * i) * radius + x3;
		yNext[i] = sin(f - 6 * i) * radius + y3;
	}
	double D1[] = { x1,y1,0 };
	double E1[] = { x2,y2,0 };
	double O[] = { x3,y3,0 };
	double O1[] = { xNext[0],yNext[0],0 };
	double O2[] = { xNext[1],yNext[1],0 };
	double O3[] = { xNext[2],yNext[2],0 };
	double O4[] = { xNext[3],yNext[3],0 };
	double O5[] = { xNext[4],yNext[4],0 };
	double O6[] = { xNext[5],yNext[5],0 };
	double O7[] = { xNext[6],yNext[6],0 };
	double O8[] = { xNext[7],yNext[7],0 };
	double O9[] = { xNext[8],yNext[8],0 };
	double O10[] = { xNext[9],yNext[9],0 };
	double O11[] = { xNext[10],yNext[10],0 };
	double D2[] = { x1,y1,t };
	double E2[] = { x2,y2,t };
	double Z[] = { x3,y3,t };
	double Z1[] = { xNext[0],yNext[0],t };
	double Z2[] = { xNext[1],yNext[1],t };
	double Z3[] = { xNext[2],yNext[2],t };
	double Z4[] = { xNext[3],yNext[3],t };
	double Z5[] = { xNext[4],yNext[4],t };
	double Z6[] = { xNext[5],yNext[5],t };
	double Z7[] = { xNext[6],yNext[6],t };
	double Z8[] = { xNext[7],yNext[7],t };
	double Z9[] = { xNext[8],yNext[8],t };
	double Z10[] = { xNext[9],yNext[9],t };
	double Z11[] = { xNext[10],yNext[10],t };
	//нижняя//
	glColor3d(0.133, 0.545, 0.133);
	normalvec(E1, O, O1);
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(E1);
	glVertex3dv(O1);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O1);
	glVertex3dv(O2);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O2);
	glVertex3dv(O3);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O3);
	glVertex3dv(O4);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O4);
	glVertex3dv(O5);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O5);
	glVertex3dv(O6);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O6);
	glVertex3dv(O7);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O7);
	glVertex3dv(O8);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O8);
	glVertex3dv(O9);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O9);
	glVertex3dv(O10);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O10);
	glVertex3dv(O11);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(O);
	glVertex3dv(O11);
	glVertex3dv(D1);
	glEnd();
	//верхушка//
	normalvec(Z, E2, Z1);
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(E2);
	glVertex3dv(Z1);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z1);
	glVertex3dv(Z2);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z2);
	glVertex3dv(Z3);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z3);
	glVertex3dv(Z4);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z4);
	glVertex3dv(Z5);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z5);
	glVertex3dv(Z6);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z6);
	glVertex3dv(Z7);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z7);
	glVertex3dv(Z8);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z8);
	glVertex3dv(Z9);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z9);
	glVertex3dv(Z10);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z10);
	glVertex3dv(Z11);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(Z);
	glVertex3dv(Z11);
	glVertex3dv(D2);
	glEnd();
	///боковые///
	normalvec(O1, Z1, E2);
	glBegin(GL_QUADS);
	glVertex3dv(E1);
	glVertex3dv(O1);
	glVertex3dv(Z1);
	glVertex3dv(E2);
	glEnd();
	normalvec(O2, Z2, Z1);
	glBegin(GL_QUADS);
	glVertex3dv(O1);
	glVertex3dv(O2);
	glVertex3dv(Z2);
	glVertex3dv(Z1);
	glEnd();
	normalvec(O3, Z3, Z2);
	glBegin(GL_QUADS);
	glVertex3dv(O2);
	glVertex3dv(O3);
	glVertex3dv(Z3);
	glVertex3dv(Z2);
	glEnd();
	normalvec(O4, Z4, Z3);
	glBegin(GL_QUADS);
	glVertex3dv(O3);
	glVertex3dv(O4);
	glVertex3dv(Z4);
	glVertex3dv(Z3);
	glEnd();
	normalvec(O5, Z5, Z4);
	glBegin(GL_QUADS);
	glVertex3dv(O4);
	glVertex3dv(O5);
	glVertex3dv(Z5);
	glVertex3dv(Z4);
	glEnd();
	normalvec(O6, Z6, Z5);
	glBegin(GL_QUADS);
	glVertex3dv(O5);
	glVertex3dv(O6);
	glVertex3dv(Z6);
	glVertex3dv(Z5);
	glEnd();
	normalvec(O7, Z7, Z6);
	glBegin(GL_QUADS);
	glVertex3dv(O6);
	glVertex3dv(O7);
	glVertex3dv(Z7);
	glVertex3dv(Z6);
	glEnd();
	normalvec(O8, Z8, Z7);
	glBegin(GL_QUADS);
	glVertex3dv(O7);
	glVertex3dv(O8);
	glVertex3dv(Z8);
	glVertex3dv(Z7);
	glEnd();
	normalvec(O9, Z9,Z8);
	glBegin(GL_QUADS);
	glVertex3dv(O8);
	glVertex3dv(O9);
	glVertex3dv(Z9);
	glVertex3dv(Z8);
	glEnd();
	normalvec(O10, Z10, Z9);
	glBegin(GL_QUADS);
	glVertex3dv(O9);
	glVertex3dv(O10);
	glVertex3dv(Z10);
	glVertex3dv(Z9);
	glEnd();
	normalvec(O11, Z11, Z10);
	glBegin(GL_QUADS);
	glVertex3dv(O10);
	glVertex3dv(O11);
	glVertex3dv(Z11);
	glVertex3dv(Z10);
	glEnd();
	normalvec(Z11, O11, D1);
	glBegin(GL_QUADS);
	glVertex3dv(O11);
	glVertex3dv(D1);
	glVertex3dv(D2);
	glVertex3dv(Z11);
	glEnd();





}
/*
void Semicircle2()
{
	double pi = 3.14159;
	double n = 10;
	double xA = 0, yA = 0, xB = 8, yB = -1, xC = 3, yC = -4;
	double radius;
	double xO, yO;
	//pow(xA-xO,2)+pow(yA-yO,2)=pow(radius,2)
	//pow(xB-xO,2)+pow(yB-yO,2)=pow(radius,2)
	//pow(xC - xO, 2) + pow(yC - yO, 2) = pow(radius, 2)
	//pow(xO,2)+pow(yO,2)=pow(8-xO,2)+pow(-1-yO,2)//// yO=(16*xO-65)/2
	//pow(xO,2)+pow(yO,2)=pow(3-xO,2)+pow(-4-yO,2)
	xO = 235 / 58;
	yO = (16 * xO - 65) / 2;
	radius = sqrt(pow(xO, 2) + pow(yO, 2));
	double f = 90;
	double xNext[20];
	double yNext[20];
	for (int i = 0;i < n;i++)
	{
		xNext[i] = cos(220 - 6 * i) * radius + xO;//////////допилить
		yNext[i] = sin(220 - 6 * i) * radius + yO;
	}
	double A1[] = { xA,yA,0 };
	double H1[] = { xB,yB,0 };
	double M0[] = { xO,yO,0 };
	double M1[] = { xNext[0],yNext[0],0 };
	double M2[] = { xNext[1],yNext[1],0 };
	double M3[] = { xNext[2],yNext[2],0 };
	double M4[] = { xNext[3],yNext[3],0 };
	double M5[] = { xNext[4],yNext[4],0 };
	double M6[] = { xNext[5],yNext[5],0 };
	double M7[] = { xNext[6],yNext[6],0 };
	double M8[] = { xNext[7],yNext[7],0 };
	glColor3d(1.000, 0.894, 0.710);
	glBegin(GL_TRIANGLES);
	glVertex3dv(M0);
	glVertex3dv(H1);
	glVertex3dv(M1);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(M0);
	glVertex3dv(M1);
	glVertex3dv(M2);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(M0);
	glVertex3dv(M2);
	glVertex3dv(M3);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(M0);
	glVertex3dv(M3);
	glVertex3dv(M4);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(M0);
	glVertex3dv(M4);
	glVertex3dv(M5);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(M0);
	glVertex3dv(M5);
	glVertex3dv(M6);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3dv(M0);
	glVertex3dv(M6);
	glVertex3dv(M7);
	glEnd();
}
*/



void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)

	//===================================
	//Прогать тут  


	Prism();
	Semicircle();
	glShadeModel(GL_SMOOTH);


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}