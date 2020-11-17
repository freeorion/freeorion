#include "VisualScriptFunctionState.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


VisualScriptFunctionState::___method_bindings VisualScriptFunctionState::___mb = {};

void VisualScriptFunctionState::___init_method_bindings() {
	___mb.mb__signal_callback = godot::api->godot_method_bind_get_method("VisualScriptFunctionState", "_signal_callback");
	___mb.mb_connect_to_signal = godot::api->godot_method_bind_get_method("VisualScriptFunctionState", "connect_to_signal");
	___mb.mb_is_valid = godot::api->godot_method_bind_get_method("VisualScriptFunctionState", "is_valid");
	___mb.mb_resume = godot::api->godot_method_bind_get_method("VisualScriptFunctionState", "resume");
}

VisualScriptFunctionState *VisualScriptFunctionState::_new()
{
	return (VisualScriptFunctionState *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptFunctionState")());
}
Variant VisualScriptFunctionState::_signal_callback(const Array& __var_args) {


	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 0));


	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 0] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb__signal_callback, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 0), nullptr);


	return __result;
}

void VisualScriptFunctionState::connect_to_signal(const Object *obj, const String signals, const Array args) {
	___godot_icall_void_Object_String_Array(___mb.mb_connect_to_signal, (const Object *) this, obj, signals, args);
}

bool VisualScriptFunctionState::is_valid() const {
	return ___godot_icall_bool(___mb.mb_is_valid, (const Object *) this);
}

Variant VisualScriptFunctionState::resume(const Array args) {
	return ___godot_icall_Variant_Array(___mb.mb_resume, (const Object *) this, args);
}

}