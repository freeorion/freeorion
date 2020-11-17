#include "VisibilityNotifier2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisibilityNotifier2D::___method_bindings VisibilityNotifier2D::___mb = {};

void VisibilityNotifier2D::___init_method_bindings() {
	___mb.mb_get_rect = godot::api->godot_method_bind_get_method("VisibilityNotifier2D", "get_rect");
	___mb.mb_is_on_screen = godot::api->godot_method_bind_get_method("VisibilityNotifier2D", "is_on_screen");
	___mb.mb_set_rect = godot::api->godot_method_bind_get_method("VisibilityNotifier2D", "set_rect");
}

VisibilityNotifier2D *VisibilityNotifier2D::_new()
{
	return (VisibilityNotifier2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisibilityNotifier2D")());
}
Rect2 VisibilityNotifier2D::get_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_rect, (const Object *) this);
}

bool VisibilityNotifier2D::is_on_screen() const {
	return ___godot_icall_bool(___mb.mb_is_on_screen, (const Object *) this);
}

void VisibilityNotifier2D::set_rect(const Rect2 rect) {
	___godot_icall_void_Rect2(___mb.mb_set_rect, (const Object *) this, rect);
}

}