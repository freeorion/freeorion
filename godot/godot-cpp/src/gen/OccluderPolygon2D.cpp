#include "OccluderPolygon2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


OccluderPolygon2D::___method_bindings OccluderPolygon2D::___mb = {};

void OccluderPolygon2D::___init_method_bindings() {
	___mb.mb_get_cull_mode = godot::api->godot_method_bind_get_method("OccluderPolygon2D", "get_cull_mode");
	___mb.mb_get_polygon = godot::api->godot_method_bind_get_method("OccluderPolygon2D", "get_polygon");
	___mb.mb_is_closed = godot::api->godot_method_bind_get_method("OccluderPolygon2D", "is_closed");
	___mb.mb_set_closed = godot::api->godot_method_bind_get_method("OccluderPolygon2D", "set_closed");
	___mb.mb_set_cull_mode = godot::api->godot_method_bind_get_method("OccluderPolygon2D", "set_cull_mode");
	___mb.mb_set_polygon = godot::api->godot_method_bind_get_method("OccluderPolygon2D", "set_polygon");
}

OccluderPolygon2D *OccluderPolygon2D::_new()
{
	return (OccluderPolygon2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"OccluderPolygon2D")());
}
OccluderPolygon2D::CullMode OccluderPolygon2D::get_cull_mode() const {
	return (OccluderPolygon2D::CullMode) ___godot_icall_int(___mb.mb_get_cull_mode, (const Object *) this);
}

PoolVector2Array OccluderPolygon2D::get_polygon() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_polygon, (const Object *) this);
}

bool OccluderPolygon2D::is_closed() const {
	return ___godot_icall_bool(___mb.mb_is_closed, (const Object *) this);
}

void OccluderPolygon2D::set_closed(const bool closed) {
	___godot_icall_void_bool(___mb.mb_set_closed, (const Object *) this, closed);
}

void OccluderPolygon2D::set_cull_mode(const int64_t cull_mode) {
	___godot_icall_void_int(___mb.mb_set_cull_mode, (const Object *) this, cull_mode);
}

void OccluderPolygon2D::set_polygon(const PoolVector2Array polygon) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_polygon, (const Object *) this, polygon);
}

}