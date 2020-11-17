#include "Navigation.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "NavigationMesh.hpp"


namespace godot {


Navigation::___method_bindings Navigation::___mb = {};

void Navigation::___init_method_bindings() {
	___mb.mb_get_closest_point = godot::api->godot_method_bind_get_method("Navigation", "get_closest_point");
	___mb.mb_get_closest_point_normal = godot::api->godot_method_bind_get_method("Navigation", "get_closest_point_normal");
	___mb.mb_get_closest_point_owner = godot::api->godot_method_bind_get_method("Navigation", "get_closest_point_owner");
	___mb.mb_get_closest_point_to_segment = godot::api->godot_method_bind_get_method("Navigation", "get_closest_point_to_segment");
	___mb.mb_get_simple_path = godot::api->godot_method_bind_get_method("Navigation", "get_simple_path");
	___mb.mb_get_up_vector = godot::api->godot_method_bind_get_method("Navigation", "get_up_vector");
	___mb.mb_navmesh_add = godot::api->godot_method_bind_get_method("Navigation", "navmesh_add");
	___mb.mb_navmesh_remove = godot::api->godot_method_bind_get_method("Navigation", "navmesh_remove");
	___mb.mb_navmesh_set_transform = godot::api->godot_method_bind_get_method("Navigation", "navmesh_set_transform");
	___mb.mb_set_up_vector = godot::api->godot_method_bind_get_method("Navigation", "set_up_vector");
}

Navigation *Navigation::_new()
{
	return (Navigation *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Navigation")());
}
Vector3 Navigation::get_closest_point(const Vector3 to_point) {
	return ___godot_icall_Vector3_Vector3(___mb.mb_get_closest_point, (const Object *) this, to_point);
}

Vector3 Navigation::get_closest_point_normal(const Vector3 to_point) {
	return ___godot_icall_Vector3_Vector3(___mb.mb_get_closest_point_normal, (const Object *) this, to_point);
}

Object *Navigation::get_closest_point_owner(const Vector3 to_point) {
	return (Object *) ___godot_icall_Object_Vector3(___mb.mb_get_closest_point_owner, (const Object *) this, to_point);
}

Vector3 Navigation::get_closest_point_to_segment(const Vector3 start, const Vector3 end, const bool use_collision) {
	return ___godot_icall_Vector3_Vector3_Vector3_bool(___mb.mb_get_closest_point_to_segment, (const Object *) this, start, end, use_collision);
}

PoolVector3Array Navigation::get_simple_path(const Vector3 start, const Vector3 end, const bool optimize) {
	return ___godot_icall_PoolVector3Array_Vector3_Vector3_bool(___mb.mb_get_simple_path, (const Object *) this, start, end, optimize);
}

Vector3 Navigation::get_up_vector() const {
	return ___godot_icall_Vector3(___mb.mb_get_up_vector, (const Object *) this);
}

int64_t Navigation::navmesh_add(const Ref<NavigationMesh> mesh, const Transform xform, const Object *owner) {
	return ___godot_icall_int_Object_Transform_Object(___mb.mb_navmesh_add, (const Object *) this, mesh.ptr(), xform, owner);
}

void Navigation::navmesh_remove(const int64_t id) {
	___godot_icall_void_int(___mb.mb_navmesh_remove, (const Object *) this, id);
}

void Navigation::navmesh_set_transform(const int64_t id, const Transform xform) {
	___godot_icall_void_int_Transform(___mb.mb_navmesh_set_transform, (const Object *) this, id, xform);
}

void Navigation::set_up_vector(const Vector3 up) {
	___godot_icall_void_Vector3(___mb.mb_set_up_vector, (const Object *) this, up);
}

}