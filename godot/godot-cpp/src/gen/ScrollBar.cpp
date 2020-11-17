#include "ScrollBar.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"


namespace godot {


ScrollBar::___method_bindings ScrollBar::___mb = {};

void ScrollBar::___init_method_bindings() {
	___mb.mb__drag_node_exit = godot::api->godot_method_bind_get_method("ScrollBar", "_drag_node_exit");
	___mb.mb__drag_node_input = godot::api->godot_method_bind_get_method("ScrollBar", "_drag_node_input");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("ScrollBar", "_gui_input");
	___mb.mb_get_custom_step = godot::api->godot_method_bind_get_method("ScrollBar", "get_custom_step");
	___mb.mb_set_custom_step = godot::api->godot_method_bind_get_method("ScrollBar", "set_custom_step");
}

void ScrollBar::_drag_node_exit() {
	___godot_icall_void(___mb.mb__drag_node_exit, (const Object *) this);
}

void ScrollBar::_drag_node_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__drag_node_input, (const Object *) this, arg0.ptr());
}

void ScrollBar::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

real_t ScrollBar::get_custom_step() const {
	return ___godot_icall_float(___mb.mb_get_custom_step, (const Object *) this);
}

void ScrollBar::set_custom_step(const real_t step) {
	___godot_icall_void_float(___mb.mb_set_custom_step, (const Object *) this, step);
}

}