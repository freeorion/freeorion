#include "MobileVRInterface.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


MobileVRInterface::___method_bindings MobileVRInterface::___mb = {};

void MobileVRInterface::___init_method_bindings() {
	___mb.mb_get_display_to_lens = godot::api->godot_method_bind_get_method("MobileVRInterface", "get_display_to_lens");
	___mb.mb_get_display_width = godot::api->godot_method_bind_get_method("MobileVRInterface", "get_display_width");
	___mb.mb_get_eye_height = godot::api->godot_method_bind_get_method("MobileVRInterface", "get_eye_height");
	___mb.mb_get_iod = godot::api->godot_method_bind_get_method("MobileVRInterface", "get_iod");
	___mb.mb_get_k1 = godot::api->godot_method_bind_get_method("MobileVRInterface", "get_k1");
	___mb.mb_get_k2 = godot::api->godot_method_bind_get_method("MobileVRInterface", "get_k2");
	___mb.mb_get_oversample = godot::api->godot_method_bind_get_method("MobileVRInterface", "get_oversample");
	___mb.mb_set_display_to_lens = godot::api->godot_method_bind_get_method("MobileVRInterface", "set_display_to_lens");
	___mb.mb_set_display_width = godot::api->godot_method_bind_get_method("MobileVRInterface", "set_display_width");
	___mb.mb_set_eye_height = godot::api->godot_method_bind_get_method("MobileVRInterface", "set_eye_height");
	___mb.mb_set_iod = godot::api->godot_method_bind_get_method("MobileVRInterface", "set_iod");
	___mb.mb_set_k1 = godot::api->godot_method_bind_get_method("MobileVRInterface", "set_k1");
	___mb.mb_set_k2 = godot::api->godot_method_bind_get_method("MobileVRInterface", "set_k2");
	___mb.mb_set_oversample = godot::api->godot_method_bind_get_method("MobileVRInterface", "set_oversample");
}

MobileVRInterface *MobileVRInterface::_new()
{
	return (MobileVRInterface *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MobileVRInterface")());
}
real_t MobileVRInterface::get_display_to_lens() const {
	return ___godot_icall_float(___mb.mb_get_display_to_lens, (const Object *) this);
}

real_t MobileVRInterface::get_display_width() const {
	return ___godot_icall_float(___mb.mb_get_display_width, (const Object *) this);
}

real_t MobileVRInterface::get_eye_height() const {
	return ___godot_icall_float(___mb.mb_get_eye_height, (const Object *) this);
}

real_t MobileVRInterface::get_iod() const {
	return ___godot_icall_float(___mb.mb_get_iod, (const Object *) this);
}

real_t MobileVRInterface::get_k1() const {
	return ___godot_icall_float(___mb.mb_get_k1, (const Object *) this);
}

real_t MobileVRInterface::get_k2() const {
	return ___godot_icall_float(___mb.mb_get_k2, (const Object *) this);
}

real_t MobileVRInterface::get_oversample() const {
	return ___godot_icall_float(___mb.mb_get_oversample, (const Object *) this);
}

void MobileVRInterface::set_display_to_lens(const real_t display_to_lens) {
	___godot_icall_void_float(___mb.mb_set_display_to_lens, (const Object *) this, display_to_lens);
}

void MobileVRInterface::set_display_width(const real_t display_width) {
	___godot_icall_void_float(___mb.mb_set_display_width, (const Object *) this, display_width);
}

void MobileVRInterface::set_eye_height(const real_t eye_height) {
	___godot_icall_void_float(___mb.mb_set_eye_height, (const Object *) this, eye_height);
}

void MobileVRInterface::set_iod(const real_t iod) {
	___godot_icall_void_float(___mb.mb_set_iod, (const Object *) this, iod);
}

void MobileVRInterface::set_k1(const real_t k) {
	___godot_icall_void_float(___mb.mb_set_k1, (const Object *) this, k);
}

void MobileVRInterface::set_k2(const real_t k) {
	___godot_icall_void_float(___mb.mb_set_k2, (const Object *) this, k);
}

void MobileVRInterface::set_oversample(const real_t oversample) {
	___godot_icall_void_float(___mb.mb_set_oversample, (const Object *) this, oversample);
}

}