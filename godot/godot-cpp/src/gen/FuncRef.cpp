#include "FuncRef.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


FuncRef::___method_bindings FuncRef::___mb = {};

void FuncRef::___init_method_bindings() {
	___mb.mb_call_func = godot::api->godot_method_bind_get_method("FuncRef", "call_func");
	___mb.mb_call_funcv = godot::api->godot_method_bind_get_method("FuncRef", "call_funcv");
	___mb.mb_is_valid = godot::api->godot_method_bind_get_method("FuncRef", "is_valid");
	___mb.mb_set_function = godot::api->godot_method_bind_get_method("FuncRef", "set_function");
	___mb.mb_set_instance = godot::api->godot_method_bind_get_method("FuncRef", "set_instance");
}

FuncRef *FuncRef::_new()
{
	return (FuncRef *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"FuncRef")());
}
Variant FuncRef::call_func(const Array& __var_args) {


	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 0));


	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 0] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_call_func, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 0), nullptr);


	return __result;
}

Variant FuncRef::call_funcv(const Array arg_array) {
	return ___godot_icall_Variant_Array(___mb.mb_call_funcv, (const Object *) this, arg_array);
}

bool FuncRef::is_valid() const {
	return ___godot_icall_bool(___mb.mb_is_valid, (const Object *) this);
}

void FuncRef::set_function(const String name) {
	___godot_icall_void_String(___mb.mb_set_function, (const Object *) this, name);
}

void FuncRef::set_instance(const Object *instance) {
	___godot_icall_void_Object(___mb.mb_set_instance, (const Object *) this, instance);
}

}