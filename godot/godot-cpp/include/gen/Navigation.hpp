#ifndef GODOT_CPP_NAVIGATION_HPP
#define GODOT_CPP_NAVIGATION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {

class Object;
class NavigationMesh;

class Navigation : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_get_closest_point;
		godot_method_bind *mb_get_closest_point_normal;
		godot_method_bind *mb_get_closest_point_owner;
		godot_method_bind *mb_get_closest_point_to_segment;
		godot_method_bind *mb_get_simple_path;
		godot_method_bind *mb_get_up_vector;
		godot_method_bind *mb_navmesh_add;
		godot_method_bind *mb_navmesh_remove;
		godot_method_bind *mb_navmesh_set_transform;
		godot_method_bind *mb_set_up_vector;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Navigation"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Navigation *_new();

	// methods
	Vector3 get_closest_point(const Vector3 to_point);
	Vector3 get_closest_point_normal(const Vector3 to_point);
	Object *get_closest_point_owner(const Vector3 to_point);
	Vector3 get_closest_point_to_segment(const Vector3 start, const Vector3 end, const bool use_collision = false);
	PoolVector3Array get_simple_path(const Vector3 start, const Vector3 end, const bool optimize = true);
	Vector3 get_up_vector() const;
	int64_t navmesh_add(const Ref<NavigationMesh> mesh, const Transform xform, const Object *owner = nullptr);
	void navmesh_remove(const int64_t id);
	void navmesh_set_transform(const int64_t id, const Transform xform);
	void set_up_vector(const Vector3 up);

};

}

#endif