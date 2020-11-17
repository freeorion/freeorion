#ifndef GODOT_CPP_COLLISIONPOLYGON_HPP
#define GODOT_CPP_COLLISIONPOLYGON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {


class CollisionPolygon : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb__is_editable_3d_polygon;
		godot_method_bind *mb_get_depth;
		godot_method_bind *mb_get_polygon;
		godot_method_bind *mb_is_disabled;
		godot_method_bind *mb_set_depth;
		godot_method_bind *mb_set_disabled;
		godot_method_bind *mb_set_polygon;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CollisionPolygon"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CollisionPolygon *_new();

	// methods
	bool _is_editable_3d_polygon() const;
	real_t get_depth() const;
	PoolVector2Array get_polygon() const;
	bool is_disabled() const;
	void set_depth(const real_t depth);
	void set_disabled(const bool disabled);
	void set_polygon(const PoolVector2Array polygon);

};

}

#endif