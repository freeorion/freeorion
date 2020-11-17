#include "ConvexPolygonShape.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ConvexPolygonShape::___method_bindings ConvexPolygonShape::___mb = {};

void ConvexPolygonShape::___init_method_bindings() {
	___mb.mb_get_points = godot::api->godot_method_bind_get_method("ConvexPolygonShape", "get_points");
	___mb.mb_set_points = godot::api->godot_method_bind_get_method("ConvexPolygonShape", "set_points");
}

ConvexPolygonShape *ConvexPolygonShape::_new()
{
	return (ConvexPolygonShape *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ConvexPolygonShape")());
}
PoolVector3Array ConvexPolygonShape::get_points() const {
	return ___godot_icall_PoolVector3Array(___mb.mb_get_points, (const Object *) this);
}

void ConvexPolygonShape::set_points(const PoolVector3Array points) {
	___godot_icall_void_PoolVector3Array(___mb.mb_set_points, (const Object *) this, points);
}

}