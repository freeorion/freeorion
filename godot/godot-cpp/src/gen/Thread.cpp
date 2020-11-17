#include "Thread.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


Thread::___method_bindings Thread::___mb = {};

void Thread::___init_method_bindings() {
	___mb.mb_get_id = godot::api->godot_method_bind_get_method("_Thread", "get_id");
	___mb.mb_is_active = godot::api->godot_method_bind_get_method("_Thread", "is_active");
	___mb.mb_start = godot::api->godot_method_bind_get_method("_Thread", "start");
	___mb.mb_wait_to_finish = godot::api->godot_method_bind_get_method("_Thread", "wait_to_finish");
}

Thread *Thread::_new()
{
	return (Thread *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"_Thread")());
}
String Thread::get_id() const {
	return ___godot_icall_String(___mb.mb_get_id, (const Object *) this);
}

bool Thread::is_active() const {
	return ___godot_icall_bool(___mb.mb_is_active, (const Object *) this);
}

Error Thread::start(const Object *instance, const String method, const Variant userdata, const int64_t priority) {
	return (Error) ___godot_icall_int_Object_String_Variant_int(___mb.mb_start, (const Object *) this, instance, method, userdata, priority);
}

Variant Thread::wait_to_finish() {
	return ___godot_icall_Variant(___mb.mb_wait_to_finish, (const Object *) this);
}

}