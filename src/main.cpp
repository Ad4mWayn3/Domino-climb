#include "gui.hpp"
#include "levelEditor.hpp"
#include "platformer.hpp"

#include <string> 

struct { int key; std::string name; } toggleKey = { KEY_W, "W"};

int main() {
	SetTraceLogLevel(LOG_WARNING); // Logging only for priority at or above warnings.
	InitWindow(resolution.width, resolution.height, "collision test");
	
	GameData gameData{};
	gameData.camera = {{0,0}, {0,0}, 0, 1};
	editor::loadMap(gameData.structures, "resources/map.txt");
	EditData editData{gameData.structures};
	SetExitKey(KEY_NULL);
	guiInit();
	
	SetTargetFPS(60);
	while (!WindowShouldClose()) {
		float delta = GetFrameTime();

		BeginDrawing();
		ClearBackground(BLACK);

		static int mode = (int)Modes::play;
		static Menu menu = menu_pause;
		static bool pause = false;

		switch (mode) {
		case (int)Modes::play:
			process(delta, gameData);
			break;
		case (int)Modes::edit:
			editor::editProcess(delta, editData);
			break;
		}

		if (mode == (int)Modes::exit) break;

		if (IsKeyPressed(KEY_ESCAPE)) {
			pause = pause != true; // flips the value of pause
			menu = menu_pause;
		}

		if (pause) switch (menu) {
		case menu_pause:
			menu = pauseMenu(mode, pause, gameData, editData);
			break;
		case menu_controls:
			menu = controlsMenu();
			break;
		}
		
		EndDrawing();
	}
	
	CloseWindow();
}
