#include "ARVRInterface.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ARVRInterface::___method_bindings ARVRInterface::___mb = {};

void ARVRInterface::___init_method_bindings() {
	___mb.mb_get_anchor_detection_is_enabled = godot::api->godot_method_bind_get_method("ARVRInterface", "get_anchor_detection_is_enabled");
	___mb.mb_get_camera_feed_id = godot::api->godot_method_bind_get_method("ARVRInterface", "get_camera_feed_id");
	___mb.mb_get_capabilities = godot::api->godot_method_bind_get_method("ARVRInterface", "get_capabilities");
	___mb.mb_get_name = godot::api->godot_method_bind_get_method("ARVRInterface", "get_name");
	___mb.mb_get_render_targetsize = godot::api->godot_method_bind_get_method("ARVRInterface", "get_render_targetsize");
	___mb.mb_get_tracking_status = godot::api->godot_method_bind_get_method("ARVRInterface", "get_tracking_status");
	___mb.mb_initialize = godot::api->godot_method_bind_get_method("ARVRInterface", "initialize");
	___mb.mb_is_initialized = godot::api->godot_method_bind_get_method("ARVRInterface", "is_initialized");
	___mb.mb_is_primary = godot::api->godot_method_bind_get_method("ARVRInterface", "is_primary");
	___mb.mb_is_stereo = godot::api->godot_method_bind_get_method("ARVRInterface", "is_stereo");
	___mb.mb_set_anchor_detection_is_enabled = godot::api->godot_method_bind_get_method("ARVRInterface", "set_anchor_detection_is_enabled");
	___mb.mb_set_is_initialized = godot::api->godot_method_bind_get_method("ARVRInterface", "set_is_initialized");
	___mb.mb_set_is_primary = godot::api->godot_method_bind_get_method("ARVRInterface", "set_is_primary");
	___mb.mb_uninitialize = godot::api->godot_method_bind_get_method("ARVRInterface", "uninitialize");
}

bool ARVRInterface::get_anchor_detection_is_enabled() const {
	return ___godot_icall_bool(___mb.mb_get_anchor_detection_is_enabled, (const Object *) this);
}

int64_t ARVRInterface::get_camera_feed_id() {
	return ___godot_icall_int(___mb.mb_get_camera_feed_id, (const Object *) this);
}

int64_t ARVRInterface::get_capabilities() const {
	return ___godot_icall_int(___mb.mb_get_capabilities, (const Object *) this);
}

String ARVRInterface::get_name() const {
	return ___godot_icall_String(___mb.mb_get_name, (const Object *) this);
}

Vector2 ARVRInterface::get_render_targetsize() {
	return ___godot_icall_Vector2(___mb.mb_get_render_targetsize, (const Object *) this);
}

ARVRInterface::Tracking_status ARVRInterface::get_tracking_status() const {
	return (ARVRInterface::Tracking_status) ___godot_icall_int(___mb.mb_get_tracking_status, (const Object *) this);
}

bool ARVRInterface::initialize() {
	return ___godot_icall_bool(___mb.mb_initialize, (const Object *) this);
}

bool ARVRInterface::is_initialized() const {
	return ___godot_icall_bool(___mb.mb_is_initialized, (const Object *) this);
}

bool ARVRInterface::is_primary() {
	return ___godot_icall_bool(___mb.mb_is_primary, (const Object *) this);
}

bool ARVRInterface::is_stereo() {
	return ___godot_icall_bool(___mb.mb_is_stereo, (const Object *) this);
}

void ARVRInterface::set_anchor_detection_is_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_anchor_detection_is_enabled, (const Object *) this, enable);
}

void ARVRInterface::set_is_initialized(const bool initialized) {
	___godot_icall_void_bool(___mb.mb_set_is_initialized, (const Object *) this, initialized);
}

void ARVRInterface::set_is_primary(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_is_primary, (const Object *) this, enable);
}

void ARVRInterface::uninitialize() {
	___godot_icall_void(___mb.mb_uninitialize, (const Object *) this);
}

}