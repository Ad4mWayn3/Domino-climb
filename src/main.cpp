#include "platformer.hpp"
#include "levelEditor.hpp"

#include <string>

enum Modes {play=0, edit};
struct { int key; std::string name; } toggleKey = { KEY_W, "W"};
const unsigned modeCount = 2;

int main() {
	InitWindow(resolution.width, resolution.height, "collision test");
	SetTargetFPS(60);
	GameData gameData{};
	loadMap(gameData.structures, "resources/map.txt");
	EditData editData{gameData.structures};
	
	while (!WindowShouldClose()) {
		float delta = GetFrameTime();

		BeginDrawing();
		ClearBackground(BLACK);

		static int mode = play;
		if (IsKeyPressed(toggleKey.key)) {
			mode += 1;
			mode %= modeCount;
		}

		switch (mode) {
		case play:
			_process(delta, gameData);
			break;
		case edit:
			editProcess(delta, editData);
			break;
		}

		std::stringstream ss;
		ss << "Press " << toggleKey.name << " to toggle gamemodes";

		DrawText(ss.str().c_str(), 500, 30, 32, GOLD);
		
		EndDrawing();
	}
	
	CloseWindow();
}
