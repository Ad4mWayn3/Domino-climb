#pragma once

#include "data.hpp"

enum class Position {
	upper_left=0, upper_center, upper_right,
	center_left, center, center_right,
	lower_left, lower_center, lower_right,
	none
};
struct EditData {
	int controls[edit_count]{ // controls intended to be indexed by EditControls values
		MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_LEFT, KEY_F,
		KEY_S, KEY_D, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
		KEY_C, KEY_DELETE,
		KEY_K, KEY_L, KEY_LEFT_SHIFT
	};
	std::vector<Rectangle>& structures;
	std::vector<Rectangle*> selectedStructures;
	std::vector<Rectangle*> clipboard;
	TimeSeconds inputBufferDelay = 0.2f;
	Rectangle* selectedStructure = nullptr;
	Rectangle selector = {0.0f, 0.0f, 0.0f, 0.0f};
	Rectangle resizeBox = {0.0f, 0.0f, 0.0f, 0.0f};
	Camera2D camera{{0,0},{0,0},0.0f,0.8f};
	bool creating = false;
//	bool resizing = false;
	Position resizePos = Position::none;
	Rectangle newStructure{0.0f, 0.0f, 0.0f, 0.0f};

	EditData(std::vector<Rectangle>& _structures)
	:	structures{_structures}{}
};

namespace editor {
	const float resizeButtonSize = 8.0f;
	void draw(EditData& editData, TimeSeconds delta, Rectangle player); 
	void process(EditData& editData, TimeSeconds delta);
	void loadMap(std::vector<Rectangle>& mapData, const char* mapFileName);
	void saveMap(const std::vector<Rectangle>& mapData, const char* mapFileName);
};
