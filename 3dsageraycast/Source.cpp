#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"
#include "Textures.h"
#include <math.h>
#include <cmath>
#define PI 3.1415926535
#define P2 PI / 2
#define P3 3 * PI / 2
#define DR PI / 180

const int mapx = 8;
const int mapy = 8;
int tilesize = 64;
int mapsize = mapx * mapy;

float degToRad(float a) { return a * (PI / 180.0f); }
float FixAng(float a) { if (a > 359) { a -= 360; } if (a < 0) { a += 360; } return a; }

struct Player {
	float x;
	float y;
	float width;
	float height;
	int turnDirection;
	int walkDirection;
	float rotationAngle;
	float angle;
	float walkSpeed;
	float turnSpeed;
	float strafeDirection;
	float fFOV;

}player;

struct Ray {
	float x, y, angle;
};



class Raycaster : public olc::PixelGameEngine
{
public:
	Raycaster()
	{
		sAppName = "Raycaster";
	}

	int mapW[mapx * mapy] =
	{
		1,1,1,1,1,2,1,1,
		1,0,0,1,0,0,0,1,
		1,0,0,3,0,0,0,1,
		1,1,3,1,0,0,0,1,
		1,0,0,0,0,0,0,1,
		1,0,0,0,1,0,0,1,
		1,0,0,0,1,0,0,1,
		1,1,2,1,1,4,1,1,
	};



public:
	bool OnUserCreate() override
	{

		player.x = float(mapx * mapsize / 5);
		player.y = float(mapy * mapsize / 2);
		player.width = 4;
		player.height = 4;
		player.turnDirection = 0;
		player.walkDirection = 0;
		player.strafeDirection = 0;
		player.rotationAngle = 2 * PI / 3;
		player.angle = 90 * (3.14159 / 180);
		player.walkSpeed = 100;
		player.turnSpeed = 45 * (PI / 180);
		player.fFOV = 3.14159f / 4.0f;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::DARK_GREY);



		if (GetKey(olc::Key::W).bHeld)
			player.walkDirection = +1; //up
		if (GetKey(olc::Key::S).bHeld)
			player.walkDirection = -1; //down
		if (GetKey(olc::Key::A).bHeld)
			player.turnDirection = -1; //left
		if (GetKey(olc::Key::D).bHeld)
			player.turnDirection = +1; //right
		
		if (GetKey(olc::Key::E).bPressed) //open doors
		{

			float pdx = cos(player.angle);
			float pdy = sin(player.angle);

			int xo = 0; 
			if (pdx < 0) 
			{ 
				xo = -40; 
			} 
			else
			{
				xo = 40;
			}
			int yo = 0; 
			if (pdy < 0) 
			{ 
				yo = -40; 
			}
			else 
			{ 
				yo = 40; 
			}

			int ipx = player.x / 64.0f, ipx_add_xo = (player.x + xo) / 64.0f;
			int ipy = player.y / 64.0f, ipy_add_yo = (player.y + yo) / 64.0f;

			
			if (mapW[ipy_add_yo * mapx + ipx_add_xo] == 3)
			{
				mapW[ipy_add_yo * mapx + ipx_add_xo] = 0;
			}
			
			
		}

		if (GetKey(olc::Key::W).bReleased)
			player.walkDirection = 0; //up
		if (GetKey(olc::Key::S).bReleased)
			player.walkDirection = 0; //down
		if (GetKey(olc::Key::A).bReleased)
			player.turnDirection = 0; //left
		if (GetKey(olc::Key::D).bReleased)
			player.turnDirection = 0; //right
		

		anglenormalize(player.angle);
		movePlayer(fElapsedTime);
		drawMap();
		drawRays3D();
		FillRect(player.x, player.y, player.width, player.height, olc::GREEN);
		DrawLine(player.x, player.y, player.x + cos(player.angle) * 5, player.y + sin(player.angle) * 5, olc::GREEN);

		return true;
	}

	void anglenormalize(float& angle)
	{
		angle = remainder(angle, 2 * PI);
		if (angle < 0)
			angle = 2 * PI + angle;
	}

	void movePlayer(float deltaTime)
	{
		player.angle += player.turnDirection * player.turnSpeed * deltaTime;
		float moveStep = player.walkDirection * player.walkSpeed * deltaTime;
		float strafeStep = player.strafeDirection * player.walkSpeed * deltaTime;

	

		float newPlayerX = player.x + cos(player.angle) * moveStep;
		float newPlayerY = player.y + sin(player.angle) * moveStep;
		if (!hasWallAt(newPlayerX, newPlayerY))
		{
			player.x = newPlayerX;
			player.y = newPlayerY;
		}
	}

	void drawMap()
	{

		for (int y = 0; y < mapy; y++)
		{
			for (int x = 0; x < mapx; x++)
			{
				int worldX = x * tilesize;
				int worldY = y * tilesize;
				if (mapW[(y * mapx) + x] > 0)
				{
					FillRect(worldX, worldY, tilesize, tilesize, olc::WHITE);
					DrawRect(worldX, worldY, tilesize, tilesize, olc::BLACK);
				}
				else {
					FillRect(worldX, worldY, tilesize, tilesize, olc::BLACK);
					DrawRect(worldX, worldY, tilesize, tilesize, olc::WHITE);
				}


			}
		}
	}

	float dist(float ax, float ay, float bx, float by, float ang)
	{
		return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
	}
	
	

	void drawRays3D()
	{
		int r, mx, my, mp, dof;  float rx, ry, ra, xo, yo, disT, shade, ty, tx;

		ra = player.angle - DR * 30;
		
		anglenormalize(ra);

		FillRect(530, 0, 60 * 8, 160, olc::CYAN);
		//FillRect(530, 160, 60 * 8, 160, olc::BLUE);
		for(int r = 0; r < 60; r++)
		{	
			int vmt = 0, hmt = 0, mt = 0; //vertical and horizontal map texture number
			//horizontal line check

			dof = 0;
			float disH = 1000000, hx = player.x, hy = player.y;

			float a1tan =  -1 / tan(ra);
			
			//looking up
			if (ra > PI) {
				ry = (((int)player.y >> 6) << 6) - 0.0001; 
				
				rx = (player.y - ry) * a1tan + player.x;
				yo = -64;
				xo = -yo * a1tan;

			}

			//looking down
			if (ra < PI) {
				ry = (((int)player.y >> 6) << 6) + 64;
				tx = (player.y - ry) * a1tan;
				rx = (player.y - ry) * a1tan + player.x;
				yo = +64;
				xo = -yo * a1tan;

			}
			if (ra == 0 || ra == PI) { rx = player.x; ry = player.y; dof = 8; }
			while (dof < 8)
			{
				mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapx + mx;
				if (mp >= 0 && mp < mapx * mapy && mapW[mp] > 0) 
				{
					hx = rx; hy = ry; disH = dist(player.x, player.y, hx, hy, ra); dof = 8; hmt = mapW[mp] - 1;
				}
				else { rx += xo; ry += yo; dof += 1;  }
			}

			

			//vertical line check
			dof = 0;
			float ntan = -tan(ra);
            float disV = 1000000, vx = player.x, vy = player.y;
			//looking left
			if (ra > P2 && ra < P3) {
				rx = (((int)player.x >> 6) << 6) - 0.0001;
				
				ry = (player.x - rx) * ntan + player.y;
				xo = -64;
				yo = -xo * ntan;

			}
			//looking right
			if (ra < P2 || ra > P3) {
				rx = (((int)player.x >> 6) << 6) + 64;
				ry = (player.x - rx) * ntan + player.y;
				xo = +64;
				yo = -xo * ntan;

			}
			if (ra == 0.5f * PI || ra == 1.5f * PI) { rx = player.x; ry = player.y; dof = 8; }
			
			while (dof < 8)
			{
				mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapx + mx;
				if (mp >= 0 && mp < mapx * mapy && mapW[mp] > 0) 
				{
					vx = rx; vy = ry; disV = dist(player.x, player.y, vx, vy, ra); dof = 8; vmt = mapW[mp] - 1;
				}
				else { rx += xo; ry += yo; dof += 1;  }
			}

			shade = 1;
			olc::Pixel p1 = olc::PixelF(0, 0.8, 0);
			if (disV < disH) { mt = vmt; shade = 0.5f; rx = vx; ry = vy; disT = disV; }
			if (disH < disV) { mt = hmt; rx = hx; ry = hy; disT = disH; }
			
			DrawLine(player.x, player.y, rx, ry, olc::GREEN);
			
			draw3DWalls(disT,ra,r, shade,rx,ry, mt);
			ra += DR;
			anglenormalize(ra);
		}
	}

	void draw3DWalls(float disT, float ra, int r, float shade, float rx, float ry, int mt)
	{
		float ca = player.angle - ra;
		float newDisT = disT;
		newDisT *= cos(ca);
		float lineH = (mapsize * 320) / newDisT;
		float ty_step = 32.0f / (float)lineH;
		float ty_off = 0;

		if (lineH > 320.0f)
		{
			ty_off = (lineH - 320) / 2.0f;
			lineH = 320.0f;
		}

		float lineO = 160 - lineH / 2.0f;

		//draws walls
		int y;
		float ty = ty_off * ty_step + mt * 32;
		float tx = 0;
		if (shade == 1) 
		{ 
			tx = (int)(rx / 2.0f) % 32; 
			if (ra > 180) { tx = 31 - tx; }
		}
		else
		{
			tx = (int)(ry / 2.0f) % 32; 
			if (ra > 90 && ra < 270) { tx = 31 - tx; }
		}

		//ty += 32;
		
		for (y = 0; y < lineH; y++)
		{
			float c = All_Textures[(int)(ty) * 32 + (int)(tx)] * shade;
			
			olc::Pixel p;

			if (mt == 0) { p = olc::PixelF(c, c / 2.0f, c / 2.0f); } //checkboard red
			if (mt == 1) { p = olc::PixelF(c / 2.0f,c /2.0f, c); } //window blue
			if (mt == 2) { p = olc::PixelF(c / 2.0f, c, c / 2.0f); } //door green
			if (mt == 3) { p = olc::PixelF(c , c, c / 2.0f); } //brick yellow
			FillRect(r * 8 + 530, lineO + y, 8, 1, p);
			ty += ty_step;
		}

		//draw floors
		for (y = lineO + lineH; y < 320; y++)
		{
			float dy = y - (320 / 2.0f), deg = degToRad(ra), raFix = cos(degToRad(FixAng(player.angle - ra)));
			tx = player.x / 2 + cos(deg) * 158 * 32 / dy / raFix;
			ty = player.y / 2 - sin(deg) * 158 * 32 / dy / raFix;
			float c = All_Textures[((int)(ty) & 31) * 32 + ((int)(tx) & 31)] * 0.7f;
			olc::Pixel p = olc::PixelF(c,c,c);
			FillRect(r * 8 + 530, y, 8, 1, p); 
		}
	}

	bool hasWallAt(float x, float y)
	{
		if (x < 0 || x >= mapx * mapsize || y < 0 || y >= mapy * mapsize)
			return true;

		
		int worldX = floor(x / mapsize);
		int worldy = floor(y / mapsize);
		int mp = (worldy * mapx) + worldX;
		return mapW[mp] != 0;
	}
	
public:



};

int main()
{
	Raycaster game;
	if (game.Construct(1024, 512, 1, 1))
	{
		game.Start();
	}

	return 0;
}