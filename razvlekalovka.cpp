
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <strstream>
#include <Windows.h>
#include <cmath>
#include <conio.h>
#include <chrono>
#include <vector>
#include <string>
#include <strsafe.h>
#define PI 3.14159265
using namespace std;

int const nScreenWidth = 120;				// Console Screen Size X (columns)
int const nScreenHeight = 120;				// Console Screen Size Y (rows)


void DrawPixel(int x, int y, unsigned short color, wchar_t* screen, unsigned short* color_screen);
void DrawLine(int x0, int y0, int x1, int y1, int color, wchar_t* screen, unsigned short* color_screen);
void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color, wchar_t* screen, unsigned short* color_screen);

class point 
{
public:
	double x, y, z;
	point(double const x, double const y, double const z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	point()
	{
		x = 0;
		y = 0;
		z = 0;
	}
};
class triangle
{
public:
	point p1, p2, p3;

	triangle(point const p1, point const p2, point const p3)
	{
		this->p1 = p1;
		this->p2 = p2;
		this->p3 = p3;
	}
	triangle()
	{
		p1 = { 0,0,0 };
		p2 = { 1,1,1 };
		p3 = { 1,0,1 };
	}
};

class model
{
public:
	vector<triangle> triangles;
	model(const vector<triangle> tri)
	{
		triangles = tri;
	}
	model(){}
	bool Load_From_File(string fname) 
	{
		ifstream file(fname);
		if (!file.is_open())
		{
			return false;
		}

		vector<point> v;

		while (!file.eof()) 
		{
			char line[128];
			file.getline(line, 128);
			strstream s;
			s << line;
			char determin;

			if (line[0] == 'v') 
			{
				point p;
				s >> determin >> p.x >> p.y >> p.z;
				v.push_back(p);
			}

			if (line[0] == 'f') 
			{
				int f[3];
				s >> determin >> f[0] >> f[1] >> f[2];
				triangles.push_back({ v[f[0] - 1],v[f[1] - 1],v[f[2] - 1] });
			}
		}

		return true;
	}
};

void MatrMult(point& i, point& o, double matr[][4])
{
	o.x = i.x * matr[0][0] + i.y * matr[1][0] + i.z * matr[2][0] + matr[3][0];
	o.y = i.x * matr[0][1] + i.y * matr[1][1] + i.z * matr[2][1] + matr[3][1];
	o.z = i.x * matr[0][2] + i.y * matr[1][2] + i.z * matr[2][2] + matr[3][2];
	double w = i.x * matr[0][3] + i.y * matr[1][3] + i.z * matr[2][3] + matr[3][3];

	if (w != 0.0) 
	{
		o.x /= w;
		o.y /= w;
		o.z /= w;
	}
}

void DrawPixel(int x, int y, unsigned short color, wchar_t* screen, unsigned short* color_screen)
{
	if (x >= 0 && x < nScreenWidth && y >= 0 && y < nScreenHeight) 
	{
		screen[y * nScreenWidth + x] = L'█';
		color_screen[y * nScreenWidth + x] = color;
	}
}

void DrawLine(int x0, int y0, int x1, int y1, int color, wchar_t* screen, unsigned short* color_screen)
{
	double a = abs(((double)y1 - (double)y0) / ((double)x1 - (double)x0));
	double d;
	int incr_x = x0 < x1 ? 1 : -1;
	int incr_y = y0 < y1 ? 1 : -1;
	if (a <= 1) {
		
		d = y0;
		for (int i = x0; i != x1; i += incr_x) 
		{
			DrawPixel(i, round(d), color, screen, color_screen);
			d += a*incr_y;
		}
	}
	else
	{
		
		d = x0;
		a = 1.0 / a;
		for (int i = y0; i != y1; i += incr_y)
		{
			DrawPixel(round(d), i, color, screen, color_screen);
			d += a*incr_x;
		}
	}
}

void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color, wchar_t* screen, unsigned short* color_screen)
{
	DrawLine(x0, y0, x1, y1, color, screen, color_screen);
	DrawLine(x1, y1, x2, y2, color, screen, color_screen);
	DrawLine(x2, y2, x0, y0, color, screen, color_screen);
}


int main()
{
	auto t1 = chrono::system_clock::now();
	auto t2 = chrono::system_clock::now();


	//настройка консоли
	wchar_t* screen = new wchar_t[(int)nScreenWidth * (int)nScreenHeight];
	unsigned short* color = new unsigned short[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
	{
		color[i] = 0xF0;
		screen[i] = L' ';
	}
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	SetConsoleScreenBufferSize(hConsole, { (short)nScreenWidth,(short)nScreenHeight });
	CONSOLE_FONT_INFOEX font = {};
	GetCurrentConsoleFontEx(hConsole, false, &font);
	font.dwFontSize = { 5,5 };
	font.cbSize = sizeof(font);
	SetCurrentConsoleFontEx(hConsole, false, &font);
	DWORD dwBytesWritten = 0;
	wchar_t fps_string[10];
	
	//-------------------------------------------

	//тут характристики для проекции точек
	//такие как fov и дальность отрисовки
	double zNear = 0.1;
	double zFar = 1000.0;
	double fov = 90.0;
	double AspectRatio = (double)nScreenHeight / (double)nScreenWidth;
	double fovRad = 1.0 / tan(fov * 0.5 / 180.0 * PI);
	double Projection[4][4] =
	{AspectRatio*fovRad,0.0,0.0,0.0,
	 0.0, fovRad,0.0,0.0,
	0.0,0.0,zFar/(zFar-zNear),1.0,
	0.0,0.0,(-zFar*zNear)/ (zFar - zNear) ,0.0};
	//-------------------------------------------


	double RotZ[4][4], RotX[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			RotZ[i][j] = 0;
			RotX[i][j] = 0;
		}
	}

	point camera = {0,0,0};
	double direction_horisontal = 90.0, direction_vertical = 90.0;			//в градусах
	double fps;
	double dteta = 0;
	chrono::duration<float> elapsedt;
	int x = 70;
	int y = 10;
	model cube; /*= { {
		{{0,1,0},{1,0,0},{0,0,0}},
		{{0,1,0},{1,1,0},{1,0,0}},
		{{0,1,1},{0,1,0},{0,0,0}},
		{{0,1,1},{0,0,0},{0,0,1}},
		{{0,0,1},{0,0,0},{1,0,0}},
		{{0,0,1},{1,0,0},{1,0,1}},
		{{1,1,0},{1,0,1},{1,0,0}},
		{{1,1,0},{1,1,1},{1,0,1}},
		{{1,1,1},{0,1,1},{0,0,1}},
		{{1,1,1},{0,0,1},{1,0,1}},
		{{0,1,1},{1,1,0},{0,1,0}},
		{{0,1,1},{1,1,1},{1,1,0}}
		} };*/
	cube.Load_From_File("test.obj");
	while (1) 
	{
		t2 = chrono::system_clock::now();
		elapsedt = t2 - t1;
		t1 = t2;
		double dt = elapsedt.count();
		dteta += dt;
		
		RotZ[0][0] = cos(dteta);
		RotZ[0][1] = sin(dteta);
		RotZ[1][0] = -sin(dteta);
		RotZ[1][1] = cos(dteta);
		RotZ[2][2] = 1;
		RotZ[3][3] = 1;

		RotX[0][0] = 1;
		RotX[1][1] = cos(dteta * 0.5);
		RotX[1][2] = sin(dteta * 0.5);
		RotX[2][1] = -sin(dteta * 0.5);
		RotX[2][2] = cos(dteta * 0.5);
		RotX[3][3] = 1;


		for (auto tri : cube.triangles)
		{
			
			triangle triProj, triTransl, triRotZ, triRotZX;

			MatrMult(tri.p1, triRotZ.p1, RotZ);
			MatrMult(tri.p2, triRotZ.p2, RotZ);
			MatrMult(tri.p3, triRotZ.p3, RotZ);

			MatrMult(triRotZ.p1, triRotZX.p1, RotX);
			MatrMult(triRotZ.p2, triRotZX.p2, RotX);
			MatrMult(triRotZ.p3, triRotZX.p3, RotX);

			triTransl = triRotZX;
			triTransl.p1.z = triRotZX.p1.z + 5.0;
			triTransl.p2.z = triRotZX.p2.z + 5.0;
			triTransl.p3.z = triRotZX.p3.z + 5.0;

			point normal, A, B;
			A.x = triTransl.p2.x - triTransl.p1.x;
			A.y = triTransl.p2.y - triTransl.p1.y;
			A.z = triTransl.p2.z - triTransl.p1.z;

			B.x = triTransl.p3.x - triTransl.p1.x;
			B.y = triTransl.p3.y - triTransl.p1.y;
			B.z = triTransl.p3.z - triTransl.p1.z;

			normal.x = A.y * B.z - A.z * B.y;
			normal.y = A.z * B.x - A.x * B.z;
			normal.z = A.x * B.y - A.y * B.x;

			double l = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);

			normal.x /= l; normal.y /= l;normal.z /= l;

			if (normal.x * (triTransl.p1.x - camera.x) +
				normal.y * (triTransl.p1.y - camera.y) +
				normal.z * (triTransl.p1.z - camera.z) < 0.0 )
			{
				MatrMult(triTransl.p1, triProj.p1, Projection);
				MatrMult(triTransl.p2, triProj.p2, Projection);
				MatrMult(triTransl.p3, triProj.p3, Projection);

				triProj.p1.x += 1.0; triProj.p1.y += 1.0;
				triProj.p2.x += 1.0; triProj.p2.y += 1.0;
				triProj.p3.x += 1.0; triProj.p3.y += 1.0;

				triProj.p1.x *= 0.5 * (double)nScreenWidth; triProj.p1.y *= 0.5 * (double)nScreenHeight;
				triProj.p2.x *= 0.5 * (double)nScreenWidth; triProj.p2.y *= 0.5 * (double)nScreenHeight;
				triProj.p3.x *= 0.5 * (double)nScreenWidth; triProj.p3.y *= 0.5 * (double)nScreenHeight;

				DrawTriangle(triProj.p1.x, triProj.p1.y, triProj.p2.x, triProj.p2.y, triProj.p3.x, triProj.p3.y, 0xFF, screen, color);
			}

		}


		fps = (1.0 / dt);
		swprintf_s(fps_string, 9, L"Fps:%4.0f", fps);
		SetConsoleTitle(fps_string);
		WriteConsoleOutputAttribute(hConsole, color, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth*nScreenHeight, { 0,0 }, &dwBytesWritten);
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
		{
			color[i] = 0x0F;
			screen[i] = L' ';
		}
	}
	
}

