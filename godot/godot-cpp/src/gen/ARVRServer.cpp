#include "ARVRServer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "ARVRInterface.hpp"
#include "ARVRPositionalTracker.hpp"


namespace godot {


ARVRServer *ARVRServer::_singleton = NULL;


ARVRServer::ARVRServer() {
	_owner = godot::api->godot_global_get_singleton((char *) "ARVRServer");
}


ARVRServer::___method_bindings ARVRServer::___mb = {};

void ARVRServer::___init_method_bindings() {
	___mb.mb_center_on_hmd = godot::api->godot_method_bind_get_method("ARVRServer", "center_on_hmd");
	___mb.mb_find_interface = godot::api->godot_method_bind_get_method("ARVRServer", "find_interface");
	___mb.mb_get_hmd_transform = godot::api->godot_method_bind_get_method("ARVRServer", "get_hmd_transform");
	___mb.mb_get_interface = godot::api->godot_method_bind_get_method("ARVRServer", "get_interface");
	___mb.mb_get_interface_count = godot::api->godot_method_bind_get_method("ARVRServer", "get_interface_count");
	___mb.mb_get_interfaces = godot::api->godot_method_bind_get_method("ARVRServer", "get_interfaces");
	___mb.mb_get_last_commit_usec = godot::api->godot_method_bind_get_method("ARVRServer", "get_last_commit_usec");
	___mb.mb_get_last_frame_usec = godot::api->godot_method_bind_get_method("ARVRServer", "get_last_frame_usec");
	___mb.mb_get_last_process_usec = godot::api->godot_method_bind_get_method("ARVRServer", "get_last_process_usec");
	___mb.mb_get_primary_interface = godot::api->godot_method_bind_get_method("ARVRServer", "get_primary_interface");
	___mb.mb_get_reference_frame = godot::api->godot_method_bind_get_method("ARVRServer", "get_reference_frame");
	___mb.mb_get_tracker = godot::api->godot_method_bind_get_method("ARVRServer", "get_tracker");
	___mb.mb_get_tracker_count = godot::api->godot_method_bind_get_method("ARVRServer", "get_tracker_count");
	___mb.mb_get_world_scale = godot::api->godot_method_bind_get_method("ARVRServer", "get_world_scale");
	___mb.mb_set_primary_interface = godot::api->godot_method_bind_get_method("ARVRServer", "set_primary_interface");
	___mb.mb_set_world_scale = godot::api->godot_method_bind_get_method("ARVRServer", "set_world_scale");
}

void ARVRServer::center_on_hmd(const int64_t rotation_mode, const bool keep_height) {
	___godot_icall_void_int_bool(___mb.mb_center_on_hmd, (const Object *) this, rotation_mode, keep_height);
}

Ref<ARVRInterface> ARVRServer::find_interface(const String name) const {
	return Ref<ARVRInterface>::__internal_constructor(___godot_icall_Object_String(___mb.mb_find_interface, (const Object *) this, name));
}

Transform ARVRServer::get_hmd_transform() {
	return ___godot_icall_Transform(___mb.mb_get_hmd_transform, (const Object *) this);
}

Ref<ARVRInterface> ARVRServer::get_interface(const int64_t idx) const {
	return Ref<ARVRInterface>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_interface, (const Object *) this, idx));
}

int64_t ARVRServer::get_interface_count() const {
	return ___godot_icall_int(___mb.mb_get_interface_count, (const Object *) this);
}

Array ARVRServer::get_interfaces() const {
	return ___godot_icall_Array(___mb.mb_get_interfaces, (const Object *) this);
}

int64_t ARVRServer::get_last_commit_usec() {
	return ___godot_icall_int(___mb.mb_get_last_commit_usec, (const Object *) this);
}

int64_t ARVRServer::get_last_frame_usec() {
	return ___godot_icall_int(___mb.mb_get_last_frame_usec, (const Object *) this);
}

int64_t ARVRServer::get_last_process_usec() {
	return ___godot_icall_int(___mb.mb_get_last_process_usec, (const Object *) this);
}

Ref<ARVRInterface> ARVRServer::get_primary_interface() const {
	return Ref<ARVRInterface>::__internal_constructor(___godot_icall_Object(___mb.mb_get_primary_interface, (const Object *) this));
}

Transform ARVRServer::get_reference_frame() const {
	return ___godot_icall_Transform(___mb.mb_get_reference_frame, (const Object *) this);
}

ARVRPositionalTracker *ARVRServer::get_tracker(const int64_t idx) const {
	return (ARVRPositionalTracker *) ___godot_icall_Object_int(___mb.mb_get_tracker, (const Object *) this, idx);
}

int64_t ARVRServer::get_tracker_count() const {
	return ___godot_icall_int(___mb.mb_get_tracker_count, (const Object *) this);
}

real_t ARVRServer::get_world_scale() const {
	return ___godot_icall_float(___mb.mb_get_world_scale, (const Object *) this);
}

void ARVRServer::set_primary_interface(const Ref<ARVRInterface> interface) {
	___godot_icall_void_Object(___mb.mb_set_primary_interface, (const Object *) this, interface.ptr());
}

void ARVRServer::set_world_scale(const real_t arg0) {
	___godot_icall_void_float(___mb.mb_set_world_scale, (const Object *) this, arg0);
}

}