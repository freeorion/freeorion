#include "CanvasLayer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"


namespace godot {


CanvasLayer::___method_bindings CanvasLayer::___mb = {};

void CanvasLayer::___init_method_bindings() {
	___mb.mb_get_canvas = godot::api->godot_method_bind_get_method("CanvasLayer", "get_canvas");
	___mb.mb_get_custom_viewport = godot::api->godot_method_bind_get_method("CanvasLayer", "get_custom_viewport");
	___mb.mb_get_follow_viewport_scale = godot::api->godot_method_bind_get_method("CanvasLayer", "get_follow_viewport_scale");
	___mb.mb_get_layer = godot::api->godot_method_bind_get_method("CanvasLayer", "get_layer");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("CanvasLayer", "get_offset");
	___mb.mb_get_rotation = godot::api->godot_method_bind_get_method("CanvasLayer", "get_rotation");
	___mb.mb_get_rotation_degrees = godot::api->godot_method_bind_get_method("CanvasLayer", "get_rotation_degrees");
	___mb.mb_get_scale = godot::api->godot_method_bind_get_method("CanvasLayer", "get_scale");
	___mb.mb_get_transform = godot::api->godot_method_bind_get_method("CanvasLayer", "get_transform");
	___mb.mb_is_following_viewport = godot::api->godot_method_bind_get_method("CanvasLayer", "is_following_viewport");
	___mb.mb_set_custom_viewport = godot::api->godot_method_bind_get_method("CanvasLayer", "set_custom_viewport");
	___mb.mb_set_follow_viewport = godot::api->godot_method_bind_get_method("CanvasLayer", "set_follow_viewport");
	___mb.mb_set_follow_viewport_scale = godot::api->godot_method_bind_get_method("CanvasLayer", "set_follow_viewport_scale");
	___mb.mb_set_layer = godot::api->godot_method_bind_get_method("CanvasLayer", "set_layer");
	___mb.mb_set_offset = godot::api->godot_method_bind_get_method("CanvasLayer", "set_offset");
	___mb.mb_set_rotation = godot::api->godot_method_bind_get_method("CanvasLayer", "set_rotation");
	___mb.mb_set_rotation_degrees = godot::api->godot_method_bind_get_method("CanvasLayer", "set_rotation_degrees");
	___mb.mb_set_scale = godot::api->godot_method_bind_get_method("CanvasLayer", "set_scale");
	___mb.mb_set_transform = godot::api->godot_method_bind_get_method("CanvasLayer", "set_transform");
}

CanvasLayer *CanvasLayer::_new()
{
	return (CanvasLayer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CanvasLayer")());
}
RID CanvasLayer::get_canvas() const {
	return ___godot_icall_RID(___mb.mb_get_canvas, (const Object *) this);
}

Node *CanvasLayer::get_custom_viewport() const {
	return (Node *) ___godot_icall_Object(___mb.mb_get_custom_viewport, (const Object *) this);
}

real_t CanvasLayer::get_follow_viewport_scale() const {
	return ___godot_icall_float(___mb.mb_get_follow_viewport_scale, (const Object *) this);
}

int64_t CanvasLayer::get_layer() const {
	return ___godot_icall_int(___mb.mb_get_layer, (const Object *) this);
}

Vector2 CanvasLayer::get_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_offset, (const Object *) this);
}

real_t CanvasLayer::get_rotation() const {
	return ___godot_icall_float(___mb.mb_get_rotation, (const Object *) this);
}

real_t CanvasLayer::get_rotation_degrees() const {
	return ___godot_icall_float(___mb.mb_get_rotation_degrees, (const Object *) this);
}

Vector2 CanvasLayer::get_scale() const {
	return ___godot_icall_Vector2(___mb.mb_get_scale, (const Object *) this);
}

Transform2D CanvasLayer::get_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_transform, (const Object *) this);
}

bool CanvasLayer::is_following_viewport() const {
	return ___godot_icall_bool(___mb.mb_is_following_viewport, (const Object *) this);
}

void CanvasLayer::set_custom_viewport(const Node *viewport) {
	___godot_icall_void_Object(___mb.mb_set_custom_viewport, (const Object *) this, viewport);
}

void CanvasLayer::set_follow_viewport(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_follow_viewport, (const Object *) this, enable);
}

void CanvasLayer::set_follow_viewport_scale(const real_t scale) {
	___godot_icall_void_float(___mb.mb_set_follow_viewport_scale, (const Object *) this, scale);
}

void CanvasLayer::set_layer(const int64_t layer) {
	___godot_icall_void_int(___mb.mb_set_layer, (const Object *) this, layer);
}

void CanvasLayer::set_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_offset, (const Object *) this, offset);
}

void CanvasLayer::set_rotation(const real_t radians) {
	___godot_icall_void_float(___mb.mb_set_rotation, (const Object *) this, radians);
}

void CanvasLayer::set_rotation_degrees(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_rotation_degrees, (const Object *) this, degrees);
}

void CanvasLayer::set_scale(const Vector2 scale) {
	___godot_icall_void_Vector2(___mb.mb_set_scale, (const Object *) this, scale);
}

void CanvasLayer::set_transform(const Transform2D transform) {
	___godot_icall_void_Transform2D(___mb.mb_set_transform, (const Object *) this, transform);
}

}