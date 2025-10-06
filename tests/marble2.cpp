#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

using f = float[];

int main() {
	struct { int width; int height; } screen {800, 600};
	SetTraceLogLevel(LOG_WARNING);
	SetExitKey(KEY_NULL);
	InitWindow(screen.width, screen.height, "shader!");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	auto shader = LoadShader("tests/test.vert", "tests/test.frag");
	auto timeu = GetShaderLocation(shader, "time");
	auto winSizeu = GetShaderLocation(shader, "winSize");

	auto mvp = MatrixMultiply(rlGetMatrixProjection(), rlGetMatrixModelview());
	auto tex = LoadTexture("resources/domino.png");

	while (!WindowShouldClose()) {
		SetShaderValue(shader, winSizeu, f{(float)GetScreenWidth(),
			(float)GetScreenHeight()}, SHADER_UNIFORM_VEC2);
		SetShaderValue(shader, timeu, f{(float)GetTime()},
			SHADER_UNIFORM_FLOAT);

		BeginDrawing();
		ClearBackground(BLACK);
		DrawFPS(20, 20);

		BeginShaderMode(shader);
		DrawTexturePro(tex, {0.,0.,200.,200.}, {400.,90.,200.,200.}, {0.,0.}, 0.,
			WHITE);
		EndShaderMode();
		DrawTexturePro(tex, {0.,0.,200.,200.}, {90.,90.,200.,200.}, {0.,0.}, 0.,
			WHITE);
		EndDrawing();
	}

	UnloadShader(shader);
	CloseWindow();
}
