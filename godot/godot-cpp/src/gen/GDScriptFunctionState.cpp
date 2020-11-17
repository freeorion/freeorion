#include "GDScriptFunctionState.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


GDScriptFunctionState::___method_bindings GDScriptFunctionState::___mb = {};

void GDScriptFunctionState::___init_method_bindings() {
	___mb.mb__signal_callback = godot::api->godot_method_bind_get_method("GDScriptFunctionState", "_signal_callback");
	___mb.mb_is_valid = godot::api->godot_method_bind_get_method("GDScriptFunctionState", "is_valid");
	___mb.mb_resume = godot::api->godot_method_bind_get_method("GDScriptFunctionState", "resume");
}

Variant GDScriptFunctionState::_signal_callback(const Array& __var_args) {


	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 0));


	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 0] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb__signal_callback, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 0), nullptr);


	return __result;
}

bool GDScriptFunctionState::is_valid(const bool extended_check) const {
	return ___godot_icall_bool_bool(___mb.mb_is_valid, (const Object *) this, extended_check);
}

Variant GDScriptFunctionState::resume(const Variant arg) {
	return ___godot_icall_Variant_Variant(___mb.mb_resume, (const Object *) this, arg);
}

}