#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <vector>

using Vel2 = Vector2;
using Line = Vector4;

//struct Map;
//struct Player;

struct Game {
	using TimeSeconds = float;
	Map map;
	Player player;
	void init();
	void update(TimeSeconds delta);
	void draw();
};

struct Triangle {
	Vector2 p[3];
	bool isCCW();
	void draw();
	static Triangle unordered(Vector2 p[3]);
	static Triangle slope(Line l);
};

struct Map {
	std::vector<Vector2> vertices;
	std::vector<size_t[2]> iLines;
	std::vector<size_t[3]> iTriangles;
	template <class T>
	bool colliding(T& solid);
	void draw();
	void save(const char* fileName);
	void load(const char* fileName);
	Vector2 vertexAt(size_t i);
	Line lineAt(size_t i);
	Triangle triangleAt(size_t i);
};

struct Player {
	Vector2 pos;
	Vel2 rmd;
	float size;
	Vel2 move(Map& map, Vel2 vel);
};

void Game::init() {
	map.load("resources/map2.txt");
}

void Game::update(TimeSeconds delta) {

}

void Game::draw() {

}

void Map::draw() {
	
}

Vector2 Player::move(Map& map, Vel2 vel) {
	pos += vel;

	if (!map.colliding<Player>(*this))
		return vel;

	if (Vector2LengthSqr(vel) == 0.f) {
		TraceLog(LOG_WARNING, "Velocity is zero, player is stuck.");
		return {0., 0.};
	}
	Vel2 step = Vector2Normalize(vel);
	while (map.colliding<Player>(*this))
		pos -= step;
}

int main() {
	SetTraceLogLevel(LOG_WARNING);
	SetExitKey(KEY_NULL);
	InitWindow(800, 600, "marble!");

	Game game;
	game.init();

	while (!WindowShouldClose()) {
		Game::TimeSeconds delta = GetFrameTime();
		game.update(delta);
		BeginDrawing();
		game.draw();
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
