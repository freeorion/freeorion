#ifndef GODOT_CPP_COLLISIONSHAPE_HPP
#define GODOT_CPP_COLLISIONSHAPE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {

class Shape;
class Resource;

class CollisionShape : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb__shape_changed;
		godot_method_bind *mb__update_debug_shape;
		godot_method_bind *mb_get_shape;
		godot_method_bind *mb_is_disabled;
		godot_method_bind *mb_make_convex_from_brothers;
		godot_method_bind *mb_resource_changed;
		godot_method_bind *mb_set_disabled;
		godot_method_bind *mb_set_shape;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CollisionShape"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CollisionShape *_new();

	// methods
	void _shape_changed();
	void _update_debug_shape();
	Ref<Shape> get_shape() const;
	bool is_disabled() const;
	void make_convex_from_brothers();
	void resource_changed(const Ref<Resource> resource);
	void set_disabled(const bool enable);
	void set_shape(const Ref<Shape> shape);

};

}

#endif