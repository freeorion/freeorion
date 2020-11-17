#ifndef GODOT_CPP_PATHFOLLOW2D_HPP
#define GODOT_CPP_PATHFOLLOW2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {


class PathFollow2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_cubic_interpolation;
		godot_method_bind *mb_get_h_offset;
		godot_method_bind *mb_get_lookahead;
		godot_method_bind *mb_get_offset;
		godot_method_bind *mb_get_unit_offset;
		godot_method_bind *mb_get_v_offset;
		godot_method_bind *mb_has_loop;
		godot_method_bind *mb_is_rotating;
		godot_method_bind *mb_set_cubic_interpolation;
		godot_method_bind *mb_set_h_offset;
		godot_method_bind *mb_set_lookahead;
		godot_method_bind *mb_set_loop;
		godot_method_bind *mb_set_offset;
		godot_method_bind *mb_set_rotate;
		godot_method_bind *mb_set_unit_offset;
		godot_method_bind *mb_set_v_offset;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PathFollow2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static PathFollow2D *_new();

	// methods
	bool get_cubic_interpolation() const;
	real_t get_h_offset() const;
	real_t get_lookahead() const;
	real_t get_offset() const;
	real_t get_unit_offset() const;
	real_t get_v_offset() const;
	bool has_loop() const;
	bool is_rotating() const;
	void set_cubic_interpolation(const bool enable);
	void set_h_offset(const real_t h_offset);
	void set_lookahead(const real_t lookahead);
	void set_loop(const bool loop);
	void set_offset(const real_t offset);
	void set_rotate(const bool enable);
	void set_unit_offset(const real_t unit_offset);
	void set_v_offset(const real_t v_offset);

};

}

#endif