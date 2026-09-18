package main

import rl "vendor:raylib"

import "core:debug/trace"
import "core:fmt"
import "core:os"
import "core:mem"
import "core:slice"

_main :: proc() {
	rl.SetTraceLogLevel(.WARNING)
	rl.SetConfigFlags({.WINDOW_RESIZABLE, .MSAA_4X_HINT})
	rl.InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game")
	defer rl.CloseWindow()

	file, file_err := os.open("res/level_fix.bin")
	ensure(file_err == os.ERROR_NONE)
	sz, sz_err := os.file_size(file)
	ensure(sz_err == os.ERROR_NONE)

	buffer := make([]u8, sz)
	defer delete(buffer)
	_, read_err := os.read(file, buffer)
	ensure(read_err == os.ERROR_NONE)
	assert(sz % size_of(rl.Rectangle) == 0)
	os.close(file)

	recs := slice_cast(rl.Rectangle, buffer)
	//recs_soa := soa_from_aos(recs)

	// level_file, level_file_err := os.open("res/map.bin",
	// 	flags = {.Create, .Read, .Write})
	// ensure(level_file_err == os.ERROR_NONE)
	// {
	// 	offset: uintptr = 0
	// 	x,y,w,h := soa_unzip(recs_soa)
	// 	os.write_at(file, slice_cast(u8,x), cast(i64)offset)
	// 	offset += slice_size_of(x)
	// 	os.write_at(file, slice_cast(u8,y), cast(i64)offset)
	// 	offset += slice_size_of(y)
	// 	os.write_at(file, slice_cast(u8,w), cast(i64)offset)
	// 	offset += slice_size_of(w)
	// 	os.write_at(file, slice_cast(u8,h), cast(i64)offset)
	// 	offset += slice_size_of(h)
	// }
	// os.close(level_file)

	{
		file, err := os.open("res/map.bin")
		ensure(err == os.ERROR_NONE)
		sz, _ := os.file_size(file)
		defer os.close(file)
		level_fix := make([]rl.Rectangle, sz/size_of(rl.Rectangle))
		defer delete(level_fix)
		_, err2 := os.read_slice(file, level_fix)
		ensure(err2 == os.ERROR_NONE)

		for &rec in level_fix do rec = rec_clip_space(SCREEN, rec)

		level_fix_file, err3 := os.open("res/level_fix.bin",
			flags = {.Create, .Read, .Write})
		ensure(err3 == os.ERROR_NONE)
		defer os.close(level_fix_file)
		os.write_slice(level_fix_file, level_fix)
	}

	camera := rl.Camera2D{offset={0,0}, target={0,0}, rotation=0, zoom=1}
	CAM_ACCEL :: 400

	for !rl.WindowShouldClose() {
		delta := rl.GetFrameTime()
		if rl.IsKeyDown(.E) {
			camera.target.y -= CAM_ACCEL * delta
		} if rl.IsKeyDown(.D) {
			camera.target.y += CAM_ACCEL * delta
		} if rl.IsKeyDown(.F) {
			camera.target.x += CAM_ACCEL * delta
		} if rl.IsKeyDown(.S) {
			camera.target.x -= CAM_ACCEL * delta
		}

		rl.BeginDrawing()
		rl.ClearBackground(rl.BLACK)
		//rl.DrawRectangleRec({x=0,y=0,width=300,height=100}, rl.BEIGE)
		rl.BeginMode2D(camera)
		rl.DrawCircleV(camera.target, 3., rl.BEIGE)
		for rec in recs {
			fmt.printf("drawing rec %v\n", rec_screen_space(SCREEN, rec))
			rl.DrawRectangleRec(rec_screen_space(SCREEN, rec), rl.BEIGE)
			//fmt.printf("i = %i, x = %f, w = %f\n", i / 4, rec.x, rec.width)
		}
		rl.EndMode2D()

		pos_txt := fmt.caprintf("x=%f\ny=%f", camera.target.x, camera.target.y)
		rl.DrawText(pos_txt, 20,20, 24, rl.WHITE)
		free(transmute(rawptr)pos_txt)

		rl.EndDrawing()
	}
}

game: Game
main :: proc() {
	track: trace.Tracking_Allocator
	trace.tracking_allocator_init(&track, context.allocator)
	defer trace.tracking_allocator_destroy(&track)

	context.allocator = trace.tracking_allocator(&track)
	defer trace.tracking_allocator_print_results(&track)

	context.assertion_failure_proc = trace.assertion_failure_proc

	game = game_init("res/level_fix.bin")
	defer game_deinit(game)

	rl.SetTraceLogLevel(.WARNING)
	rl.SetConfigFlags({.WINDOW_RESIZABLE, .MSAA_4X_HINT})
	rl.InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game")
	defer rl.CloseWindow()

	for !rl.WindowShouldClose() {
		game_update(&game, rl.GetFrameTime())
		game_draw(game)
	}
}
