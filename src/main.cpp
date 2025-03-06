#include "gui.hpp"
#include "levelEditor.hpp"
#include "platformer.hpp"

#include <string>

int main() {
	resolution.width = 1920;
	resolution.height = resolution.width * 9 / 16;
	resolutionV = {(float)resolution.width, (float)resolution.height};
	SetTraceLogLevel(LOG_WARNING); // Logging only for priority at or above \
		warnings.
	InitWindow(resolution.width, resolution.height, "Domino climb");
	SetWindowMinSize(resolution.width, resolution.height);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetWindowMaxSize(1920, 1080);
	SetTargetFPS(60);
	SetExitKey(KEY_NULL);
	
	GameData gameData{};
	EditData editData{gameData.structures};
	editor::loadMap(gameData.structures, "resources/map.txt");
	gui::init();
	
	while (!WindowShouldClose()) {
		float delta = GetFrameTime();

		BeginDrawing();
		ClearBackground(BLACK);

		static int mode = (int)Modes::play;
		static Menu menu = Menu::menu_pause;
		static bool pause = false;

		switch (mode) {
		case (int)Modes::play:
			platformer::draw(gameData);
			if (!pause) platformer::process(gameData, delta);
			break;
		case (int)Modes::edit:
			if (!pause) editor::process(editData, delta);
			editor::draw(editData, delta, gameData.player.rectangle());
			break;
		}

		if (mode == (int)Modes::exit) break;

		if (IsKeyPressed(KEY_ESCAPE)) {
			pause = pause != true; // flips the value of pause
			menu = Menu::menu_pause;
		}

		if (pause) switch (menu) {
		case Menu::menu_pause:
			menu = gui::pauseMenu(mode, pause, gameData, editData);
			break;
		case Menu::menu_controls:
			menu = gui::controlsMenu();
			break;
		}

		if (IsWindowResized()) {
			resolutionV.x = GetScreenWidth();
			resolutionV.y = GetScreenHeight();
		}
		
		EndDrawing();
	}
	
	CloseWindow();
}
