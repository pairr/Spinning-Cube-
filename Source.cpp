#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES // for C++
#include <math.h>
using namespace std;
class Donut : public olc::PixelGameEngine
{
private:
	bool play = true; // used to check if paused
	float delR = 0.05; // density of side of cube
	float alpha1 = 0, alpha2 = 0; // angles of rotation around OX and OY
	float R = 8; // side length of cube
	float K1 = 300, K2 = 30; // distance from viewer to the screen, distance from torus to screen
	float lx = -1.0, ly = 0, lz = -1; // direction of light
	float sqrt2 = sqrt(2.0);
	int** output; //keeps output pixel saturation (of a red color)
	float** zbuffer; // keeps 1/z of a point projected onto pixel
	int mx, my; // mouse x and y
	float sensitivity = 0.01; // sensitivity of a mouse

	bool grow = true;
	float grow_rate = 0.1;
public:
	Donut()
	{
		// Name your application
		sAppName = "Donut";
	}
public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		output = new int* [ScreenWidth()];
		for (int i = 0; i < ScreenWidth(); i++)output[i] = new int[ScreenHeight()];

		zbuffer = new float* [ScreenWidth()];
		for (int i = 0; i < ScreenWidth(); i++)zbuffer[i] = new float[ScreenHeight()];

		for (int i = 0; i < ScreenWidth(); i++)
		{
			for (int j = 0; j < ScreenHeight(); j++)
			{
				output[i][j] = 0;
				zbuffer[i][j] = 0;
			}
		}

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetMouse(0).bPressed)play = !play; // if left clicked 
		if (!play)		 						//pause animation
		{
			if (GetMouse(1).bPressed) { mx = GetMouseX(); my = GetMouseY(); }

			int new_mx, new_my;

			if (GetMouse(1).bHeld)
			{
				new_mx = GetMouseX();
				new_my = GetMouseY();

				alpha1 -= sensitivity * (new_mx - mx); // rotate around OX
				alpha2 -= sensitivity * (new_my - my); // rotate around OY

				mx = new_mx; my = new_my;
			}
			else return true;
		}
		else // if animation is not paused
		{
			alpha1 += 1 * fElapsedTime; // auto rotate around OX
			alpha2 += 1 * fElapsedTime; // auto rotate around OY

			if (grow)
			{
				K2 += grow_rate;
				if (K2 > 70)grow = false;
			}
			else
			{
				K2 -= grow_rate;
				if (K2 < 30)grow = true;
			}
		}
		Clear(olc::BLACK);

		float x, y, z;
		float tx, ty, tz; // inbetween values
		float nx, ny, nz;
		float tnx, tny, tnz; // inbetween values
		float cosalpha1, sinalpha1;
		float cosalpha2, sinalpha2;
		float ooz;
		int xp, yp; // projection on screen coordinates
		float res; // stores const * cos between light direction and normal vector

		for (float a = -R / 2; a < R / 2; a += delR)
		{
			for (float b = -R / 2; b < R / 2; b += delR)
			{
				for (int face = 0; face < 6; face++)
				{
					cosalpha1 = cos(alpha1);
					sinalpha1 = sin(alpha1);
					cosalpha2 = cos(alpha2);
					sinalpha2 = sin(alpha2);
					//calculate x, y, z of point
					//and normal vector
					switch (face)
					{
					case 0:
						x = a; y = b; z = R / 2;
						nx = 0; ny = 0; nz = 1;
						break;
					case 1:
						x = R / 2; y = a; z = b;
						nx = 1; ny = 0; nz = 0;
						break;
					case 2:
						x = a; y = b; z = -R / 2;
						nx = 0; ny = 0; nz = -1;
						break;
					case 3:
						x = -R / 2; y = a; z = b;
						nx = -1; ny = 0; nz = 0;
						break;
					case 4:
						x = b; y = -R / 2; z = a;
						nx = 0; ny = -1; nz = 0;
						break;
					case 5:
						x = b; y = R / 2; z = a;
						nx = 0; ny = 1; nz = 0;
						break;
					}
					//point
					//rotate around OY
					tx = x * cosalpha1 - z * sinalpha1;
					ty = y;
					tz = x * sinalpha1 + z * cosalpha1;
					//rotate around OX
					x = tx;
					y = ty * cosalpha2 - tz * sinalpha2;
					z = ty * sinalpha2 + tz * cosalpha2 + K2;

					//normal vector
					//rotate around OY
					tnx = nx * cosalpha1 - nz * sinalpha1;
					tny = ny;
					tnz = nx * sinalpha1 + nz * cosalpha1;
					//rotate around OX
					nx = tnx;
					ny = tny * cosalpha2 - tnz * sinalpha2;
					nz = tny * sinalpha2 + tnz * cosalpha2;

					ooz = 1 / z;
					//calculate projection on screen coordinates
					xp = (int)(ScreenWidth() / 2 + K1 * ooz * x);
					yp = (int)(ScreenHeight() / 2 - K1 * ooz * y);

					if (xp < 0 || xp >= ScreenWidth())continue;
					if (yp < 0 || yp >= ScreenHeight())continue;

					if (zbuffer[xp][yp] < ooz) // if this point is closer to the screen
					{
						zbuffer[xp][yp] = ooz;
						res = nx * lx + ny * ly + nz * lz; // stores cos * sqrt(2) between light direction and normal vector
						res = (res / sqrt2 + 1.0) / 2.0; // make 0 <= res <= 1
						res = pow(100, res) / 100; // do that to make light more drammatic and noticable 
						output[xp][yp] = res * 255;
					}
				}
			}
		}
		//Draw pixels
		for (int i = 0; i < ScreenWidth(); i++)
		{
			for (int j = 0; j < ScreenHeight(); j++)
			{
				Draw(i, j, olc::Pixel(output[i][j], 0, 0));
				zbuffer[i][j] = 0;
				output[i][j] = 0;
			}
		}
		return true;
	}
};

int main()
{
	Donut demo;
	if (demo.Construct(256 * 1, 256 * 1, 3, 3))
		demo.Start();
	return 0;
}