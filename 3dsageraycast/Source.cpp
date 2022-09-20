#define OLC_PGE_APPLICATION

#include <cmath>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include "textures/Ttest.ppm"
#include "textures/All_Textures.ppm"
#include "textures/sky.ppm"
#include "textures/won.ppm"
#include "textures/lost.ppm"
#include "textures/title.ppm"
#include "textures/sprites.ppm"

#define PI 3.1415926535f
#define P2 PI / 2.0f    // Joseph21 - be explicit when using floats
#define P3 3.0f * PI / 2.0f
#define DR 0.0174533f   // one degree in radians

#define MOVE_SPEED 50.0f
#define MOVE_FRACTION 0.1f
#define EPSILON 0.00001f


constexpr int mapS = 64;
float degToRad(float a) { return a * (PI / 180.0f); }

// Joseph21 - your parameter is a float. If you use > 359, you create errors when a is 359.2 for instance
float FixAng(float a) { if (a >= 360.0f) { a -= 360.0f; } if (a < 0.0f) { a += 360.0f; } return a; }

typedef struct
{
	int w, a, d, s; // button state on and off
} ButtonKeys;
ButtonKeys keys;

typedef struct
{
	int type;
	int state;
	int map;
	int x, y, z;
} sprite;
sprite sp[4];

class Raycaster : public olc::PixelGameEngine
{
public:
	Raycaster()
	{
		sAppName = "Raycaster";
	}

	int mapW[mapS] =          //walls
	{
	 1,1,1,1,1,3,1,1,
	 6,0,0,1,0,0,0,1,
	 1,0,0,4,0,2,0,1,
	 1,1,4,1,0,0,0,1,
	 2,0,0,0,0,0,0,1,
	 2,0,0,0,0,1,0,1,
	 2,0,0,0,0,0,0,1,
	 1,1,3,1,3,1,3,1,
	};

	int mapF[mapS] =          //floors
	{
	 0,0,0,0,0,0,0,0,
	 0,0,0,0,1,1,1,0,
	 0,0,0,0,2,0,1,0,
	 0,0,0,0,1,1,1,0,
	 0,0,2,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,
	 0,1,1,1,1,0,0,0,
	 0,0,0,0,0,0,0,0,
	};

	int mapC[mapS] =          //ceiling
	{
	 0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,1,0,
	 0,1,3,1,0,0,0,0,
	 0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,
	};

	void init()
	{
		px = 150; py = 400; pa = 1.73f;
		//		pdx = cos(pa) * 5; pdy = sin(pa) * 5;
		pdx = cos(pa); pdy = -sin(pa);    // Joseph21 - notice that 3dsage uses -sin() call here for pdy
		sp[0].type = 1; sp[0].state = 1; sp[0].map = 0; sp[0].x = 1.5 * 64; sp[0].y = 6 * 64; sp[0].z = 20; //key
		sp[1].type = 2; sp[1].state = 1; sp[1].map = 1; sp[1].x = 1.5 * 64; sp[1].y = 5 * 64; sp[1].z = 0; //light 1
		sp[2].type = 2; sp[2].state = 1; sp[2].map = 1; sp[2].x = 3.5 * 64; sp[2].y = 5 * 64; sp[2].z = 0; //light 2
		sp[3].type = 3; sp[3].state = 1; sp[3].map = 2; sp[3].x = 4.5 * 64; sp[3].y = 6 * 64; sp[3].z = 20; //enemy
	}
	void playerreset()
	{
		px = 150; py = 400;
	}

public:
	bool OnUserCreate() override
	{
		init();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		Clear(olc::DARK_GREY);
		if (gameState == 0) {
			mapW[19] = 4; mapW[26] = 4; fade = 0; timer = 0; gameState = 1;

		}
		if (gameState == 1) { screen(1);  timer += 1; if (timer > 50) { fade = 0; timer = 0; gameState = 2; } }
		if (gameState == 2)
		{
			buttons(fElapsedTime);
			drawSky();
			drawRays3D();
			drawSprite(fElapsedTime);

			//			drawMap2D();     // Joseph21 - temporarily enabled for debugging
						//drawPlayer();

			DrawString(5, 500, "Player X: " + std::to_string(px));    // Joseph21 - for debugging
			DrawString(5, 510, "Player Y: " + std::to_string(py));
			DrawString(5, 520, "Player A: " + std::to_string(pa));

			if ((int)px >> 6 == 1 && (int)py >> 6 == 1)
			{
				fade = 0;
				timer = 0;
				gameState = 3;
			}

		}
		if (gameState == 3) { screen(2);  timer += 1; if (timer > 50) { px = 300; py = 300; fade = 0; timer = 0; gameState = 0; } }

		return true;
	}

	void buttons(float deltatime)
	{

		int xo = 0;
		if (pdx < 0) { xo = -20; }
		else { xo = 20; }
		int yo = 0;
		if (pdy < 0) { yo = -20; }
		else { yo = 20; }

		int ipx = px / 64.0, ipx_add_xo = (px + xo) / 64.0, ipx_sub_xo = (px - xo) / 64.0;
		int ipy = py / 64.0, ipy_add_yo = (py + yo) / 64.0, ipy_sub_yo = (py - yo) / 64.0;

		float fMoveSpeed = deltatime * 100.0f;    // Joseph21 - improved speed control for debugging
		float fRotSpeed = deltatime * 2.0f;

		if (GetKey(olc::Key::W).bHeld)
		{
			if (mapW[ipy * mapX + ipx_add_xo] == 0) { px += pdx * fMoveSpeed; }
			if (mapW[ipy_add_yo * mapX + ipx] == 0) { py += pdy * fMoveSpeed; }

		}
		if (GetKey(olc::Key::S).bHeld)
		{
			if (mapW[ipy * mapX + ipx_sub_xo] == 0) { px -= pdx * fMoveSpeed; }
			if (mapW[ipy_sub_yo * mapX + ipx] == 0) { py -= pdy * fMoveSpeed; }
		}
		if (GetKey(olc::Key::A).bHeld)
		{
			pa += fRotSpeed; if (pa > 2.0f * PI) { pa -= 2.0f * PI; }
			//			pdx = cos(pa) * 5; pdy = sin(pa) * 5;
			pdx = cos(pa); pdy = -sin(pa);    // Joseph21 - notice that 3dsage uses -sin() for pdy
		}
		if (GetKey(olc::Key::D).bHeld)
		{
			pa -= fRotSpeed; if (pa < 0.0f) { pa += 2.0f * PI; }
			//			pdx = cos(pa) * 5; pdy = sin(pa) * 5;
			pdx = cos(pa); pdy = -sin(pa);    // Joseph21 - notice that 3dsage uses -sin() for pdy
		}

		if (GetKey(olc::Key::E).bPressed && sp[0].state == 0) //open door
		{
			int xo = 0;
			if (pdx < 0)
			{
				xo = -25;
			}
			else
			{
				xo = 25;
			}
			int yo = 0;
			if (pdy < 0)
			{
				yo = -25;
			}
			else
			{
				yo = 25;
			}
			//			int ipx = px / 64.0;       // Joseph21 - unused variable
			ipx_add_xo = (px + xo) / 64.0;
			//			int ipy = py / 64.0;       // Joseph21 - unused variable
			ipy_add_yo = (py + yo) / 64.0;
			//			int i = 0;       // Joseph21 - unused variable
			if (mapW[ipy_add_yo * mapX + ipx_add_xo] == 4)
			{
				mapW[ipy_add_yo * mapX + ipx_add_xo] = 0;
			}
		}
	}


	void anglenormalize(float& angle)
	{
		angle = remainder(angle, 2 * PI);
		if (angle < 0)
			angle = 2 * PI + angle;
	}

	// draw functions

	void drawSprite(float deltatime)
	{
		float fMoveSpeed = deltatime * 50.0f;    // Joseph21 - improved speed control for debugging
		float fRotSpeed = deltatime * 2.0f;
		int x, y, s;
		if (px < sp[0].x + 30 && px > sp[0].x - 30 && py < sp[0].y + 30 && py > sp[0].y - 30) 
		{ 
			sp[0].state = 0; 
		}
		if (px < sp[3].x + 30 && px > sp[3].x - 30 && py < sp[3].y + 30 && py > sp[3].y - 30)
		{
			gameState = 0;
			playerreset();
		}

		//enemy move and attack
		int spx = (int)sp[3].x >> 6, spy = (int)sp[3].y >> 6;          //normal grid position
		int spx_add = int(sp[3].x + 15)  / 64.0, spy_add = int(sp[3].y + 15) / 64.0; //normal grid position plus     offset
		int spx_sub = int(sp[3].x - 15) / 64.0, spy_sub = int(sp[3].y - 15) / 64.0; //normal grid position subtract offset
		
		if (sp[3].x > px && mapW[spy * 8 + spx_sub] == 0) 
		{ 
			sp[3].x -= 0.04 * fRotSpeed;
		}
		if (sp[3].x < px && mapW[spy * 8 + spx_add] == 0)
		{
			sp[3].x += 0.04 * fRotSpeed;
		}
		if (sp[3].y > py && mapW[spy_sub * 8 + spx] == 0) 
		{ 
			sp[3].y -= 0.04 * fRotSpeed;
		}
		if (sp[3].y < py && mapW[spy_add * 8 + spx] == 0) 
		{
			sp[3].y += 0.04 * fRotSpeed; 
		}


		for (s = 0; s < 4; s++)
		{
			float sx = sp[s].x - px;
			float sy = sp[s].y - py;
			float sz = sp[s].z;

			float CS = cos(pa), SN = sin(pa);
			float a = sy * CS + sx * SN;
			float b = sx * CS - sy * SN;
			sx = a; sy = b;
			sx = (sx * 108.0 / sy) + (120 / 2);
			sy = (sz * 108.0 / sy) + (80 / 2);


			int scale = 32 * 80 / b;

			if (scale < 0) { scale = 0; } if (scale > 120) { scale = 120; }

			//textures
			float t_x = 0, t_y = 31, t_x_step = 31.0 / (float)scale, t_y_step = 32.0 / (float)scale;

			for (x = sx - scale / 2; x < sx + scale / 2; x++)
			{
				t_y = 31;
				for (y = 0; y < scale; y++)
				{
					if (sp[s].state == 1 && x > 0 && x < 120 && b < depth[x])
					{
						int pixel = ((int)t_y * 32 + (int)t_x) * 3 + (sp[s].map * 32 * 32 * 3);
						int red = sprites[pixel + 0];
						int green = sprites[pixel + 1];
						int blue = sprites[pixel + 2];
						if (red != 255, green != 0, blue != 255) //dont draw if purple
						{
							FillRect(x * 8, sy * 8 - y * 8, 8, 8, olc::Pixel(red, green, blue));
						}
						t_y -= t_y_step; if (t_y < 0) { t_y = 0; }
					}
				}
				t_x += t_x_step; 
			}
		}
	}

	void drawPlayer()
	{
		//		FillRect(px, py, 4, 4, olc::GREEN);

		// Joseph21 - improved drawPlayer() code

				// display little yellow square of 8x8 pixels
		FillRect(px - 4, py - 4, 8, 8, olc::YELLOW);
		// show the looking direction using a little line segment
		DrawLine(px, py, px + pdx * 10.0f, py + pdy * 10.0f, olc::YELLOW);
	}

	void drawMap2D()
	{
		//		int x, y, xo, yo;       // Joseph21 - unused variable
		int x, y;
		olc::Pixel p;
		for (y = 0; y < mapY; y++)
		{
			for (x = 0; x < mapX; x++)
			{
				int worldX = x * mapS;
				int worldY = y * mapS;
				if (mapW[(y * mapX) + x] > 0)
				{
					FillRect(worldX, worldY, mapS, mapS, olc::WHITE);
					DrawRect(worldX, worldY, mapS, mapS, olc::BLACK);
				}
				else {
					FillRect(worldX, worldY, mapS, mapS, olc::BLACK);
					DrawRect(worldX, worldY, mapS, mapS, olc::WHITE);
				}
			}
		}
	}

	void drawSky()
	{
		int x, y;

		for (y = 0; y < 40; y++)
		{
			for (x = 0; x < 120; x++)
			{
				int xo = int(pa / DR) * 2 - x;   // Joseph21 - here we need degree value so divide pa by DR
				if (xo < 0) { xo += 120; }
				xo = xo % 120;
				int pixel = (y * 120 + xo) * 3;
				int red = sky[pixel + 0];
				int green = sky[pixel + 1];
				int blue = sky[pixel + 2];

				FillRect(x * 8, y * 8, 8, 8, olc::Pixel(red, green, blue));
			}
		}
	}

	void screen(int v)
	{
		int x, y;
		int* T = nullptr;
		if (v == 1) { T = title; }
		if (v == 2) { T = won; }
		if (v == 3) { T = lost; }
		for (y = 0; y < 80; y++)
		{
			for (x = 0; x < 120; x++)
			{

				int pixel = (y * 120 + x) * 3;
				int red = T[pixel + 0] * fade;
				int green = T[pixel + 1] * fade;
				int blue = T[pixel + 2] * fade;

				FillRect(x * 8, y * 8, 8, 8, olc::Pixel(red, green, blue));
			}
		}
		if (fade < 1) { fade += 0.05f; }
		if (fade > 1) { fade = 1; }
	}

	void drawRays3D()
	{
		int r, mx, my, mp, dof, side; float vx, vy, rx, ry, ra, xo, yo, disV, disH;

		//		ra = FixAng(pa + 30);                                                              //ray set back 30 degrees
		ra = pa + 30.0f * DR;                                                              //ray set back 30 degrees
		anglenormalize(ra);

		for (r = 0; r < 120; r++)
		{

			// Joseph21 - removed calls to degToRad() since ra is already in radians
			int vmt = 0, hmt = 0;                                                              //vertical and horizontal map texture number
			//---Vertical---
			dof = 0; side = 0; disV = 100000;
			float Tan = tan(ra);
			if (cos(ra) > 0.001) { rx = (((int)px >> 6) << 6) + 64;     ry = (px - rx) * Tan + py; xo = 64; yo = -xo * Tan; }//looking left
			else if (cos(ra) < -0.001) { rx = (((int)px >> 6) << 6) - 0.0001; ry = (px - rx) * Tan + py; xo = -64; yo = -xo * Tan; }//looking right
			else { rx = px; ry = py; dof = 8; }                                                  //looking up or down. no hit

			while (dof < 8)
			{
				mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapX + mx;
				if (mp >= 0 && mp < mapX * mapY && mapW[mp]>0) { vmt = mapW[mp] - 1; dof = 8; disV = cos(ra) * (rx - px) - sin(ra) * (ry - py); }//hit
				else { rx += xo; ry += yo; dof += 1; }                                               //check next horizontal
			}
			vx = rx; vy = ry;

			//---Horizontal---
			dof = 0; disH = 100000;
			Tan = 1.0 / Tan;
			if (sin(ra) > 0.001) { ry = (((int)py >> 6) << 6) - 0.0001; rx = (py - ry) * Tan + px; yo = -64; xo = -yo * Tan; }//looking up
			else if (sin(ra) < -0.001) { ry = (((int)py >> 6) << 6) + 64;     rx = (py - ry) * Tan + px; yo = 64; xo = -yo * Tan; }//looking down
			else { rx = px; ry = py; dof = 8; }                                                   //looking straight left or right

			while (dof < 8)
			{
				mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapX + mx;
				if (mp >= 0 && mp < mapX * mapY && mapW[mp]>0) { hmt = mapW[mp] - 1; dof = 8; disH = cos(ra) * (rx - px) - sin(ra) * (ry - py); }//hit
				else { rx += xo; ry += yo; dof += 1; }                                               //check next horizontal
			}

			float shade = 1;

			if (disV < disH) { hmt = vmt; shade = 0.5; rx = vx; ry = vy; disH = disV; } // vertical hit is shortest

//			int ca = pa - ra;
			float ca = pa - ra;       // Joseph21 - pa and ra are floats: it makes no sense if ca is int

			disH = disH * cos(ca);                            //fix fisheye

//			int lineH = (mapS * 640) / disH;+
			

			float lineH = (mapS * 640) / disH;    // Joseph21 - you're dividing by a float, it makes no sense to put the result in an int variable

			float ty_step = 32.0 / (float)lineH;
			float ty_off = 0;
			if (lineH > 640) { ty_off = (lineH - 640) / 2.0; lineH = 640; }                            //line height and limit
			int lineOff = 320 - (lineH / 2);                                               //line offset

			depth[r] = disH; //save this lines depth
			//---draw walls---
			int y;
			float ty = ty_off * ty_step; //  + hmt * 32;
			float tx;
			if (shade == 1) { tx = (int)(rx / 2.0) % 32; if (ra > 180) { tx = 31 - tx; } }
			else { tx = (int)(ry / 2.0) % 32; if (ra > 90 && ra < 270) { tx = 31 - tx; } }
			for (y = 0; y < lineH; y++)
			{
				int pixel = ((int)ty * 32 + (int)tx) * 3 + (hmt * 32 * 32 * 3);
				int red = All_Textures[pixel + 0] * shade;
				int green = All_Textures[pixel + 1] * shade;
				int blue = All_Textures[pixel + 2] * shade;

				olc::Pixel Testp = olc::Pixel(red, green, blue);

				FillRect(r * 8, lineOff + y, 8, 1, Testp);

				ty += ty_step;

				//				ty += ty_step;     // Joseph21 - doesn't look logical to increase ty twice
			}

			//---draw floors---
			for (y = lineOff + lineH; y < 640; y++)
			{
				//				float dy = y - (640 / 2.0), deg = degToRad(ra), raFix = cos(degToRad(pa - ra));
				float dy = y - (640 / 2.0), deg = ra, raFix = cos(pa - ra);       // Joseph21 - pa and ra are in radians already

				tx = px / 2 + cos(deg) * 158 * 2 * 32 / dy / raFix;
				ty = py / 2 - sin(deg) * 158 * 2 * 32 / dy / raFix;
				int nmp = mapF[(int)(ty / 32.0) * mapX + (int)(tx / 32.0)] * 32 * 32;
				int pixel = (((int)ty & 31) * 32 + ((int)tx & 31)) * 3 + nmp * 3;
				int red = All_Textures[pixel + 0] * 0.7;
				int green = All_Textures[pixel + 1] * 0.7;
				int blue = All_Textures[pixel + 2] * 0.7;

				olc::Pixel Testp = olc::Pixel(red, green, blue);

				FillRect(r * 8, y, 8, 1, Testp);

				//---draw ceiling---
				nmp = mapC[(int)(ty / 32.0) * mapX + (int)(tx / 32.0)] * 32 * 32;
				pixel = (((int)ty & 31) * 32 + ((int)tx & 31)) * 3 + nmp * 3;
				red = All_Textures[pixel + 0];
				green = All_Textures[pixel + 1];
				blue = All_Textures[pixel + 2];

				Testp = olc::Pixel(red, green, blue);
				if (nmp > 0) {
					FillRect(r * 8, 640 - y, 8, 1, Testp);
				}
			}

			//			ra = FixAng(ra - 0.5);
			ra -= 0.5f * DR;       // Joseph21 - ra is in radians, and FixAng works for degrees...
			anglenormalize(ra);
		}
	}

	float dist(float ax, float ay, float bx, float by, float angle)
	{
		return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
	}

public:
	float px, py, pdx, pdy, pa; //player: position, movement, angle
	int mapX = 8, mapY = 8;
	int gameState = 0, timer = 0;
	float fade = 0;
	int depth[120];
};

int main()
{
	Raycaster game;
	if (game.Construct(960, 640, 1, 1))
	{
		game.Start();
	}

	return 0;
}