package main

import "core:fmt"
import rl "vendor:raylib"
import rlgl "vendor:raylib/rlgl"

vertex :: `
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
out vec3 vColor;

void main() {
	gl_Position = vec4(aPos, 0., 1.);
	vColor = aColor;
}
`

fragment :: `
#version 330 core
in vec3 vColor;
out vec4 fColor;

void main() {
	fColor = vec4(vColor, 1.);
}
`

positions := [?]f32{
	-.3, -.3,
	.3, -.3,
	0., .5,
}

colors := [?]u8{
	0xff,0x00,0x00,
	0x00,0xff,0x00,
	0x00,0x00,0xff,
}

Mesh2D :: struct {
	positions: [][2]f32,
	colors: Maybe([][3]u8),
}

demo :: proc() {
	rl.InitWindow(1200,600,"hello odin!")
	defer rl.CloseWindow()

	program := rl.LoadShaderFromMemory(vertex, fragment)
	defer rl.UnloadShader(program)

	vao := rlgl.LoadVertexArray()
	rlgl.EnableVertexArray(vao)
	defer rlgl.UnloadVertexArray(vao)

	posBuffer := rlgl.LoadVertexBuffer(&positions, len(positions)*size_of(f32), false)
	rlgl.SetVertexAttribute(0, 2, rlgl.FLOAT, false, 0,0)
	rlgl.EnableVertexAttribute(0)

	colorBuffer := rlgl.LoadVertexBuffer(&colors, len(colors)*size_of(u8), false)
	rlgl.SetVertexAttribute(1, 3, rlgl.UNSIGNED_BYTE, true, 0,0)
	rlgl.EnableVertexAttribute(1)

	defer rlgl.UnloadVertexBuffer(posBuffer)
	defer rlgl.UnloadVertexBuffer(colorBuffer)

	for !rl.WindowShouldClose() {
		rl.BeginDrawing()

		rl.ClearBackground(rl.BLACK)
		rl.DrawFPS(30,30)
		rl.DrawRectangleRec(rl.Rectangle{x=80,y=80,width=50,height=40}, rl.WHITE)

		rlgl.EnableVertexArray(vao)
		rlgl.EnableShader(program.id)
		rlgl.DrawVertexArray(0,6)
		rlgl.EnableVertexArray(0)

		rl.EndDrawing()
	}
}
