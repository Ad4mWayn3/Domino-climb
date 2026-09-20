package main

import rl "vendor:raylib"

SCREEN_WIDTH :: 1200
SCREEN_HEIGHT :: 600
SCREEN :: [2]f32{SCREEN_WIDTH, SCREEN_HEIGHT}

CLIP_SPACE_PIXEL_SIZE_X :: 1./(2.*SCREEN_WIDTH)
CLIP_SPACE_PIXEL_SIZE_Y :: 1./(2.*SCREEN_HEIGHT)

vec_screen_size :: proc() -> [2]f32 {
	return {cast(f32)rl.GetScreenWidth(), cast(f32)rl.GetScreenHeight()}
}

screen_space :: proc(point: $Num, screen: [2]$Num2) -> Num {
	return (point + 1)/2 * min(screen.x, screen.y)
}

screen_space_linear :: proc(point: $Num, screen: [2]$Num2) -> Num {
	return point * min(screen.x, screen.y) / 2.
}

clip_space :: proc(point: $Num, screen: [2]$Num2) -> Num {
	return point / min(screen.x, screen.y) * 2 - 1
}

clip_space_linear :: proc(point: $Num, screen: [2]$Num2) -> Num {
	return point / min(screen.x, screen.y) * 2.
}

rec_clip_space :: proc(screen: [2]$Num, rec: rl.Rectangle) -> rl.Rectangle {
	p := clip_space([?]f32{rec.x,rec.y}, screen)
	s := clip_space_linear([?]f32{rec.width,rec.height}, screen)
	return {x = p.x, y = p.y, width = s.x, height = s.y}
}

rec_screen_space :: proc(screen: [2]$Num, rec: rl.Rectangle) -> rl.Rectangle {
	p := screen_space([?]f32{rec.x,rec.y}, screen)
	s := screen_space_linear([?]f32{rec.width,rec.height}, screen)
	return {x = p.x, y = p.y, width = s.x, height = s.y}
}