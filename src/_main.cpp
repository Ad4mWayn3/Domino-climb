#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#ifndef RAYGUI_IMPLEMENTATION
#define RAYGUI_IMPLEMENTATION
#endif
#include <raygui.h>

#include <array>
#include <cassert>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <vector>

constexpr double TAU = PI * 2.;

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

struct Triangle {
	Vector2 p1, p2, p3;
};

// signed area of the parallelogram described by u and v
float wedge(Vector2 u, Vector2 v) {
	return v.x*u.y - v.y*u.x;
}

// returns the normal of v with greater angle betweeclsn u, or 0 if 
Vector2 opposingNormal(Vector2 v, Vector2 u) {
	Vector2 vn1 = {-v.y, v.x}, vn2 = {v.y, -v.x};
	printf("opposingNormal: %f, %f\n", Vector2Angle(vn1, u), Vector2Angle(vn2, u));
	if (abs(Vector2Angle(vn1, u)) > TAU/4.) return vn2;
	else if (abs(Vector2Angle(vn2, u)) > TAU/4.) return vn1;
	else return {0., 0.};
}

// checks if the points of the triangle are ordered counter-clockwise
bool isCCW(Triangle t) {
	return wedge(t.p2 - t.p1, t.p3 - t.p1) >= 0.f;
}

Triangle unordered(Vector2 p1, Vector2 p2, Vector2 p3) {
	auto t = Triangle{p1, p2, p3};
	if (!isCCW(t)) std::swap(t.p2, t.p3);
	return t;
}

Triangle slope(Vector2 p1, Vector2 p2) {
	Triangle t = {p1, p2, {p1.x, p2.y}};
	if (!isCCW(t)) std::swap(t.p2, t.p3);
	return t;
}

Rectangle recV(Vector2 p1, Vector2 p2) {
	auto size = p1 - p2;
	size.x = abs(size.x);
	size.y = abs(size.y);
	return {fmin(p1.x, p2.x), fmin(p1.y, p2.y), size.x, size.y};
}

bool colliding(Rectangle r, Triangle t) {
	return CheckCollisionPointTriangle({r.x,r.y}, t.p1, t.p2, t.p3);
}

bool colliding(Rectangle r, std::vector<Triangle>& world) {
	for (auto e : world) {
		if (colliding(r, e)) return true;
	}
	return false;
}

class Node {
public:
	using TimeSeconds = double;
	virtual void init() = 0;
	virtual void update(TimeSeconds delta) = 0;
	virtual void draw() = 0;
};

Vector4 makeLine(Vector2 p0, Vector2 p1) {
	return {p0.x, p0.y, p1.x, p1.y};
}

bool getVertexAt(const std::vector<Vector2>& vertices, Vector2 pos, size_t& index) {
	for (int i=0; i < vertices.size(); ++i) {
		if (CheckCollisionPointCircle(pos, vertices[i], 10.f)) {
			index = i;
			return true;
		}
	}
	return false;
}

void getVerticesAt(const std::vector<Vector2>& vertices,
	std::vector<size_t>& inside, const Rectangle& selector, bool append) {
	TraceLog(LOG_INFO, "getVerticesAt(rec %f %f %f %f)", selector.x, selector.y,
		selector.width, selector.height);

	if (!append) inside.resize(0);
	if (selector.width * selector.height == 0.f) {
		size_t i;
		if (getVertexAt(vertices, {selector.x, selector.y}, i))
			inside.push_back(i);
		return;
	}
	for (size_t i=0; i < vertices.size(); ++i) {
		if (CheckCollisionPointRec(vertices[i], selector))
			inside.push_back(i);
	}
}

Vector2 screenMiddleV() {
	return {GetScreenWidth()/2.f, GetScreenHeight()/2.f};
}

struct World {
	std::vector<Vector2> vertices;
	std::vector<std::array<size_t,3>> triangleIds;
	std::vector<std::array<size_t,3>> slopeIds;
	std::vector<std::array<size_t,3>> lineIds;
	Triangle triangleAt(size_t i0, size_t i1, size_t i2);
	Triangle slopeAt(size_t i0, size_t i1);
};

Triangle World::triangleAt(size_t i0, size_t i1, size_t i2) {
	auto t = Triangle{vertices[i0], vertices[i1], vertices[i2]};
	assert(isCCW(t));
	return t;
}

Triangle World::slopeAt(size_t i0, size_t i1) {
	return slope(vertices[i0], vertices[i1]);
}

struct Player : Node {
	World& world;
	Camera2D camera;
	Vector2 pos;
	Vector2 vel;
	Vector2 velRmd;
	Vector2 gravity;
	float accel;
	float size;
	bool inContact;
	Player(World& w) : world{w}{}
	void move();
	bool collidingTriangle(Triangle t,Vector4& line);
	bool colliding(Vector4& line);
	void init();
	void update(TimeSeconds delta);
	void draw();
	Vector2 onCollide(Vector4 line);
};

bool Player::collidingTriangle(Triangle t, Vector4& line) {
	//Vector2 line;
	bool cl1 = CheckCollisionCircleLine(pos, size, t.p1, t.p2);
	bool cl2 = CheckCollisionCircleLine(pos, size, t.p2, t.p3);
	bool cl3 = CheckCollisionCircleLine(pos, size, t.p3, t.p1);
	if (cl1) line = makeLine(t.p1, t.p2);
	if (cl2) line = makeLine(t.p2, t.p3);
	if (cl3) line = makeLine(t.p3, t.p1);
	if (cl1 || cl2 || cl3) return true;
	return false;
}

bool Player::colliding(Vector4& line) {
	for (auto& tid : world.triangleIds) {
		auto t = world.triangleAt(tid[0], tid[1], tid[2]);
		if (collidingTriangle(t, line)) return true;
	}
	for (auto& sid : world.slopeIds) {
		auto t = world.slopeAt(sid[0], sid[1]);
		if (collidingTriangle(t, line)) return true;
	}
	for (auto& lid : world.lineIds) {
		auto v0 = world.vertices[lid[0]], v1 = world.vertices[lid[1]];
		if (CheckCollisionCircleLine(pos, size, v0, v1)){
			line = makeLine(v0, v1);
			return true;
		}
	}
	return false;
}

Vector2 Player::onCollide(Vector4 line) {
	Vector2 v = Vector2{line.x, line.y} - Vector2{line.z, line.w};
	return (v/Vector2LengthSqr(v))*Vector2DotProduct(vel, v);
}

void Player::move() {
	TraceLog(LOG_INFO, "Player::move: 1");
	auto step = Vector2Normalize(vel);
	int stepCount = (int)Vector2Length(vel);
	velRmd += vel - step * (float)stepCount;
	if (Vector2LengthSqr(velRmd) >= 1.) {
		velRmd -= Vector2Normalize(velRmd);
		stepCount += 1;
	}

	Vector4 line;
	for (; stepCount > 0; --stepCount) {
		pos += step;
		if (colliding(line)) {
			pos -= step;
			inContact = true;
			vel = onCollide(line);
			return;
		}
	}
	TraceLog(LOG_INFO, "Player::move: 2");
	inContact = false;
}

void Player::init() {
	accel = 1200.f;
	pos = {0.f,0.f};
	vel = {0.f, 0.f};
	velRmd = {0.f, 0.f};
	gravity = {0.f, accel};
	size = 15.f;
	camera = {screenMiddleV(), {0.,0.}, 0.f, 1.f};
	inContact = false;
}

void Player::draw() {
	DrawCircleV(pos, size, WHITE);
}

void Player::update(TimeSeconds delta) {
	TraceLog(LOG_INFO, "Player::update:");
	if (IsKeyDown(KEY_F))
		vel.x += accel * delta;
	if (IsKeyDown(KEY_S))
		vel.x -= accel * delta;
	if (IsKeyDown(KEY_R))
		init();

	float maxVel = 1300.f;
	vel.x = Clamp(vel.x, -maxVel, maxVel);
	pos += vel * delta;

	Vector4 line;
	if (colliding(line))
		vel = onCollide(line);
	//if (!inContact) vel += gravity * delta;
	camera.target = pos;
}

struct Editor : Node {
	enum class Mode {game, editor, count};
	std::vector<Vector2>& vertices;
	std::vector<std::array<size_t,3>>& triangleIds;
	std::vector<std::array<size_t,3>>& slopeIds;
	std::vector<std::array<size_t,3>>& lineIds;
	World world;
	Player player;
	std::vector<size_t> selectedVertices;
	Vector2 selector[2];
	Camera2D camera;
	Mode mode;
	bool selectMultiple;
	bool selecting;
	Editor()
	:	vertices{world.vertices}
	,	triangleIds{world.triangleIds}
	,	slopeIds{world.slopeIds}
	,	lineIds{world.lineIds}
	,	player{world}{}
	void init();
	void draw();
	void createInput();
	void editInput(TimeSeconds delta);
	void selectInput();
	void update(TimeSeconds delta);
	void editorUpdate(TimeSeconds delta);
	void gameUpdate(TimeSeconds delta);
};

void Editor::editInput(TimeSeconds delta) {
	selectMultiple = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
	Vector2 mousePos = GetMousePosition() / camera.zoom + (camera.target-camera.offset);

	if (IsKeyDown(KEY_UP))
		camera.zoom += 0.4 * delta;
	
	if (IsKeyDown(KEY_DOWN))
		camera.zoom -= 0.4 * delta;

	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		camera.target -= GetMouseDelta();
	}
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		selector[0] = mousePos;
		selecting = true;
	}
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		selector[1] = mousePos;
	} else {
		Rectangle selectorR = recV(selector[0], selector[1]);
		if (selecting) getVerticesAt(vertices, selectedVertices, selectorR,
			selectMultiple);
		selecting = false;
		selector[1] = selector[0];
	}

	if (IsKeyPressed(KEY_T)) for (size_t s=0; s+2<selectedVertices.size(); s += 2)
		triangleIds.push_back({selectedVertices[s],
			selectedVertices[s+1],
			selectedVertices[s+2]});

	if (IsKeyPressed(KEY_S)) for (size_t s=0; s+1<selectedVertices.size(); ++s)
		slopeIds.push_back({selectedVertices[s], selectedVertices[s+1]});
	
	if (IsKeyPressed(KEY_E)) for (size_t s=0; s+1<selectedVertices.size(); ++s)
		lineIds.push_back({selectedVertices[s], selectedVertices[s+1]});

	if (IsKeyPressed(KEY_V)) vertices.push_back(mousePos);
}

void Editor::init() {
	TraceLog(LOG_INFO, "Editor::init()");
	const size_t vcap = 120;
	mode = Mode::editor;
	vertices.reserve(vcap);
	triangleIds.reserve(vcap/3);
	slopeIds.reserve(vcap/2);
	lineIds.reserve(vcap/2);
	selectedVertices.reserve(vcap/3);
	player.init();
	camera = {screenMiddleV(), {0., 0.}, 0., 1.};
}

void Editor::draw() {
	char text[0xff];
	sprintf(text, "Player.vel = {%f,\t%f}\n"
		"camera.zoom = %f",
		player.vel.x, player.vel.y,
		camera.zoom);
	DrawText(text, 30, 30, 20, WHITE);

	auto c = mode == Mode::editor? camera : player.camera;
	BeginMode2D(c);
	player.draw();
	DrawCircle(0, 0, 30, WHITE);
	DrawRectangleRec(recV(selector[0], selector[1]), Color {255, 255, 255, 0x7f});

	for (auto& v : vertices) {
		DrawCircleV(v, 9, WHITE);
	}
	
	for (auto t : triangleIds) {
		auto tr = unordered(vertices[t[0]], vertices[t[1]], vertices[t[2]]);
		DrawTriangle(tr.p1, tr.p2, tr.p3, WHITE);
	}
	
	auto inRange = [](int x, int l, int h){ return x >= l && x <= h; };
	for (auto& s : slopeIds) {
		if (!inRange(s[0], 0, vertices.size()-1) ||
		!inRange(s[1], 0, vertices.size()-1)) {
			std::swap(s, slopeIds[slopeIds.size()-1]);
			slopeIds.pop_back();
			continue;
		}
		Triangle t = slope(vertices[s[0]], vertices[s[1]]);
		DrawTriangle(t.p1, t.p2, t.p3, LIGHTGRAY);
	}
	
	for (auto& lid : lineIds)
		DrawLineEx(vertices[lid[0]], vertices[lid[1]], 2.f, LIGHTGRAY);
	
	for (auto i : selectedVertices)
		DrawCircleV(vertices[i], 12, RED);
	EndMode2D();
}

void Editor::update(TimeSeconds delta) {
	if (GuiButton({100.,30.,200.,40.}, "change gamemode"))
		mode = Mode(((int)mode + 1) % (int)Mode::count); // ugly cast because enum classes dont convert implicitly.
	switch(mode) {
	case Mode::game:
		player.update(delta);
		return;
	case Mode::editor:
		editInput(delta);
		return;
	}
}

int _main() {
	Vector2 resolution{1920., 1080.};
	Vector2 x{100, 0};
	SetTraceLogLevel(LOG_WARNING);
	InitWindow((int)resolution.x, (int)resolution.y, "editor");
	SetExitKey(KEY_NULL);
	//SetTargetFPS(10);

	Editor node = Editor();
	node.init();

	while (!WindowShouldClose()) {
		Node::TimeSeconds delta = GetFrameTime();
		node.update(delta);
		BeginDrawing();
		DrawFPS(60, 60);
		ClearBackground(DARKGRAY);
		node.draw();
		EndDrawing();
	}

	CloseWindow();
}

int main() {
	SetTraceLogLevel(LOG_WARNING);
	InitWindow(1920, 1080, "marble test");

	World world;
	Player player{world};
	player.init();

	
}
