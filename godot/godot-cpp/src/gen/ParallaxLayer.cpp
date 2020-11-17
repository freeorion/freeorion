#include "ParallaxLayer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ParallaxLayer::___method_bindings ParallaxLayer::___mb = {};

void ParallaxLayer::___init_method_bindings() {
	___mb.mb_get_mirroring = godot::api->godot_method_bind_get_method("ParallaxLayer", "get_mirroring");
	___mb.mb_get_motion_offset = godot::api->godot_method_bind_get_method("ParallaxLayer", "get_motion_offset");
	___mb.mb_get_motion_scale = godot::api->godot_method_bind_get_method("ParallaxLayer", "get_motion_scale");
	___mb.mb_set_mirroring = godot::api->godot_method_bind_get_method("ParallaxLayer", "set_mirroring");
	___mb.mb_set_motion_offset = godot::api->godot_method_bind_get_method("ParallaxLayer", "set_motion_offset");
	___mb.mb_set_motion_scale = godot::api->godot_method_bind_get_method("ParallaxLayer", "set_motion_scale");
}

ParallaxLayer *ParallaxLayer::_new()
{
	return (ParallaxLayer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ParallaxLayer")());
}
Vector2 ParallaxLayer::get_mirroring() const {
	return ___godot_icall_Vector2(___mb.mb_get_mirroring, (const Object *) this);
}

Vector2 ParallaxLayer::get_motion_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_motion_offset, (const Object *) this);
}

Vector2 ParallaxLayer::get_motion_scale() const {
	return ___godot_icall_Vector2(___mb.mb_get_motion_scale, (const Object *) this);
}

void ParallaxLayer::set_mirroring(const Vector2 mirror) {
	___godot_icall_void_Vector2(___mb.mb_set_mirroring, (const Object *) this, mirror);
}

void ParallaxLayer::set_motion_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_motion_offset, (const Object *) this, offset);
}

void ParallaxLayer::set_motion_scale(const Vector2 scale) {
	___godot_icall_void_Vector2(___mb.mb_set_motion_scale, (const Object *) this, scale);
}

}