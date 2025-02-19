#include "gui.hpp"
#include "levelEditor.hpp"
#include "platformer.hpp"

#include <string> 

enum Modes {play=0, edit};
struct { int key; std::string name; } toggleKey = { KEY_W, "W"};
const unsigned modeCount = 2;

int main() {
	SetTraceLogLevel(LOG_WARNING); // Logging only for priority at or above warnings.
	InitWindow(resolution.width, resolution.height, "collision test");
	SetTargetFPS(60);

	GameData gameData{};
	gameData.camera = {{0,0}, {0,0}, 0, 1};
	loadMap(gameData.structures, "resources/map.txt");
	EditData editData{gameData.structures};
	SetExitKey(KEY_NULL);
	guiInit();

	while (!WindowShouldClose()) {
		float delta = GetFrameTime();

		BeginDrawing();
		ClearBackground(BLACK);

		static int mode = play;
		static Menu menu = menu_pause;
		static bool pause = false;

		switch (mode) {
		case play:
			process(delta, gameData);
			break;
		case edit:
			editProcess(delta, editData);
			break;
		}

		if (IsKeyPressed(KEY_ESCAPE))
			pause = pause != true; // flips the value of pause

		if (pause) switch (menu) {
		case menu_pause:
			menu = pauseMenu(mode, 2);
			break;
		case menu_controls:
			menu = controlsMenu();
			break;
		}
		
		EndDrawing();
	}
	
	CloseWindow();
}
