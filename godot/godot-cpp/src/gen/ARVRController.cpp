#include "ARVRController.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Mesh.hpp"


namespace godot {


ARVRController::___method_bindings ARVRController::___mb = {};

void ARVRController::___init_method_bindings() {
	___mb.mb_get_controller_id = godot::api->godot_method_bind_get_method("ARVRController", "get_controller_id");
	___mb.mb_get_controller_name = godot::api->godot_method_bind_get_method("ARVRController", "get_controller_name");
	___mb.mb_get_hand = godot::api->godot_method_bind_get_method("ARVRController", "get_hand");
	___mb.mb_get_is_active = godot::api->godot_method_bind_get_method("ARVRController", "get_is_active");
	___mb.mb_get_joystick_axis = godot::api->godot_method_bind_get_method("ARVRController", "get_joystick_axis");
	___mb.mb_get_joystick_id = godot::api->godot_method_bind_get_method("ARVRController", "get_joystick_id");
	___mb.mb_get_mesh = godot::api->godot_method_bind_get_method("ARVRController", "get_mesh");
	___mb.mb_get_rumble = godot::api->godot_method_bind_get_method("ARVRController", "get_rumble");
	___mb.mb_is_button_pressed = godot::api->godot_method_bind_get_method("ARVRController", "is_button_pressed");
	___mb.mb_set_controller_id = godot::api->godot_method_bind_get_method("ARVRController", "set_controller_id");
	___mb.mb_set_rumble = godot::api->godot_method_bind_get_method("ARVRController", "set_rumble");
}

ARVRController *ARVRController::_new()
{
	return (ARVRController *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ARVRController")());
}
int64_t ARVRController::get_controller_id() const {
	return ___godot_icall_int(___mb.mb_get_controller_id, (const Object *) this);
}

String ARVRController::get_controller_name() const {
	return ___godot_icall_String(___mb.mb_get_controller_name, (const Object *) this);
}

ARVRPositionalTracker::TrackerHand ARVRController::get_hand() const {
	return (ARVRPositionalTracker::TrackerHand) ___godot_icall_int(___mb.mb_get_hand, (const Object *) this);
}

bool ARVRController::get_is_active() const {
	return ___godot_icall_bool(___mb.mb_get_is_active, (const Object *) this);
}

real_t ARVRController::get_joystick_axis(const int64_t axis) const {
	return ___godot_icall_float_int(___mb.mb_get_joystick_axis, (const Object *) this, axis);
}

int64_t ARVRController::get_joystick_id() const {
	return ___godot_icall_int(___mb.mb_get_joystick_id, (const Object *) this);
}

Ref<Mesh> ARVRController::get_mesh() const {
	return Ref<Mesh>::__internal_constructor(___godot_icall_Object(___mb.mb_get_mesh, (const Object *) this));
}

real_t ARVRController::get_rumble() const {
	return ___godot_icall_float(___mb.mb_get_rumble, (const Object *) this);
}

int64_t ARVRController::is_button_pressed(const int64_t button) const {
	return ___godot_icall_int_int(___mb.mb_is_button_pressed, (const Object *) this, button);
}

void ARVRController::set_controller_id(const int64_t controller_id) {
	___godot_icall_void_int(___mb.mb_set_controller_id, (const Object *) this, controller_id);
}

void ARVRController::set_rumble(const real_t rumble) {
	___godot_icall_void_float(___mb.mb_set_rumble, (const Object *) this, rumble);
}

}