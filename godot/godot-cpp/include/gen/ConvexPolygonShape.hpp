#ifndef GODOT_CPP_CONVEXPOLYGONSHAPE_HPP
#define GODOT_CPP_CONVEXPOLYGONSHAPE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Shape.hpp"
namespace godot {


class ConvexPolygonShape : public Shape {
	struct ___method_bindings {
		godot_method_bind *mb_get_points;
		godot_method_bind *mb_set_points;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ConvexPolygonShape"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ConvexPolygonShape *_new();

	// methods
	PoolVector3Array get_points() const;
	void set_points(const PoolVector3Array points);

};

}

#endif