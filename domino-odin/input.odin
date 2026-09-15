package main

import rl "vendor:raylib"

Action :: distinct enum {
	Up, Down, Left, Right,
	Jump, Turn,
}

// whether the action is currently active
Present_Action_State :: distinct enum { Up, Down }

// timed action transitions
Timed_Action_State :: distinct enum {
	Held, Unheld, // continued action state
	Pressed, Released, // transition action state
}

Input :: union {
	rl.KeyboardKey,
	rl.MouseButton,
	rl.GamepadButton,
}

DEFAULT_INPUT_MAP :: [Action][]Input {
	.Up = { rl.KeyboardKey.E },
	.Down = { rl.KeyboardKey.D },
	.Left = { rl.KeyboardKey.S },
	.Right = { rl.KeyboardKey.F },
	.Jump = { rl.KeyboardKey.SPACE, rl.KeyboardKey.J },
	.Turn = { rl.KeyboardKey.K },
}

// contains a set of the currently and previously triggered actions
Action_Buffer :: struct {
	current: bit_set[Action],
	previous: bit_set[Action],
}

input_query :: proc(input: Input) -> (present: Present_Action_State,
	timed: Timed_Action_State
) {
	switch i in input {
	case rl.KeyboardKey:
		present = .Down if rl.IsKeyDown(i) else .Up
		timed = .Pressed if rl.IsKeyPressed(i) else
			.Held if rl.IsKeyDown(i) else
			.Released if rl.IsKeyReleased(i) else .Unheld
	case rl.MouseButton:
		present = .Down if rl.IsMouseButtonDown(i) else .Up
		timed = .Pressed if rl.IsMouseButtonPressed(i) else
			.Held if rl.IsMouseButtonDown(i) else
			.Released if rl.IsMouseButtonReleased(i) else .Unheld
	case rl.GamepadButton:
		present = .Down if rl.IsGamepadButtonDown(0, i) else .Up
		timed = .Pressed if rl.IsGamepadButtonPressed(0, i) else
			.Held if rl.IsGamepadButtonDown(0, i) else
			.Released if rl.IsGamepadButtonReleased(0, i) else .Unheld
	}
	return
}

action_buffer_refresh :: proc(buffer: ^Action_Buffer,
	input_map: [Action][]Input = DEFAULT_INPUT_MAP
) {
	buffer.previous = buffer.current
	buffer.current = {}

	for action in Action {
		inputs := input_map[action]
		lbl: for input in inputs {
			switch i in input {
			case rl.KeyboardKey:
				if rl.IsKeyDown(i) {
					buffer.current += {action}
					break lbl
				}
			case rl.MouseButton:
				if rl.IsMouseButtonDown(i) {
					buffer.current += {action}
					break lbl
				}
			case rl.GamepadButton:
				if rl.IsGamepadButtonDown(0,i) {
					buffer.current += {action}
					break lbl
				}
			}
		}
	}
}

input_get_action_state :: proc(buffer: Action_Buffer, action: Action
) -> struct {present: Present_Action_State, timed: Timed_Action_State} {
	if action in buffer.current {
		return {.Down, .Held if action in buffer.previous else .Pressed}
	} else {
		return {.Up, .Released if action in buffer.previous else .Unheld}
	}
}