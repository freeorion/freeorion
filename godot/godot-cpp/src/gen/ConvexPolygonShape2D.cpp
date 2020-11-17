#include "ConvexPolygonShape2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ConvexPolygonShape2D::___method_bindings ConvexPolygonShape2D::___mb = {};

void ConvexPolygonShape2D::___init_method_bindings() {
	___mb.mb_get_points = godot::api->godot_method_bind_get_method("ConvexPolygonShape2D", "get_points");
	___mb.mb_set_point_cloud = godot::api->godot_method_bind_get_method("ConvexPolygonShape2D", "set_point_cloud");
	___mb.mb_set_points = godot::api->godot_method_bind_get_method("ConvexPolygonShape2D", "set_points");
}

ConvexPolygonShape2D *ConvexPolygonShape2D::_new()
{
	return (ConvexPolygonShape2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ConvexPolygonShape2D")());
}
PoolVector2Array ConvexPolygonShape2D::get_points() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_points, (const Object *) this);
}

void ConvexPolygonShape2D::set_point_cloud(const PoolVector2Array point_cloud) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_point_cloud, (const Object *) this, point_cloud);
}

void ConvexPolygonShape2D::set_points(const PoolVector2Array points) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_points, (const Object *) this, points);
}

}