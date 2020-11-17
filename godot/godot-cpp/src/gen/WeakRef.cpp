#include "WeakRef.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


WeakRef::___method_bindings WeakRef::___mb = {};

void WeakRef::___init_method_bindings() {
	___mb.mb_get_ref = godot::api->godot_method_bind_get_method("WeakRef", "get_ref");
}

WeakRef *WeakRef::_new()
{
	return (WeakRef *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"WeakRef")());
}
Variant WeakRef::get_ref() const {
	return ___godot_icall_Variant(___mb.mb_get_ref, (const Object *) this);
}

}