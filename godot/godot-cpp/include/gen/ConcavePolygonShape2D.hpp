#ifndef GODOT_CPP_CONCAVEPOLYGONSHAPE2D_HPP
#define GODOT_CPP_CONCAVEPOLYGONSHAPE2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Shape2D.hpp"
namespace godot {


class ConcavePolygonShape2D : public Shape2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_segments;
		godot_method_bind *mb_set_segments;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ConcavePolygonShape2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ConcavePolygonShape2D *_new();

	// methods
	PoolVector2Array get_segments() const;
	void set_segments(const PoolVector2Array segments);

};

}

#endif