
#include <cstdlib>
#include <iostream>
#include <Windows.h>
#include <cmath>
#include <conio.h>
#include <chrono>
#define PI 3.14159265
using namespace std;

class point 
{
public:
	double x, y, z;
	point(double x, double y, double z) 
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
class cube 
{
public:
	point p1;
	point p2;
	cube(point start, point end) 
	{
		p1 = start;
		p2 = end;
	}
	bool point_in(const point p) 
	{
		if (p.x >= p1.x && p.x <= p2.x) {
			if (p.y >= p2.y && p.y <= p1.y) {
				if (p.z >= p1.z && p.z <= p2.z) {
					return true;
				}
			}
		}
		return false;
	}
};

const double step = 0.05;
int main()
{
	auto t1 = chrono::system_clock::now();
	auto t2 = chrono::system_clock::now();
	int nScreenWidth = 80;				// Console Screen Size X (columns)
	int nScreenHeight = 30;				// Console Screen Size Y (rows)
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	point camera = {0,0,0};
	double direction_horisontal = 90.0, direction_vertical = 90.0;			//в градусах
	cube cube = { {-5,5,10},{5,-5,20} };
	double fps;
	point ray;
	double ray_len;
	double ray_direction_horisontal;
	double ray_direction_vertical;
	chrono::duration<float> elapsedt;
	while (1) 
	{
		t2 = chrono::system_clock::now();
		elapsedt = t2 - t1;
		t1 = t2;
		double dt = elapsedt.count();
		
		if (GetAsyncKeyState((int)'A')) {
			camera.x -= 1;
		}
		if (GetAsyncKeyState((int)'D')) {
			camera.x += 1;
		}
		for (int i = 0; i < nScreenHeight; i++) 
		{
			for (int j = 0; j < nScreenWidth; j++)
			{
				ray = camera;
				ray_len = 0;
				ray_direction_horisontal = 45.0 - 1.125 * j + direction_horisontal;
				ray_direction_vertical = -45.0 + 3.0 * i + direction_vertical;
				while (!cube.point_in(ray) && ray_len <= 25) 
				{
					ray_len +=step;
					ray.x += step * sin(ray_direction_vertical * PI / 180.0) * cos(ray_direction_horisontal * PI / 180.0);
					ray.z += step * sin(ray_direction_vertical * PI / 180.0) * sin(ray_direction_horisontal * PI / 180.0);
					ray.y += step * cos(ray_direction_vertical * PI / 180.0);
				}
				if (cube.point_in(ray)) 
				{
					screen[i * nScreenWidth + j] = L'H';
				}
				else
				{
					screen[i * nScreenWidth + j] = L'.';
				}
			}
		}
		fps = 1.0f / dt;
		if(fps<=999)
		swprintf_s(screen, 9, L"Fps:%3.2f", fps);
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}
	
}

