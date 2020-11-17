#include "Popup.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Popup::___method_bindings Popup::___mb = {};

void Popup::___init_method_bindings() {
	___mb.mb_is_exclusive = godot::api->godot_method_bind_get_method("Popup", "is_exclusive");
	___mb.mb_popup = godot::api->godot_method_bind_get_method("Popup", "popup");
	___mb.mb_popup_centered = godot::api->godot_method_bind_get_method("Popup", "popup_centered");
	___mb.mb_popup_centered_clamped = godot::api->godot_method_bind_get_method("Popup", "popup_centered_clamped");
	___mb.mb_popup_centered_minsize = godot::api->godot_method_bind_get_method("Popup", "popup_centered_minsize");
	___mb.mb_popup_centered_ratio = godot::api->godot_method_bind_get_method("Popup", "popup_centered_ratio");
	___mb.mb_set_as_minsize = godot::api->godot_method_bind_get_method("Popup", "set_as_minsize");
	___mb.mb_set_exclusive = godot::api->godot_method_bind_get_method("Popup", "set_exclusive");
}

Popup *Popup::_new()
{
	return (Popup *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Popup")());
}
bool Popup::is_exclusive() const {
	return ___godot_icall_bool(___mb.mb_is_exclusive, (const Object *) this);
}

void Popup::popup(const Rect2 bounds) {
	___godot_icall_void_Rect2(___mb.mb_popup, (const Object *) this, bounds);
}

void Popup::popup_centered(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_popup_centered, (const Object *) this, size);
}

void Popup::popup_centered_clamped(const Vector2 size, const real_t fallback_ratio) {
	___godot_icall_void_Vector2_float(___mb.mb_popup_centered_clamped, (const Object *) this, size, fallback_ratio);
}

void Popup::popup_centered_minsize(const Vector2 minsize) {
	___godot_icall_void_Vector2(___mb.mb_popup_centered_minsize, (const Object *) this, minsize);
}

void Popup::popup_centered_ratio(const real_t ratio) {
	___godot_icall_void_float(___mb.mb_popup_centered_ratio, (const Object *) this, ratio);
}

void Popup::set_as_minsize() {
	___godot_icall_void(___mb.mb_set_as_minsize, (const Object *) this);
}

void Popup::set_exclusive(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_exclusive, (const Object *) this, enable);
}

}