#include "ARVRPositionalTracker.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Mesh.hpp"


namespace godot {


ARVRPositionalTracker::___method_bindings ARVRPositionalTracker::___mb = {};

void ARVRPositionalTracker::___init_method_bindings() {
	___mb.mb__set_joy_id = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "_set_joy_id");
	___mb.mb__set_mesh = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "_set_mesh");
	___mb.mb__set_name = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "_set_name");
	___mb.mb__set_orientation = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "_set_orientation");
	___mb.mb__set_rw_position = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "_set_rw_position");
	___mb.mb__set_type = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "_set_type");
	___mb.mb_get_hand = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_hand");
	___mb.mb_get_joy_id = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_joy_id");
	___mb.mb_get_mesh = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_mesh");
	___mb.mb_get_name = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_name");
	___mb.mb_get_orientation = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_orientation");
	___mb.mb_get_position = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_position");
	___mb.mb_get_rumble = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_rumble");
	___mb.mb_get_tracks_orientation = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_tracks_orientation");
	___mb.mb_get_tracks_position = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_tracks_position");
	___mb.mb_get_transform = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_transform");
	___mb.mb_get_type = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "get_type");
	___mb.mb_set_rumble = godot::api->godot_method_bind_get_method("ARVRPositionalTracker", "set_rumble");
}

ARVRPositionalTracker *ARVRPositionalTracker::_new()
{
	return (ARVRPositionalTracker *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ARVRPositionalTracker")());
}
void ARVRPositionalTracker::_set_joy_id(const int64_t joy_id) {
	___godot_icall_void_int(___mb.mb__set_joy_id, (const Object *) this, joy_id);
}

void ARVRPositionalTracker::_set_mesh(const Ref<Mesh> mesh) {
	___godot_icall_void_Object(___mb.mb__set_mesh, (const Object *) this, mesh.ptr());
}

void ARVRPositionalTracker::_set_name(const String name) {
	___godot_icall_void_String(___mb.mb__set_name, (const Object *) this, name);
}

void ARVRPositionalTracker::_set_orientation(const Basis orientation) {
	___godot_icall_void_Basis(___mb.mb__set_orientation, (const Object *) this, orientation);
}

void ARVRPositionalTracker::_set_rw_position(const Vector3 rw_position) {
	___godot_icall_void_Vector3(___mb.mb__set_rw_position, (const Object *) this, rw_position);
}

void ARVRPositionalTracker::_set_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb__set_type, (const Object *) this, type);
}

ARVRPositionalTracker::TrackerHand ARVRPositionalTracker::get_hand() const {
	return (ARVRPositionalTracker::TrackerHand) ___godot_icall_int(___mb.mb_get_hand, (const Object *) this);
}

int64_t ARVRPositionalTracker::get_joy_id() const {
	return ___godot_icall_int(___mb.mb_get_joy_id, (const Object *) this);
}

Ref<Mesh> ARVRPositionalTracker::get_mesh() const {
	return Ref<Mesh>::__internal_constructor(___godot_icall_Object(___mb.mb_get_mesh, (const Object *) this));
}

String ARVRPositionalTracker::get_name() const {
	return ___godot_icall_String(___mb.mb_get_name, (const Object *) this);
}

Basis ARVRPositionalTracker::get_orientation() const {
	return ___godot_icall_Basis(___mb.mb_get_orientation, (const Object *) this);
}

Vector3 ARVRPositionalTracker::get_position() const {
	return ___godot_icall_Vector3(___mb.mb_get_position, (const Object *) this);
}

real_t ARVRPositionalTracker::get_rumble() const {
	return ___godot_icall_float(___mb.mb_get_rumble, (const Object *) this);
}

bool ARVRPositionalTracker::get_tracks_orientation() const {
	return ___godot_icall_bool(___mb.mb_get_tracks_orientation, (const Object *) this);
}

bool ARVRPositionalTracker::get_tracks_position() const {
	return ___godot_icall_bool(___mb.mb_get_tracks_position, (const Object *) this);
}

Transform ARVRPositionalTracker::get_transform(const bool adjust_by_reference_frame) const {
	return ___godot_icall_Transform_bool(___mb.mb_get_transform, (const Object *) this, adjust_by_reference_frame);
}

ARVRServer::TrackerType ARVRPositionalTracker::get_type() const {
	return (ARVRServer::TrackerType) ___godot_icall_int(___mb.mb_get_type, (const Object *) this);
}

void ARVRPositionalTracker::set_rumble(const real_t rumble) {
	___godot_icall_void_float(___mb.mb_set_rumble, (const Object *) this, rumble);
}

}