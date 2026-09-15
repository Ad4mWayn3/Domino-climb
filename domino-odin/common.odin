package main

import rl "vendor:raylib"
import "core:fmt"
import "core:math/linalg"
import "core:slice"

slice_size_of :: #force_inline proc(xs: []$T) -> uintptr {
	return uintptr(len(xs) * size_of(T))
}

// reinterprets a `[]U` as a `[]T`. Asserts the byte count
// of `xs` is divisible by `size_of(T)`
slice_cast :: #force_inline proc($T: typeid, xs: []$U) -> []T {
	byte_count := slice_size_of(xs)
	assert(byte_count % size_of(T) == 0)
	return (transmute([^]T)slice.as_ptr(xs))[0:(len(xs)*size_of(U)/size_of(T))]
}

// packs a context value and a vtable of callbacks
Closure :: struct($Ctx, $VTable: typeid) {
	ctx: Ctx,
	vtable: VTable,
}

closure_make :: proc(ctx: $Ctx, vtable: $VTable) -> Closure(Ctx,VTable) {
	return {ctx,vtable}
}

VTable_Array_Readonly :: struct($Ctx, $Inner: typeid) {
	at: proc(ctx: Ctx, i: uintptr) -> Inner,
	len: proc(ctx: Ctx) -> uintptr,
}

slice_vtable_array_readonly :: proc($T: typeid
) -> VTable_Array_Readonly([]T, T) {
	return {
		at = proc(ctx: []T, i: uintptr) -> T {
			return ctx[i]
		},

		len = proc(ctx: []T) -> uintptr {
			return uintptr(len(ctx))
		},
	}
}

VTable_Array_Trait :: struct($Ctx, $Inner: typeid) {
	using readonly: VTable_Array_Readonly,
	set: proc(ctx: Ctx, i: uintptr, x: Inner),
}

VTable_Getter :: struct($Ctx, $Attrib, $T: typeid) { 
	get: proc(Ctx, T) -> Attrib
}

soa_from_aos :: proc(data: []$T) -> #soa[]T {
	out := make(#soa[]T, len(data))
	for e, i in data do out[i] = e
	return out
}

array_attrib_unzip :: proc(
	array: Closure($Ctx, VTable_Array_Readonly(Ctx, $T)),
	attrib_getter: Closure(
		$Getter_Ctx,
		VTable_Getter(Getter_Ctx, $Attrib, T),
	)
) -> []Attrib {
	out := make([]Attrib, array.vtable.len(array.ctx))

	for i in 0..<len(out) {
		out[i] = attrib_getter.vtable.get(attrib_getter.ctx,
			array.vtable.at(array.ctx, i))
	}
	
	return out
}

Range :: struct($Num: typeid) { min: Num, max: Num }
range_is_valid :: proc(range: Range($Num)) -> bool {
	//fmt.printf("min = %f, max = %f\n", range.min, range.max)
	return range.min <= range.max
}

// how much two numeric ranges intersect. Negative if no intersection is found
range_intersection_depth :: proc(a: Range(f32), b: Range(f32)) -> f32 {
	assert(range_is_valid(a) && range_is_valid(b))

	// determine the leftmost interval
	//left, right := a,b if a.min <= b.min else b,a
	left,right: Range(f32)
	if a.min <= b.min { left, right = a,b}
	else { left, right = b, a}
	return left.max - right.min
}

vec_reflect :: proc(v, axis: $V) -> V {
	assert(abs(linalg.vector_length(v)-1.) < 0.0001)
	b := axis * (2 * linalg.dot(axis,v))
	return b - v
}