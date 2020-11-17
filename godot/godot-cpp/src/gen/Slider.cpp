#include "Slider.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"


namespace godot {


Slider::___method_bindings Slider::___mb = {};

void Slider::___init_method_bindings() {
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("Slider", "_gui_input");
	___mb.mb_get_ticks = godot::api->godot_method_bind_get_method("Slider", "get_ticks");
	___mb.mb_get_ticks_on_borders = godot::api->godot_method_bind_get_method("Slider", "get_ticks_on_borders");
	___mb.mb_is_editable = godot::api->godot_method_bind_get_method("Slider", "is_editable");
	___mb.mb_is_scrollable = godot::api->godot_method_bind_get_method("Slider", "is_scrollable");
	___mb.mb_set_editable = godot::api->godot_method_bind_get_method("Slider", "set_editable");
	___mb.mb_set_scrollable = godot::api->godot_method_bind_get_method("Slider", "set_scrollable");
	___mb.mb_set_ticks = godot::api->godot_method_bind_get_method("Slider", "set_ticks");
	___mb.mb_set_ticks_on_borders = godot::api->godot_method_bind_get_method("Slider", "set_ticks_on_borders");
}

void Slider::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

int64_t Slider::get_ticks() const {
	return ___godot_icall_int(___mb.mb_get_ticks, (const Object *) this);
}

bool Slider::get_ticks_on_borders() const {
	return ___godot_icall_bool(___mb.mb_get_ticks_on_borders, (const Object *) this);
}

bool Slider::is_editable() const {
	return ___godot_icall_bool(___mb.mb_is_editable, (const Object *) this);
}

bool Slider::is_scrollable() const {
	return ___godot_icall_bool(___mb.mb_is_scrollable, (const Object *) this);
}

void Slider::set_editable(const bool editable) {
	___godot_icall_void_bool(___mb.mb_set_editable, (const Object *) this, editable);
}

void Slider::set_scrollable(const bool scrollable) {
	___godot_icall_void_bool(___mb.mb_set_scrollable, (const Object *) this, scrollable);
}

void Slider::set_ticks(const int64_t count) {
	___godot_icall_void_int(___mb.mb_set_ticks, (const Object *) this, count);
}

void Slider::set_ticks_on_borders(const bool ticks_on_border) {
	___godot_icall_void_bool(___mb.mb_set_ticks_on_borders, (const Object *) this, ticks_on_border);
}

}