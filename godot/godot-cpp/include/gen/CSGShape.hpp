#ifndef GODOT_CPP_CSGSHAPE_HPP
#define GODOT_CPP_CSGSHAPE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "CSGShape.hpp"

#include "GeometryInstance.hpp"
namespace godot {


class CSGShape : public GeometryInstance {
	struct ___method_bindings {
		godot_method_bind *mb__update_shape;
		godot_method_bind *mb_get_collision_layer;
		godot_method_bind *mb_get_collision_layer_bit;
		godot_method_bind *mb_get_collision_mask;
		godot_method_bind *mb_get_collision_mask_bit;
		godot_method_bind *mb_get_meshes;
		godot_method_bind *mb_get_operation;
		godot_method_bind *mb_get_snap;
		godot_method_bind *mb_is_calculating_tangents;
		godot_method_bind *mb_is_root_shape;
		godot_method_bind *mb_is_using_collision;
		godot_method_bind *mb_set_calculate_tangents;
		godot_method_bind *mb_set_collision_layer;
		godot_method_bind *mb_set_collision_layer_bit;
		godot_method_bind *mb_set_collision_mask;
		godot_method_bind *mb_set_collision_mask_bit;
		godot_method_bind *mb_set_operation;
		godot_method_bind *mb_set_snap;
		godot_method_bind *mb_set_use_collision;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CSGShape"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Operation {
		OPERATION_UNION = 0,
		OPERATION_INTERSECTION = 1,
		OPERATION_SUBTRACTION = 2,
	};

	// constants

	// methods
	void _update_shape();
	int64_t get_collision_layer() const;
	bool get_collision_layer_bit(const int64_t bit) const;
	int64_t get_collision_mask() const;
	bool get_collision_mask_bit(const int64_t bit) const;
	Array get_meshes() const;
	CSGShape::Operation get_operation() const;
	real_t get_snap() const;
	bool is_calculating_tangents() const;
	bool is_root_shape() const;
	bool is_using_collision() const;
	void set_calculate_tangents(const bool enabled);
	void set_collision_layer(const int64_t layer);
	void set_collision_layer_bit(const int64_t bit, const bool value);
	void set_collision_mask(const int64_t mask);
	void set_collision_mask_bit(const int64_t bit, const bool value);
	void set_operation(const int64_t operation);
	void set_snap(const real_t snap);
	void set_use_collision(const bool operation);

};

}

#endif