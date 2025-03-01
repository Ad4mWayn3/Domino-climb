#pragma once

#include "platformer.hpp"
#include "levelEditor.hpp"

#include <raygui.h>

enum class PauseButton {
	resume, toggle_gamemode, controls, save_map, load_map, exit_game
};

enum class Menu { menu_pause, menu_controls };

struct Button {
	Rectangle bounds;
	const char* text;
	static Button create(const char* text) {
		return {{.0f, .0f, .0f, .0f}, text};
	}
};

namespace gui {
	void init();
	std::vector<Button> buttonGrid(const std::vector<const char*>& buttonNames,
		Vector2 origin, Vector2 buttonSize, Vector2 spacing, size_t wrapSize,
		Axis wrapAxis);
		
	Menu pauseMenu(int& gameMode, bool& pause, GameData& gameData,
		EditData& editData);

	Menu controlsMenu();

	inline std::vector<Button>
		gameControlsButtons(controls_count),
		editControlsButtons(edit_count),
		pauseButtons;
};
