#pragma once

#include <raylib.h>
#include <raymath.h>

#include <iostream>
#include <sstream>
#include <vector>

#define MIN_BOUNCE_SPEED 100

using Rectangles = const std::vector<Rectangle>&;

const struct {
	int width, height;
} resolution{1920, 1080};

class Player {
	int x, y;
	int height, width;
	bool collidingX=false, collidingY=false;

	void moveAxis(int moveAm, bool& axisCollide, Vector2 shift, Rectangles recs) {
		while (moveAm) {
			if (checkCollisions({float(x+shift.x), float(y+shift.y), \
				(float)width, (float)height}, recs)) {
				axisCollide = true;
				return;
			}
			x += shift.x;
			y += shift.y;
			moveAm -= shift.x + shift.y;
		}
		axisCollide = false;
	}
public:
	Texture tex;
	Player(int _x, int _y, int _width, int _height, Texture _tex)
	:	x{_x}, y{_y}
	,	width{_width}, height{_height}
	,	tex{_tex}{}

	Vector2 position(){ return {(float)x, (float)y}; }
	Rectangle rectangle(){ return {(float)x, (float)y, (float)width,
		(float)height};
	}

	Player& moveX(float speed, Rectangles recs);
	Player& moveY(float speed, Rectangles recs);
	Player& move(Vector2 speed, Rectangles recs) {
		return (*this)
			.moveX(speed.x, recs)
			.moveY(speed.y, recs);
	}
};

enum Controls{up=0, down, left, right, jump, reset};

struct GameData{
	KeyboardKey controls[6]{KEY_E, KEY_D, KEY_S, KEY_F, KEY_SPACE, KEY_R};
	Rectangle player{resolution.width/2.0f, resolution.height/2.0f, 60, 60};
	Rectangle structure{300,900,1800,30};
	std::vector<Rectangle> structures{};
	Vector2 maxVel{454,1200};
	Vector2 accel{1777,0};
	Vector2 gravity{0,1654};
	float jumpImpulse = 647;
};


void _process(float delta, GameData& gameData);

bool checkCollisions(Rectangle rec, Rectangles recs);