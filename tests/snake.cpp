#include <raylib.h>
#include <raymath.h>
#include <vector>

enum Direction { stop, up, down, left, right };

template <class T>
struct Ring {
	std::vector<T> data;
	size_t size; // why store size separately when data.size() is available?
	size_t begin;
	T& operator[](int i) {
		return data[abs((int)begin + i) % size];
	}
	void push(T i) {
		data.push_back(i);
		size += 1;
		begin = abs((int)begin-1)%size;
		for (int i=size-1; i > begin; --i)
			std::swap(data[i], data[i-1]);
	}
	void place(T i) {
		begin = abs((int)begin-1)%size;
		data[begin] = i;
	}
};

int main() {
	SetTraceLogLevel(LOG_WARNING);
	SetTargetFPS(12);
	InitWindow(800, 600, "snake");

	Direction dir = stop;
	Vector2 head = {0., 0.};
	Ring<Vector2> body;
	body.data.reserve(200);
	body.size = body.data.size();
	body.push(head);
	float stepSize = GetScreenWidth() / 30.;
	Vector2 iHat{1., 0.}, jHat{0., 1.};

	while (!WindowShouldClose()) {
		#define k(x) (IsKeyDown(KEY_##x))
		if k(E) { if (dir != down) dir = up; }
		//else
		if k(D) { if (dir != up) dir = down; }
		if k(S) if (dir != right) dir = left;
		if k(F) if (dir != left) dir = right;
		if (IsKeyPressed(KEY_G)) body.push(head);
		#undef k

		switch (dir) {
		case up: head -= jHat * stepSize;
			break;
		case down: head += jHat * stepSize;
			break;
		case left: head -= iHat * stepSize;
			break;
		case right: head += iHat * stepSize;
			break;
		//default: break;
		}

		body.place(head);

		BeginDrawing();
		ClearBackground(BLACK);
		for (auto& v : body.data)
			DrawCircleV(v, stepSize/2., WHITE);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
