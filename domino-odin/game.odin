package main

import "core:fmt"
import "core:math/linalg"
import "core:mem"
import "core:os"
import "core:slice"

import rl "vendor:raylib"

Player_Flag :: enum {
	Turned,
	On_Ground,
	On_Wall,
}

Player :: struct {
	origin: [2]f32,
	size: [2]f32,
	velocity: [2]f32,
	flags: bit_set[Player_Flag],
}

PLAYER_WIDTH :: 1./20.
PLAYER_HEIGHT :: PLAYER_WIDTH * 2.
PLAYER_JUMP_IMPULSE :: 0.7
PLAYER_HORIZONTAL_ACCEL :: 0.14

Game :: struct {
	player: Player,
	static_props: []rl.Rectangle,
	action_buffer: Action_Buffer,
	camera: rl.Camera2D,
}

WORLD_GRAVITY :: 0.23

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

player_move :: proc(player: ^Player, world: []rl.Rectangle, delta: f32) {
	//assert(delta > 0)
	move := player.velocity * delta

	check_wrap :: proc(r: []rl.Rectangle, p: ^Player) -> Collision {
		return world_collision_check(r, player_aabb(p^))
	}
	collision := convex_set_move_and_collide(
		closure_make(player, player_convex_set_vtable),
		closure_make(world, check_wrap),
		move)
	if !collision.found do return

	// // if a collision was found, the motion vector should have an angle > 90deg.
	// assert(linalg.dot(collision.axis, move) < 0 \
	// 	// axis is expected to be normalized
	// 	&& abs(linalg.vector_length(collision.axis) - 1.) < 0.0001)
	{
		fmt.printf("player_move:\n"+
			"dot: %f\n"+
			"axis length: %f\n",
			linalg.dot(collision.axis, move),
			linalg.vector_length(collision.axis))
	}

	player.velocity = vec_reflect(player.velocity, collision.axis)
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

game_init :: proc(level_subpath: string) -> (game: Game) {
	game.player = {
		origin = {0,0},
		size = {PLAYER_WIDTH, PLAYER_HEIGHT},
		velocity = {0,0},
		flags = {},
	}

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
		game.player.flags += {.Turned}
	}

	game.player.velocity.y += WORLD_GRAVITY * delta
	player_move(&game.player, game.static_props, delta)
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
	rl.EndDrawing()
}