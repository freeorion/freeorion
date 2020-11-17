#include "CenterContainer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CenterContainer::___method_bindings CenterContainer::___mb = {};

void CenterContainer::___init_method_bindings() {
	___mb.mb_is_using_top_left = godot::api->godot_method_bind_get_method("CenterContainer", "is_using_top_left");
	___mb.mb_set_use_top_left = godot::api->godot_method_bind_get_method("CenterContainer", "set_use_top_left");
}

CenterContainer *CenterContainer::_new()
{
	return (CenterContainer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CenterContainer")());
}
bool CenterContainer::is_using_top_left() const {
	return ___godot_icall_bool(___mb.mb_is_using_top_left, (const Object *) this);
}

void CenterContainer::set_use_top_left(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_top_left, (const Object *) this, enable);
}

}