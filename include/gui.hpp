#pragma once

#include "data.hpp"

#include <raygui.h>

enum Menu { menu_pause, menu_controls };
struct Button {
	Rectangle bounds;
	const char* text;
	static Button create(const char* text) {
		return {{.0f, .0f, .0f, .0f}, text};
	}
};

void guiInit();

void fitButtonsToGrid(std::vector<Button>& buttons, Rectangle bounds,
	size_t rows, size_t cols, float spacing);

std::vector<Button> buttonGrid(const std::vector<const char*>& buttonNames,
	Vector2 origin, Vector2 buttonSize, Vector2 spacing, size_t wrapSize,
	Axis wrapAxis);

Menu pauseMenu(int& gameMode, int gameModeCount);

Menu controlsMenu();

enum class PauseButton {
	resume, toggle_gamemode, controls, save_map, load_map, exit_game
};

inline std::vector<Button>
	gameControlsButtons(controlsCount),
	editControlsButtons(editControlsCount),
	pauseButtons;
