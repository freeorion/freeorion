#include "Navigation2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "NavigationPolygon.hpp"


namespace godot {


Navigation2D::___method_bindings Navigation2D::___mb = {};

void Navigation2D::___init_method_bindings() {
	___mb.mb_get_closest_point = godot::api->godot_method_bind_get_method("Navigation2D", "get_closest_point");
	___mb.mb_get_closest_point_owner = godot::api->godot_method_bind_get_method("Navigation2D", "get_closest_point_owner");
	___mb.mb_get_simple_path = godot::api->godot_method_bind_get_method("Navigation2D", "get_simple_path");
	___mb.mb_navpoly_add = godot::api->godot_method_bind_get_method("Navigation2D", "navpoly_add");
	___mb.mb_navpoly_remove = godot::api->godot_method_bind_get_method("Navigation2D", "navpoly_remove");
	___mb.mb_navpoly_set_transform = godot::api->godot_method_bind_get_method("Navigation2D", "navpoly_set_transform");
}

Navigation2D *Navigation2D::_new()
{
	return (Navigation2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Navigation2D")());
}
Vector2 Navigation2D::get_closest_point(const Vector2 to_point) {
	return ___godot_icall_Vector2_Vector2(___mb.mb_get_closest_point, (const Object *) this, to_point);
}

Object *Navigation2D::get_closest_point_owner(const Vector2 to_point) {
	return (Object *) ___godot_icall_Object_Vector2(___mb.mb_get_closest_point_owner, (const Object *) this, to_point);
}

PoolVector2Array Navigation2D::get_simple_path(const Vector2 start, const Vector2 end, const bool optimize) {
	return ___godot_icall_PoolVector2Array_Vector2_Vector2_bool(___mb.mb_get_simple_path, (const Object *) this, start, end, optimize);
}

int64_t Navigation2D::navpoly_add(const Ref<NavigationPolygon> mesh, const Transform2D xform, const Object *owner) {
	return ___godot_icall_int_Object_Transform2D_Object(___mb.mb_navpoly_add, (const Object *) this, mesh.ptr(), xform, owner);
}

void Navigation2D::navpoly_remove(const int64_t id) {
	___godot_icall_void_int(___mb.mb_navpoly_remove, (const Object *) this, id);
}

void Navigation2D::navpoly_set_transform(const int64_t id, const Transform2D xform) {
	___godot_icall_void_int_Transform2D(___mb.mb_navpoly_set_transform, (const Object *) this, id, xform);
}

}