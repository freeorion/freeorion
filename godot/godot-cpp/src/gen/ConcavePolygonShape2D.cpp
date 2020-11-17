#include "ConcavePolygonShape2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ConcavePolygonShape2D::___method_bindings ConcavePolygonShape2D::___mb = {};

void ConcavePolygonShape2D::___init_method_bindings() {
	___mb.mb_get_segments = godot::api->godot_method_bind_get_method("ConcavePolygonShape2D", "get_segments");
	___mb.mb_set_segments = godot::api->godot_method_bind_get_method("ConcavePolygonShape2D", "set_segments");
}

ConcavePolygonShape2D *ConcavePolygonShape2D::_new()
{
	return (ConcavePolygonShape2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ConcavePolygonShape2D")());
}
PoolVector2Array ConcavePolygonShape2D::get_segments() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_segments, (const Object *) this);
}

void ConcavePolygonShape2D::set_segments(const PoolVector2Array segments) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_segments, (const Object *) this, segments);
}

}