#include "VisibilityNotifier.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisibilityNotifier::___method_bindings VisibilityNotifier::___mb = {};

void VisibilityNotifier::___init_method_bindings() {
	___mb.mb_get_aabb = godot::api->godot_method_bind_get_method("VisibilityNotifier", "get_aabb");
	___mb.mb_is_on_screen = godot::api->godot_method_bind_get_method("VisibilityNotifier", "is_on_screen");
	___mb.mb_set_aabb = godot::api->godot_method_bind_get_method("VisibilityNotifier", "set_aabb");
}

VisibilityNotifier *VisibilityNotifier::_new()
{
	return (VisibilityNotifier *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisibilityNotifier")());
}
AABB VisibilityNotifier::get_aabb() const {
	return ___godot_icall_AABB(___mb.mb_get_aabb, (const Object *) this);
}

bool VisibilityNotifier::is_on_screen() const {
	return ___godot_icall_bool(___mb.mb_is_on_screen, (const Object *) this);
}

void VisibilityNotifier::set_aabb(const AABB rect) {
	___godot_icall_void_AABB(___mb.mb_set_aabb, (const Object *) this, rect);
}

}