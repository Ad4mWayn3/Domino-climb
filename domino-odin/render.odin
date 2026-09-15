package main

import rl "vendor:raylib"

SCREEN_WIDTH :: 1202
SCREEN_HEIGHT :: 600
SCREEN :: [2]f32{SCREEN_WIDTH, SCREEN_HEIGHT}

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

// WARNING: this procedure only works if the pair `{width,height}` was
// transformed as a point in screen space (which means it very likely has at
// least one negative coordinate)
_rec_screen_space :: proc(screen: [2]$Num, rec: rl.Rectangle
) -> rl.Rectangle {
	x := screen_space([?]f32{rec.x,rec.y,rec.width,rec.height}, screen)
	return {x=x[0], y=x[1], width=x[2], height=x[3]}
}