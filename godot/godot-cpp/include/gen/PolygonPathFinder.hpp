#ifndef GODOT_CPP_POLYGONPATHFINDER_HPP
#define GODOT_CPP_POLYGONPATHFINDER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {


class PolygonPathFinder : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_data;
		godot_method_bind *mb__set_data;
		godot_method_bind *mb_find_path;
		godot_method_bind *mb_get_bounds;
		godot_method_bind *mb_get_closest_point;
		godot_method_bind *mb_get_intersections;
		godot_method_bind *mb_get_point_penalty;
		godot_method_bind *mb_is_point_inside;
		godot_method_bind *mb_set_point_penalty;
		godot_method_bind *mb_setup;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PolygonPathFinder"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static PolygonPathFinder *_new();

	// methods
	Dictionary _get_data() const;
	void _set_data(const Dictionary arg0);
	PoolVector2Array find_path(const Vector2 from, const Vector2 to);
	Rect2 get_bounds() const;
	Vector2 get_closest_point(const Vector2 point) const;
	PoolVector2Array get_intersections(const Vector2 from, const Vector2 to) const;
	real_t get_point_penalty(const int64_t idx) const;
	bool is_point_inside(const Vector2 point) const;
	void set_point_penalty(const int64_t idx, const real_t penalty);
	void setup(const PoolVector2Array points, const PoolIntArray connections);

};

}

#endif