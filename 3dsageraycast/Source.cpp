#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"
#include "textures/Ttest.ppm"
#include "textures/All_Textures.ppm"
#include "textures/sky.ppm"
#include "textures/won.ppm"
#include "textures/lost.ppm"
#include "textures/title.ppm"
#include <math.h>
#include <cmath>
#define PI 3.1415926535
#define P2 PI / 2
#define P3 3 * PI / 2
#define DR 0.0174533 // one degree in radians


constexpr int mapS = 64;
float degToRad(float a) { return a * (PI / 180.0f); }
float FixAng(float a) { if (a > 359) { a -= 360; } if (a < 0) { a += 360; } return a; }


typedef struct
{
	int w, a, d, s; // button state on and off
}ButtonKeys; ButtonKeys keys;

typedef struct
{
	int type;
	int state;
	int map;
	int x, y, z;
}sprite; sprite sp[4];

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
		px = 300; py = 300; pa = 1.73f;
		pdx = cos(pa) * 5; pdy = sin(pa) * 5;
		sp[0].type = 1; sp[0].state = 1; sp[0].map = 0; sp[0].x = 1.5 * 64; sp[0].y = 5 * 64; sp[0].z = 20;
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
		if (gameState == 1) {  screen(1);  timer += 1; if (timer > 50) { fade = 0; timer = 0; gameState = 2; } }
		if (gameState == 2)
		{
			buttons(fElapsedTime);
			drawSky();
			//drawMap2D();
			//drawPlayer();
			drawRays3D();
			drawSprite();
			if ((int)px>>6 == 1 && (int)py>>6 == 1)
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

		
		if (GetKey(olc::Key::W).bHeld) 
		{
			if (mapW[ipy * mapX + ipx_add_xo] == 0) { px += pdx * 0.7; }
			if (mapW[ipy_add_yo * mapX + ipx] == 0) { py += pdy * 0.7; }
			
		}
		if (GetKey(olc::Key::S).bHeld) 
		{
			if (mapW[ipy * mapX + ipx_sub_xo] == 0) { px -= pdx * 0.7; }
			if (mapW[ipy_sub_yo * mapX + ipx] == 0) { py -= pdy * 0.7; }
		}
		if (GetKey(olc::Key::A).bHeld) 
		{
			pa -= 0.1; if (pa < 0) { pa += 2 * PI; } 
			pdx = cos(pa) * 5; pdy = sin(pa) * 5;
		}
		if (GetKey(olc::Key::D).bHeld) 
		{
			pa += 0.1; if (pa > 2 * PI) { pa -= 2 * PI; }
			pdx = cos(pa) * 5; pdy = sin(pa) * 5;
		}

		if (GetKey(olc::Key::E).bPressed) //open door
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
			int ipx = px / 64.0;
			ipx_add_xo = (px + xo) / 64.0;
			int ipy = py / 64.0; 
			ipy_add_yo = (py + yo) / 64.0;
			int i = 0;
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

	void drawSprite()
	{
		float sx = sp[0].x - px;
		float sy = sp[0].y - py;
		float sz = sp[0].z;

		float CS = cos(degToRad(pa)), SN = sin(degToRad(pa));
		float a = sy * CS + sx * SN;
		float b = sx * CS - sy * SN;
		sx = a; sy = b;
		sx = (sx * 108.0 / sy) + (120 / 2);
		sy = (sz * 108.0 / sy) + (80 / 2);
		FillRect(sx * 8, sy * 8, 8, 8, olc::Pixel(0, 100, 255));
	}

	void drawPlayer()
	{
		FillRect(px, py, 4, 4, olc::GREEN);
		
	}
	
	void drawMap2D()
	{
		int x, y, xo, yo;
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
				int xo = ((int)(pa  * 180 / 3.141589f)* 2) + x;
				if (xo < 0) { xo += 120; }
				xo = xo % 120;
				int pixel = (y * 120 + xo) * 3;
				int red = sky[pixel + 0];
				int green = sky[pixel + 1];
				int blue = sky[pixel + 2];
		
				FillRect(x * 8 , y * 8, 8, 8, olc::Pixel(red, green, blue));
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
		int r, mapx, mapy, map, dof;  float rx, ry, ra, xo, yo, disT;
		ra = pa - DR * 30;
		anglenormalize(ra);
		
		for (r = 0; r < 120; r++)
		{
			//check horizontial lines
			int vmt = 0, hmt = 0, mt = 0;
			dof = 0;                         
			float disH = 100000, hx = px, hy = py; 
			if (ra > PI)  // looking up
			{
				ry = (((int)py / 64) * 64 ) - 0.0001;
				rx = (py - ry) / -tan(ra) + px;
				yo = -64;
				xo = -yo / -tan(ra);
			
			}
			
			if (ra < PI) // looking down
			{
				ry = (((int)py / 64) * 64) + 64;
				rx = (py - ry) / -tan(ra) + px;
				yo = 64;
				xo = -yo / -tan(ra);
			
			}
			
			if (ra == 0 || ra == PI) //looking straight left or right
			{
				rx = px; ry = py; dof = 8;
			}
			int mv = 0, mh = 0;
			while (dof < 8)
			{
				mapx = (int)(rx) / 64;
				mapy = (int)(ry) / 64; 
				map = mapy * mapX + mapx;
				if (map > 0 && map < mapX * mapY && mapW[map] > 0) 
				{ 
					hmt = mapW[map] - 1;
					hx = rx; hy = ry; disH = dist(px, py, hx, hy, ra);
					dof = 8; 
				}
				else { rx += xo; ry += yo; dof += 1; }// hit wall
			
				//DrawCircle(rx, ry, 5, olc::GREEN);
				

			}
			
			//check vertical lines

			dof = 0;
			float disV = 100000, vx = px, vy = py;
			if (ra > P2 && ra < P3)     // looking left
			{
				rx = (((int)px / 64) * 64) - 0.0001;
				ry = (px - rx) * -tan(ra) + py;
				xo = -64;
				yo = -xo * -tan(ra);
			
			}
			
			if (ra < P2 || ra > P3)      //looking right 
			{
				rx = (((int)px / 64) * 64) + 64;
				ry = (px - rx) * -tan(ra) + py;
				xo = 64;
				yo = -xo * -tan(ra);
			
			}		
			
			if (ra == 0 || ra == PI) //looking straight up or down
			{
				rx = px; ry = py; dof = 8;
			}
			while (dof < 8)
			{
				mapx = (int)(rx) / 64;
				mapy = (int)(ry) / 64;
				map = mapy * mapX + mapx;
				if ( map > 0 && map < mapX * mapY && mapW[map] > 0)
				{

					vmt = mapW[map] - 1;
					vx = rx; vy = ry; disV = dist(px, py, vx, vy, ra);
					dof = 8;
				}		
				else { rx += xo; ry += yo; dof += 1; }// hit wall
			
				//DrawCircle(rx, ry, 5, olc::CYAN);
				
			}
			olc::Pixel p;
			float shade = 1;
			if (disV < disH)      //vertical wall hit
			{
				mt = vmt; shade = 0.5; rx = vx; ry = vy; disT = disV; p = olc::PixelF(0, 100, 255);
				
			} 
			if (disH < disV)   //hortizontal wall hit
			{ 
				mt = hmt; rx = hx; ry = hy; disT = disH; p = olc::PixelF(0, 70, 200);
				
			} 

			
			//DrawLine(px , py , rx, ry, olc::CYAN);

			//Draw 3d walls
			float ca = pa - ra;
			if (ca < 0) { ca += 2 * PI; } 
			if (ca > 2 * PI) { ca -= 2 * PI;}
			disT = disT * cos(ca);
			float lineH = (mapS * 640) / disT; 
			float ty_step = 32.0 / lineH;
			float ty_off = 0;
			if (lineH > 640) 
			{
				ty_off = (lineH - 640) / 2.0;
				lineH = 640; 
			} //line height
			float lineO = 320 - lineH / 2;
			

			//draw walls
			int y;
			float ty = ty_off * ty_step;// +mt * 32;
			float tx = (int)(rx / 2.0) % 32; 
			
			
			if (shade == 1) {
				tx = (int)(rx / 2.0) % 32; if (ra > 180) { tx = 31 - tx; }
			}
			else { tx = (int)(ry / 2.0) % 32; if (ra > 90 && ra < 270) { tx = 31 - tx; } }
			
			
			for (y = 0; y < lineH; y++)
			{
				

				
			 int pixel = ((int)ty * 32 + (int)tx) * 3 + mt * 32 * 32 * 3;
			 int red = All_Textures[pixel + 0] * shade;
			 int green = All_Textures[pixel + 1] * shade;
			 int blue = All_Textures[pixel + 2] * shade;
			 
			 

					
				FillRect(r * 8, lineO + y, 8, 1, olc::Pixel(red, green, blue));
				ty += ty_step;
			}

			//draw floors
			for (y = lineO + lineH; y < 640; y++)
			{
				olc::Pixel p;
				float dy = y - (640 / 2.0), deg = ra, raFix = cos(pa - ra);
				
				float fPlayerTexX = px / 2;
				float fPlayerTexY = py / 2;
				float fDistToScreen = 158 * 2;
				float fPlayerHeight = 32;
				float fPrepDistance = fDistToScreen * fPlayerHeight / dy;
				float fFinalDistance = fPrepDistance / raFix;
				tx = fPlayerTexX + cos(deg) * fFinalDistance;
				ty = fPlayerTexY + sin(deg) * fFinalDistance;
			
				int mp = mapF[(int)(ty / 32.0) * mapX + (int)(tx / 32.0)] * 32 * 32;
				int pixel = (((int)ty & 31) * 32 + ((int)tx & 31)) * 3 + mp * 3;
				int red = All_Textures[pixel + 0] * 0.7;
				int green = All_Textures[pixel + 1] * 0.7;
				int blue = All_Textures[pixel + 2] * 0.7;
				FillRect(r * 8, y, 8, 1, olc::Pixel(red, green, blue));
			
				//draw ceiling
				mp = mapC[(int)(ty / 32.0) * mapX + (int)(tx / 32.0)] * 32 * 32;
				pixel = (((int)ty & 31) * 32 + ((int)tx & 31)) * 3 + mp * 3;
				red = All_Textures[pixel + 0];
				green = All_Textures[pixel + 1];
				blue = All_Textures[pixel + 2];
			
				
				if(mp > 0)
					FillRect(r * 8,640 - y, 8, 1, olc::Pixel(red, green, blue));
			}
			ra += DR * 0.5;
			anglenormalize(ra);
		}
	}
	
	float dist(float ax, float ay, float bx, float by, float angle)
	{
		return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
	}

public:
	float px, py, pdx,pdy,pa; //player
	int mapX = 8, mapY = 8;
	int gameState = 0, timer = 0;
	float fade = 0;
};

int main()
{
	Raycaster game;
	if (game.Construct(960, 640  , 1, 1))
	{
		game.Start();
	}

	return 0;
}