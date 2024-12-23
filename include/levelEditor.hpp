#pragma once

#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

enum EditControls {
	scroll=0, select, create,
	resize_w, resize_h, move_up, move_down, move_left, move_right,
	copy, _delete,
	save, load
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

struct EditData {
	int controls[13]{
		MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_LEFT, KEY_F,
		KEY_S, KEY_D, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
		KEY_C, KEY_DELETE,
		KEY_K, KEY_L
	};
	std::vector<Rectangle>& structures;
	Rectangle* selectedStructure = nullptr;
	Camera2D camera{{0,0},{0,0},0.0f,1.0f};

	EditData(std::vector<Rectangle>& _structures)
	:	structures{_structures}{}
};

void editProcess(float delta, EditData& editData);

void loadMap(std::vector<Rectangle>& mapData, const char* mapFileName);

void saveMap(const std::vector<Rectangle>& mapData, const char* mapFileName);
