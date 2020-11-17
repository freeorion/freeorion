#include "ViewportContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"


namespace godot {


ViewportContainer::___method_bindings ViewportContainer::___mb = {};

void ViewportContainer::___init_method_bindings() {
	___mb.mb__input = godot::api->godot_method_bind_get_method("ViewportContainer", "_input");
	___mb.mb__unhandled_input = godot::api->godot_method_bind_get_method("ViewportContainer", "_unhandled_input");
	___mb.mb_get_stretch_shrink = godot::api->godot_method_bind_get_method("ViewportContainer", "get_stretch_shrink");
	___mb.mb_is_stretch_enabled = godot::api->godot_method_bind_get_method("ViewportContainer", "is_stretch_enabled");
	___mb.mb_set_stretch = godot::api->godot_method_bind_get_method("ViewportContainer", "set_stretch");
	___mb.mb_set_stretch_shrink = godot::api->godot_method_bind_get_method("ViewportContainer", "set_stretch_shrink");
}

ViewportContainer *ViewportContainer::_new()
{
	return (ViewportContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ViewportContainer")());
}
void ViewportContainer::_input(const Ref<InputEvent> event) {
	___godot_icall_void_Object(___mb.mb__input, (const Object *) this, event.ptr());
}

void ViewportContainer::_unhandled_input(const Ref<InputEvent> event) {
	___godot_icall_void_Object(___mb.mb__unhandled_input, (const Object *) this, event.ptr());
}

int64_t ViewportContainer::get_stretch_shrink() const {
	return ___godot_icall_int(___mb.mb_get_stretch_shrink, (const Object *) this);
}

bool ViewportContainer::is_stretch_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_stretch_enabled, (const Object *) this);
}

void ViewportContainer::set_stretch(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_stretch, (const Object *) this, enable);
}

void ViewportContainer::set_stretch_shrink(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_stretch_shrink, (const Object *) this, amount);
}

}