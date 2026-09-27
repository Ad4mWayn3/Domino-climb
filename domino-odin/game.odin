package main

import "core:fmt"
import "core:math"
import "core:math/linalg"
import "core:mem"
import "core:os"
import "core:slice"

import rl "vendor:raylib"

PLAYER_WIDTH :: 1./12. + CLIP_SPACE_PIXEL_SIZE_X
PLAYER_HEIGHT :: PLAYER_WIDTH * 2. + CLIP_SPACE_PIXEL_SIZE_Y
PLAYER_JUMP_IMPULSE :: 1.8
PLAYER_HORIZONTAL_ACCEL :: 2.

Player_Flag :: enum {
	Turned,
	Wants_To_Turn,
	On_Ground,
	On_Wall,
	Collided,
}

Player :: struct {
	origin: [2]f32,
	size: [2]f32,
	velocity: [2]f32,
	flags: bit_set[Player_Flag],
}

WORLD_GRAVITY :: 5
STOPWATCH_INTERVAL :: 0.1

Game :: struct {
	player: Player,
	speed_o_meter: [2][2]f32,
	stopwatch: f32,
	static_props: []rl.Rectangle,
	action_buffer: Action_Buffer,
	camera: rl.Camera2D,
}

rec_intersects_recs :: proc(rec: rl.Rectangle,
	recs: Closure($Ctx, VTable_Array_Readonly(Ctx, rl.Rectangle)),
) -> (collision: bool, idx: int) {
	rec_count := recs.vtable.len(recs.ctx)
	for i in 0..<rec_count {
		if rl.CheckCollisionRecs(rec, recs.vtable.at(recs.ctx, i)) {
			return true, i
		}
	}

	return false, 0
}

// whether a rectangle's major side is the horizontal one. False if its a square
rec_is_horizontal :: proc "contextless" (rec: rl.Rectangle) -> bool {
	return rec.width > rec.height
}

player_convex_set_vtable :: VTable_Convex_Set(^Player) {
	support_point = proc(player: ^Player, d: [2]f32) -> [2]f32 {
		r := player_aabb(player^)
		return rec_convex_set_vtable.support_point(&r, d)
	},

	translate = proc(player: ^Player, p: [2]f32) {
		player.origin += p
	},

	origin = proc(player: ^Player) -> [2]f32 {
		return player.origin
	},
}

player_aabb :: proc(player: Player) -> rl.Rectangle {
	// Turns the bounding-box 90 degrees around the geometric center
	if .Turned in player.flags {
		shift := (player.size.y - player.size.x) / 2
		return { x = player.origin.x - shift,
			y = player.origin.y + shift,
			width = player.size.y,
			height = player.size.x,
		}
	}
	return { x = player.origin.x, y = player.origin.y,
		width = player.size.x, height = player.size.y }
}

player_move :: proc(player: ^Player, world: []rl.Rectangle, delta: f32) {
	if .Wants_To_Turn in player.flags {
		player.flags ~= {.Turned}
		c, _ := world_collision_check(world, player_aabb(player^))
		if c.found do player.flags ~= {.Turned}
	}
	move := player.velocity * delta

	Check_Ctx :: struct { recs: []rl.Rectangle, i: ^uintptr }
	check_wrap :: proc(ctx: Check_Ctx, p: ^Player) -> Collision {
		collision, i := world_collision_check(ctx.recs, player_aabb(p^))
		ctx.i^ = i
		if collision.found do fmt.printf("game::player_move::check_wrap:\n"+
			"collision index: %d\n"+
			"what i wrote: %d\n", i, ctx.i^)
		return collision
	}

	object_idx: uintptr = 0
	collision := convex_set_move_and_collide(
		closure_make(player, player_convex_set_vtable),
		closure_make(Check_Ctx{world, &object_idx}, check_wrap),
		move)

	//fmt.printfln("")
	fmt.printf("game::player_move:\n"+
		"collision = %v\n"+
		"dot: %.4f\n"+
		"idx: %d\n\n",
		collision, linalg.dot(move,collision.axis), object_idx)

	for collision.found && linalg.dot(move, collision.axis) <= 0. {
		fmt.printf("game::player_move: collided with object at index=%d\n",
			object_idx)
		if object_idx == cast(uintptr)len(world) do return

		reflected := linalg.reflect(player.velocity, collision.axis)
		proj, rej := (player.velocity + reflected) / 2.,
			(reflected - player.velocity) / 2.
		player.velocity = proj

		// p = player.origin + player.velocity
		//fmt.printf("went: x = %.4f, y = %.4f\n", p.x, p.y)

		move = player.velocity * delta

		new_idx: uintptr = 0
		collision = convex_set_move_and_collide(
			closure_make(player, player_convex_set_vtable),
			closure_make(Check_Ctx{world[object_idx+1:], &new_idx}, check_wrap),
			move)
		object_idx += new_idx+1
	}

	//player.origin += player.velocity * delta
}

game_init :: proc(level_subpath: string) -> (game: Game) {
	game.player = {
		origin = {0,0},
		size = {PLAYER_WIDTH, PLAYER_HEIGHT},
		velocity = {0,0},
		flags = {},
	}

	game.speed_o_meter = {{0,0},{0,0}}
	game.stopwatch = 0.

	{
		file, err := os.open(level_subpath, {.Read})
		ensure(err == os.ERROR_NONE)
		defer os.close(file)
		size, _ := os.file_size(file)
		
		assert(size % size_of(rl.Rectangle) == 0)
		game.static_props = make([]rl.Rectangle, size/size_of(rl.Rectangle))

		os.read_slice(file, game.static_props)
	}

	game.camera = {
		target = {0,0},
		offset = {0,0},
		rotation = 0,
		zoom = 1,
	}

	game.action_buffer = {
		current = {},
		previous = {},
	}

	return
}

game_deinit :: proc(game: ^Game) {
	delete(game.static_props)
}

game_update :: proc(game: ^Game, delta: f32) {
	game.stopwatch += delta
	cycled := false
	if game.stopwatch > STOPWATCH_INTERVAL {
		game.stopwatch -= STOPWATCH_INTERVAL
		game.speed_o_meter[0] = game.speed_o_meter[1]
		game.speed_o_meter[1] = game.player.origin
	}

	action_buffer_refresh(&game.action_buffer)
	game.camera.offset = {cast(f32)rl.GetScreenWidth(), cast(f32)rl.GetScreenHeight()}/2.
	defer game.camera.target = screen_space(game.player.origin, vec_screen_size())

	if input_get_action_state(game.action_buffer, .Jump).timed == .Pressed {
		game.player.velocity.y -= PLAYER_JUMP_IMPULSE
	} if input_get_action_state(game.action_buffer, .Left).present == .Down {
		game.player.velocity.x -= PLAYER_HORIZONTAL_ACCEL * delta
	} if input_get_action_state(game.action_buffer, .Right).present == .Down {
		game.player.velocity.x += PLAYER_HORIZONTAL_ACCEL * delta
	} if input_get_action_state(game.action_buffer, .Turn).timed == .Pressed {
		game.player.flags += {.Wants_To_Turn}
	}

	game.player.velocity.y += WORLD_GRAVITY * delta
	player_move(&game.player, game.static_props, delta)
	game.player.flags -= {.Wants_To_Turn}
}

game_draw :: proc(game: Game) {
	rl.BeginDrawing()
	rl.ClearBackground({0,20,20,0xff})
	rl.BeginMode2D(game.camera)

	for prop in game.static_props {
		rl.DrawRectangleRec(rec_screen_space(vec_screen_size(), prop), rl.BLUE)
	}

	rl.DrawRectangleRec(rec_screen_space(vec_screen_size(), player_aabb(game.player)), rl.RAYWHITE)

	rl.EndMode2D()

	vel := game.speed_o_meter[1] - game.speed_o_meter[0]
	vel /= STOPWATCH_INTERVAL
	text := fmt.caprintf("vel x = %.4f\tactual = %.4f\n"+
		"vel y = %.4f\tactual = %.9f",
		vel.x, game.player.velocity.x,
		vel.y, game.player.velocity.y)
	defer delete(text)
	rl.DrawText(text,30,30, fontSize=20, color=rl.RAYWHITE)
	rl.DrawFPS(30,70)
	rl.EndDrawing()
}