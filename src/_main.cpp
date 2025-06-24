#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <array>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <vector>

std::vector<Rectangle> loadMap(size_t capacity, const char* mapPath) {
	std::ifstream mapFile{mapPath};
	std::vector<Rectangle> map;
	map.reserve(capacity);

	Rectangle rec;
	while (true) {
		if (!(mapFile >> rec.x)) break;
		if (!(mapFile >> rec.y)) break;
		if (!(mapFile >> rec.width)) break;
		if (!(mapFile >> rec.height)) break;
		map.push_back(rec);
	}

	mapFile.close();
	return map;
}

enum class Input {
	up, down, left, right,
};

bool colliding(Rectangle r, std::vector<Rectangle>& world) {
	for (auto r_i : world) {
		if (CheckCollisionRecs(r, r_i)) return true;
	}
	return false;
}

struct ivec2 { int x, y; };
short sign(double x) { return signbit(x)? -1 : 1; }
double fract(double x) { return x - floor(x); }

void move(Rectangle& r, std::vector<Rectangle>& world, Vector2& rmd,
	Vector2 vel) {
	auto step = Vector2Normalize(vel);
	int steps = (int)Vector2Length(vel);
	rmd += vel - step * (float)steps;
	if (Vector2LengthSqr(rmd) >= 1.) {
		rmd -= Vector2Normalize(rmd);
		steps += 1;
	}

	for (; steps; --steps) {
		r.x += step.x;
		r.y += step.y;
		if (colliding(r, world)) {
			r.x -= step.x;
			if (!colliding(r,world)) {
				step.x = 0.f;
				continue;
			}
			r.y -= step.y;
			if (!colliding(r,world))
				step.y = 0.f;
			if (abs(step.x) + abs(step.y) < 1e-5f) // if step ~ 0
				return;
		}
	}
}

int main() {
	auto resolution = Vector2{1920., 1080.};
	SetTraceLogLevel(LOG_WARNING);
	InitWindow((int)resolution.x, (int)resolution.y, "platformer!");
	SetTargetFPS(60);
	SetExitKey(KEY_NULL);

	using i = Input;
	std::unordered_map<Input, KeyboardKey> controls {
		{i::up, KEY_W}, {i::down, KEY_S}, {i::left, KEY_A},
		{i::right, KEY_D},
	};

	using PixPerSec = float;
	PixPerSec speed = 300.f;
	auto map = loadMap(100, "resources/map.txt");
	auto player = Rectangle{resolution.x / 2.f, resolution.y / 2.f, 50.f, 50.f};
	auto vel = Vector2{0.f,0.f};
	auto rmd = Vector2{0.f,0.f};
	auto camera = Camera2D{ {0.,0.}, {0.,0.}, 0., 1.};
	char text[512]{};
	Vector2 points[4]{};
	size_t selectedPoint = 3;

	using TimeSeconds = float;
	while (!WindowShouldClose()) {
		TimeSeconds delta = GetFrameTime();

		vel = Vector2{0., 0.};
		if (IsKeyDown(controls[i::up])) {
			vel.y = -speed * delta;
		}
		if (IsKeyDown(controls[i::down])) {
			vel.y = speed * delta;
		}
		if (IsKeyDown(controls[i::left])) {
			vel.x = -speed * delta;
		}
		if (IsKeyDown(controls[i::right])) {
			vel.x = speed * delta;
		}
		if (IsKeyPressed(KEY_ONE))
			selectedPoint = 0;
		if (IsKeyPressed(KEY_TWO))
			selectedPoint = 1;
		if (IsKeyPressed(KEY_THREE))
			selectedPoint = 2;
		if (IsKeyPressed(KEY_FOUR))
			selectedPoint = 3;
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			points[selectedPoint] = GetMousePosition();
			if (selectedPoint == 3) {
				points[0] += GetMouseDelta();
				points[1] += GetMouseDelta();
				points[2] += GetMouseDelta();
			}
		}

		move(player, map, rmd, vel);
		sprintf(text, "Velocity = {%f,\t%f}\n"
			"Position = {%f,\t%f}\n"
			"Selected vertex: %i\n"
			"FPS: %f",
			vel.x, vel.y, 
			player.x, player.y,
			selectedPoint,
			1./delta);
		DrawText(text, 40, 40, 20, WHITE);
		camera.target = Vector2{player.x, player.y} - resolution / 2.;

		BeginDrawing();

		ClearBackground(DARKGRAY);

		BeginMode2D(camera);
		for (auto platform : map)
			DrawRectangleRec(platform, LIGHTGRAY);
		DrawTriangle({300., 400.}, {421., 400.}, {380., 200.}, LIGHTGRAY);
		DrawRectangleRec(player, BLUE);
		EndMode2D();

		DrawTriangle(points[0], points[1], points[2], LIGHTGRAY);
		#define p(i) DrawText(#i, (int)points[i-1].x, (int)points[i-1].y, 20, WHITE);
		p(1); p(2); p(3);
		#undef p

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
