#include "EncodedObjectAsID.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


EncodedObjectAsID::___method_bindings EncodedObjectAsID::___mb = {};

void EncodedObjectAsID::___init_method_bindings() {
	___mb.mb_get_object_id = godot::api->godot_method_bind_get_method("EncodedObjectAsID", "get_object_id");
	___mb.mb_set_object_id = godot::api->godot_method_bind_get_method("EncodedObjectAsID", "set_object_id");
}

EncodedObjectAsID *EncodedObjectAsID::_new()
{
	return (EncodedObjectAsID *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"EncodedObjectAsID")());
}
int64_t EncodedObjectAsID::get_object_id() const {
	return ___godot_icall_int(___mb.mb_get_object_id, (const Object *) this);
}

void EncodedObjectAsID::set_object_id(const int64_t id) {
	___godot_icall_void_int(___mb.mb_set_object_id, (const Object *) this, id);
}

}