#pragma once

#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#define MIN_BOUNCE_SPEED 100

inline const std::map<KeyboardKey, const char*> KeyName {
#define kstr(x) {KEY_##x, #x},
#define ks(a,b,c,d,e) kstr(a) kstr(b) kstr(c) kstr(d) kstr(e)
	{KEY_NULL, "NULL"},
	ks(APOSTROPHE, COMMA, MINUS, PERIOD, SLASH)
	ks(ZERO, ONE, TWO, THREE, FOUR)
	ks(FIVE, SIX, SEVEN, EIGHT, NINE)
	ks(SEMICOLON, EQUAL, A, B, C)
	ks(D, E, F, G, H)
	ks(I, J, K, L, M)
	ks(N, O, P, Q, R)
	ks(S, T, U, V, W)
	ks(X, Y, Z, LEFT_BRACKET, BACKSLASH)
	kstr(RIGHT_BRACKET) kstr(GRAVE)
#undef ks
#undef kstr
};

using Rectangles = const std::vector<Rectangle>&;
using TimeSeconds = float;

inline const struct {
	int width, height;
} resolution{1280, 720};

inline const Vector2 resolutionV
	{(float)resolution.width, (float)resolution.height};

enum CamMode{mouselook=0, follow};

enum class Modes {play=0, edit, exit};
const unsigned modeCount = 2;

enum Axis {horizontal, vertical};

enum EditControls {
	scroll=0, select, create,
	resize_w, resize_h, move_up, move_down, move_left, move_right,
	copy, _delete,
	save, load, select_multiple, edit_count
};

enum Controls{up=0, down, left, right, jump, look, toggleCamera, reset, controls_count};

struct Control { int key; const char* name; };

// This is virtually the same as using an array and indexing with enum values, \
	but works consistently even when the enum literals are out of order. \
	This decision is not consistent because the maps from controls to binds in \
	EditData and GameData use arrays, which I probably have to fix, unless this\
	option causes overhead (but i entrust the compiler that it won't)
inline std::map<Controls, Control> gameControls{
	{up, {KEY_E, "up"}},						{down, {KEY_D, "down"}},
	{left, {KEY_S, "left"}},					{right, {KEY_F, "right"}},
	{jump, {KEY_SPACE, "jump"}},				{look, {KEY_NULL,"look"}},
	{toggleCamera, {KEY_TAB, "toggle camera"}},	{reset, {KEY_R, "reset"}},
};

inline std::map<EditControls, Control> editControls {
	{scroll, {MOUSE_BUTTON_RIGHT, "scroll"}},	{select, {MOUSE_BUTTON_LEFT, "select"}},
	{create, {KEY_F, "create"}},				{resize_w, {KEY_D, "horizontal resize"}},
	{resize_h, {KEY_S, "vertical resize"}},		{move_up, {KEY_UP, "move up"}},
	{move_down, {KEY_DOWN, "move down"}},		{move_left, {KEY_LEFT, "move left"}},
	{move_right, {KEY_RIGHT, "move right"}},	{copy, {KEY_C, "copy"}},
	{_delete, {KEY_DELETE, "delete"}},			{save, {KEY_K, "save"}},
	{load, {KEY_L, "load"}},					{select_multiple, {KEY_LEFT_SHIFT, "select multiple"}},
};

struct {
	float minLength = 4;
	const char* mapFileDir = "../resources/";
	const char* controlsDisplay =
		"Scroll:\tright mouse button\n"
		"Select:\tleft mouse button\n"
		"Create:\tF\n"
		"X Resize:\tS\n"
		"Y Resize:\tD\n"
		"Move block:\tArrow keys\n"
		"Clone:\tC\n"
		"Delete:\tDel\n\n\n"
		"Save map:\tK\n"
		"Load map:\tL\n";
} constData;

class Player {
	int x, y;
	
	void moveAxis(int moveAm, bool& axisCollide, Vector2 shift, Rectangles recs);
public:
	int height, width;
	Texture tex;
	bool collidingX=false, collidingY=false, onGround=false;
	Player(int _x, int _y, int _width, int _height)
	:	x{_x}, y{_y}
	,	width{_width}, height{_height}{}

	Player(int _x, int _y, int _width, int _height, Texture _tex)
	:	x{_x}, y{_y}
	,	width{_width}, height{_height}
	,	tex{_tex}{}

	int maxFixCollisionSize() { 
		int max = std::max(width, height),
			min = std::min(width, height);
		return (max - min) / 2; 
	}
	Vector2 position(){ return {(float)x, (float)y}; }
	Vector2 center(){ return {(float)x + width / 2.0f,
		(float)y + height / 2.0f}; }
	Rectangle rectangle(){
		return {(float)x, (float)y, (float)width, (float)height};
	}

	void draw();
	bool rotate(Vector2 axis, Rectangles recs, bool forceRotate = false);
	Player& moveX(float speed, Rectangles recs);
	Player& moveY(float speed, Rectangles recs);
	Player& move(Vector2 speed, Rectangles recs) {
		return (*this)
			.moveX(speed.x, recs)
			.moveY(speed.y, recs);
	}
};

struct GameData{
	int controls[controls_count]{KEY_E, KEY_D, KEY_S, KEY_F, KEY_SPACE,
		MOUSE_BUTTON_LEFT, KEY_TAB, KEY_R};
//	Rectangle player{resolution.width/2.0f, resolution.height/2.0f, 30, 60};
//	Rectangle structure{300,900,1800,30};
	Camera2D camera{{0,0}, {0,0}, 0, 1};
	std::vector<Rectangle> structures{};
	Vector2 maxVel{450,40000};
	Vector2 accel{1777,0};
	Vector2 gravity{0,2700};
	int cameraMode{mouselook};
	int cameraModeCount = 2;
	Player player{(int)resolution.width/2, (int)resolution.height/2, 40, 80};
	float jumpImpulse = 900;
};
