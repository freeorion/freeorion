#include "Mutex.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Mutex::___method_bindings Mutex::___mb = {};

void Mutex::___init_method_bindings() {
	___mb.mb_lock = godot::api->godot_method_bind_get_method("_Mutex", "lock");
	___mb.mb_try_lock = godot::api->godot_method_bind_get_method("_Mutex", "try_lock");
	___mb.mb_unlock = godot::api->godot_method_bind_get_method("_Mutex", "unlock");
}

Mutex *Mutex::_new()
{
	return (Mutex *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"_Mutex")());
}
void Mutex::lock() {
	___godot_icall_void(___mb.mb_lock, (const Object *) this);
}

Error Mutex::try_lock() {
	return (Error) ___godot_icall_int(___mb.mb_try_lock, (const Object *) this);
}

void Mutex::unlock() {
	___godot_icall_void(___mb.mb_unlock, (const Object *) this);
}

}