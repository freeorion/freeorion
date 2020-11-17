#include "VisualScriptDeconstruct.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptDeconstruct::___method_bindings VisualScriptDeconstruct::___mb = {};

void VisualScriptDeconstruct::___init_method_bindings() {
	___mb.mb__get_elem_cache = godot::api->godot_method_bind_get_method("VisualScriptDeconstruct", "_get_elem_cache");
	___mb.mb__set_elem_cache = godot::api->godot_method_bind_get_method("VisualScriptDeconstruct", "_set_elem_cache");
	___mb.mb_get_deconstruct_type = godot::api->godot_method_bind_get_method("VisualScriptDeconstruct", "get_deconstruct_type");
	___mb.mb_set_deconstruct_type = godot::api->godot_method_bind_get_method("VisualScriptDeconstruct", "set_deconstruct_type");
}

VisualScriptDeconstruct *VisualScriptDeconstruct::_new()
{
	return (VisualScriptDeconstruct *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptDeconstruct")());
}
Array VisualScriptDeconstruct::_get_elem_cache() const {
	return ___godot_icall_Array(___mb.mb__get_elem_cache, (const Object *) this);
}

void VisualScriptDeconstruct::_set_elem_cache(const Array _cache) {
	___godot_icall_void_Array(___mb.mb__set_elem_cache, (const Object *) this, _cache);
}

Variant::Type VisualScriptDeconstruct::get_deconstruct_type() const {
	return (Variant::Type) ___godot_icall_int(___mb.mb_get_deconstruct_type, (const Object *) this);
}

void VisualScriptDeconstruct::set_deconstruct_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb_set_deconstruct_type, (const Object *) this, type);
}

}