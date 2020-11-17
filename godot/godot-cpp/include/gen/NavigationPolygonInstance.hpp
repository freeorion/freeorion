#ifndef GODOT_CPP_NAVIGATIONPOLYGONINSTANCE_HPP
#define GODOT_CPP_NAVIGATIONPOLYGONINSTANCE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {

class NavigationPolygon;

class NavigationPolygonInstance : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__navpoly_changed;
		godot_method_bind *mb_get_navigation_polygon;
		godot_method_bind *mb_is_enabled;
		godot_method_bind *mb_set_enabled;
		godot_method_bind *mb_set_navigation_polygon;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "NavigationPolygonInstance"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static NavigationPolygonInstance *_new();

	// methods
	void _navpoly_changed();
	Ref<NavigationPolygon> get_navigation_polygon() const;
	bool is_enabled() const;
	void set_enabled(const bool enabled);
	void set_navigation_polygon(const Ref<NavigationPolygon> navpoly);

};

}

#endif