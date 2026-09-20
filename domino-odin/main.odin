package main

import rl "vendor:raylib"

import "core:debug/trace"
import "core:fmt"
import "core:os"
import "core:mem"
import "core:slice"

game: Game
main :: proc() {
	track: trace.Tracking_Allocator
	trace.tracking_allocator_init(&track, context.allocator)
	defer trace.tracking_allocator_destroy(&track)

	context.allocator = trace.tracking_allocator(&track)
	defer trace.tracking_allocator_print_results(&track)

	context.assertion_failure_proc = trace.assertion_failure_proc

	game = game_init("res/level_fix.bin")
	defer game_deinit(&game)

	rl.SetTraceLogLevel(.WARNING)
	rl.SetConfigFlags({.WINDOW_RESIZABLE, .MSAA_4X_HINT})
	rl.InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game")
	defer rl.CloseWindow()

	for !rl.WindowShouldClose() {
		game_update(&game, rl.GetFrameTime())
		game_draw(game)
	}
}
