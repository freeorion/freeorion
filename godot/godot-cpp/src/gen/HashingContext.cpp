#include "HashingContext.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


HashingContext::___method_bindings HashingContext::___mb = {};

void HashingContext::___init_method_bindings() {
	___mb.mb_finish = godot::api->godot_method_bind_get_method("HashingContext", "finish");
	___mb.mb_start = godot::api->godot_method_bind_get_method("HashingContext", "start");
	___mb.mb_update = godot::api->godot_method_bind_get_method("HashingContext", "update");
}

HashingContext *HashingContext::_new()
{
	return (HashingContext *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"HashingContext")());
}
PoolByteArray HashingContext::finish() {
	return ___godot_icall_PoolByteArray(___mb.mb_finish, (const Object *) this);
}

Error HashingContext::start(const int64_t type) {
	return (Error) ___godot_icall_int_int(___mb.mb_start, (const Object *) this, type);
}

Error HashingContext::update(const PoolByteArray chunk) {
	return (Error) ___godot_icall_int_PoolByteArray(___mb.mb_update, (const Object *) this, chunk);
}

}