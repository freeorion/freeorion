#ifndef GODOT_CPP_NAVIGATION2D_HPP
#define GODOT_CPP_NAVIGATION2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {

class Object;
class NavigationPolygon;

class Navigation2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_closest_point;
		godot_method_bind *mb_get_closest_point_owner;
		godot_method_bind *mb_get_simple_path;
		godot_method_bind *mb_navpoly_add;
		godot_method_bind *mb_navpoly_remove;
		godot_method_bind *mb_navpoly_set_transform;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Navigation2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Navigation2D *_new();

	// methods
	Vector2 get_closest_point(const Vector2 to_point);
	Object *get_closest_point_owner(const Vector2 to_point);
	PoolVector2Array get_simple_path(const Vector2 start, const Vector2 end, const bool optimize = true);
	int64_t navpoly_add(const Ref<NavigationPolygon> mesh, const Transform2D xform, const Object *owner = nullptr);
	void navpoly_remove(const int64_t id);
	void navpoly_set_transform(const int64_t id, const Transform2D xform);

};

}

#endif