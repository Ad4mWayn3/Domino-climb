package main

import rl "vendor:raylib"
import "core:fmt"
import "core:math/linalg"

vector2_wedge :: proc(v: [2]f32, u: [2]f32) -> f32 {
	return v.x*u.y - v.y*u.x
}

VTable_Convex_Set :: struct($Ctx: typeid) {
	// returns the furthest boundary point in the direction `d`.
	// Assumes `d` is normalized
	support_point: proc(ctx: Ctx, d: [2]f32) -> [2]f32,

	// mutates the internal `ctx` so that its translated according to `p`
	translate: proc(ctx: Ctx, p: [2]f32),
	origin: proc(ctx: Ctx) -> [2]f32,
}

// gets the normal direcion of the boundary in the direction implied by `p`.
convex_set_support_point_from_point :: proc(
	convex_set: Closure($Ctx, VTable_Convex_Set(Ctx)),
	p: [2]f32,
) -> [2]f32 {
	p_mag := sqrt(p.x*p.x,p.y*p.y)
	return convex_set.vtable.support_point(convex_set.ctx, p/p_mag)
}

// gets the normal direction of the boundary in the angle `theta` in radians.
// `theta` = 0 is equivalent to the direction {x=1, y=0}
convex_set_support_point_from_angle :: proc(
	convex_set: Closure($Ctx, VTable_Convex_Set(Ctx)),
	theta: f32,
) -> [2]f32 {
	return convex_set.vtable.support_point(convex_set.ctx, {cos(theta),sin(theta)})
}

Collision :: struct {
	// whether an intersection was found
	found: bool,

	// if there is a collision, `axis` is a direction in which `depth`
	// is reasonably small. should be oriented so that it "shifts" the body
	// `a` out of `b`
	axis: [2]f32,

	// furthest/supporting point of shape A in the opposite direction of `axis`.
	support_point_a: [2]f32,

	// furthest/supporting point of shape B in the direction of `axis`
	support_point_b: [2]f32,
}

COLLISION_NONE :: Collision {
	found = false,
	axis = {0,0},
	support_point_a = {0,0},
	support_point_b = {0,0}
}

EPSILON :: 1./64.
convex_set_move_and_collide :: proc(
	a: Closure($A, VTable_Convex_Set(A)),
	collision_checker: Closure($Ctx, proc(ctx: Ctx, a: A) -> Collision),
	move: [2]f32,
) -> Collision {
	a.vtable.translate(a.ctx, move)

	collision_test := collision_checker.vtable(collision_checker.ctx, a.ctx)
	if !collision_test.found do return COLLISION_NONE

	step := -move / 2.
	mag := linalg.vector_length(step)
	collision := collision_test
	a.vtable.translate(a.ctx, step)

	// interpolate along `move`
	for mag > EPSILON {
		step /= 2.; mag /= 2.
		collision_test = collision_checker.vtable(collision_checker.ctx, a.ctx)
		if collision_test.found {
			collision = collision_test
			a.vtable.translate(a.ctx, step)
		} else do a.vtable.translate(a.ctx, -step)
	}

	assert(collision.found)
	// the normal should be pointing outward from the static body
	assert(a.vtable.support_point(a.ctx, linalg.normalize(-collision.axis)) \
		== collision.support_point_a)
	return collision
}

rec_convex_set_vtable :: VTable_Convex_Set(^rl.Rectangle) {
	support_point = proc(rec: ^rl.Rectangle, d: [2]f32) -> [2]f32 {
		points := [4][2]f32{
			{rec.x,rec.y},
			{rec.x+rec.width,rec.y},
			{rec.x,rec.y+rec.height},
			{rec.x+rec.width,rec.y+rec.height}
		}

		max_arg := 0
		max_dot := linalg.dot(d,points[0])
		for i in 1..<len(points) {
			curr := linalg.dot(d,points[i])
			if curr > max_dot {
				max_dot = curr
				max_arg = i
			}
		}

		return points[max_arg]
	},

	translate = proc(rec: ^rl.Rectangle, p: [2]f32) {
		rec^.x += p.x
		rec^.y += p.y
	},

	origin = proc(rec: ^rl.Rectangle) -> [2]f32 {
		return {rec.x + rec.width/2., rec.y + rec.height/2.}
	}
}

rec_collision_checker_rec :: proc(a: rl.Rectangle, b: rl.Rectangle
) -> (collision: Collision) {
	//fmt.printf("rec = %v\n", a)
	a_min, b_min := [2]f32{a.x, a.y}, [2]f32{b.x, b.y}
	a_max, b_max := a_min+{a.width,a.height}, b_min+{b.width,b.height}

	//check collision depth on each axis
	depth := [2]f32{
		range_intersection_depth({a_min.x,a_max.x},{b_min.x,b_max.x}),
		range_intersection_depth({a_min.y,a_max.y},{b_min.y,b_max.y})
	}
	if depth.x < 0 || depth.y < 0 do return COLLISION_NONE

	collision.found = true

	switch min(depth.x,depth.y) {
	case depth.x:
		collision.axis = {-depth.x, 0}
		// by default, the axis points left or upwards, assumming the
		// upper/leftmost interval is to be pushed out of the lower/rightmost--
		// but because the axis is supposed to convey "pushing" `b` out of `a`,
		// we check if b lies lower/rightmost in the smaller axis, and flip
		// the axis accordingly
		if a_min.x < b_min.x do collision.axis *= -1.
	case depth.y: collision.axis = {0, -depth.y}
		if a_min.y < b_min.y do collision.axis *= -1.
	}

	b := b
	a := a
	d := linalg.normalize(collision.axis)

	collision.support_point_b = rec_convex_set_vtable.support_point(&b, d)
	collision.support_point_a = rec_convex_set_vtable.support_point(&a, -d)

	return
}

world_collision_check :: proc(world: []rl.Rectangle, obj: rl.Rectangle
) -> (collision: Collision) {
	for shape in world {
		collision_test := rec_collision_checker_rec(shape, obj)
		if collision_test.found {
			t := obj
			t.x += collision_test.axis.x
			t.y += collision_test.axis.y

			// the collision should be resolved statically
			assert(rec_collision_checker_rec(shape, t).found == false)
			return collision_test
		}
	}

	return COLLISION_NONE
}