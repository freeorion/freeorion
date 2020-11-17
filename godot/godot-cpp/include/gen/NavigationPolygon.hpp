#ifndef GODOT_CPP_NAVIGATIONPOLYGON_HPP
#define GODOT_CPP_NAVIGATIONPOLYGON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {


class NavigationPolygon : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_outlines;
		godot_method_bind *mb__get_polygons;
		godot_method_bind *mb__set_outlines;
		godot_method_bind *mb__set_polygons;
		godot_method_bind *mb_add_outline;
		godot_method_bind *mb_add_outline_at_index;
		godot_method_bind *mb_add_polygon;
		godot_method_bind *mb_clear_outlines;
		godot_method_bind *mb_clear_polygons;
		godot_method_bind *mb_get_outline;
		godot_method_bind *mb_get_outline_count;
		godot_method_bind *mb_get_polygon;
		godot_method_bind *mb_get_polygon_count;
		godot_method_bind *mb_get_vertices;
		godot_method_bind *mb_make_polygons_from_outlines;
		godot_method_bind *mb_remove_outline;
		godot_method_bind *mb_set_outline;
		godot_method_bind *mb_set_vertices;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "NavigationPolygon"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static NavigationPolygon *_new();

	// methods
	Array _get_outlines() const;
	Array _get_polygons() const;
	void _set_outlines(const Array outlines);
	void _set_polygons(const Array polygons);
	void add_outline(const PoolVector2Array outline);
	void add_outline_at_index(const PoolVector2Array outline, const int64_t index);
	void add_polygon(const PoolIntArray polygon);
	void clear_outlines();
	void clear_polygons();
	PoolVector2Array get_outline(const int64_t idx) const;
	int64_t get_outline_count() const;
	PoolIntArray get_polygon(const int64_t idx);
	int64_t get_polygon_count() const;
	PoolVector2Array get_vertices() const;
	void make_polygons_from_outlines();
	void remove_outline(const int64_t idx);
	void set_outline(const int64_t idx, const PoolVector2Array outline);
	void set_vertices(const PoolVector2Array vertices);

};

}

#endif