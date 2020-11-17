#include "JavaScript.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


JavaScript *JavaScript::_singleton = NULL;


JavaScript::JavaScript() {
	_owner = godot::api->godot_global_get_singleton((char *) "JavaScript");
}


JavaScript::___method_bindings JavaScript::___mb = {};

void JavaScript::___init_method_bindings() {
	___mb.mb_eval = godot::api->godot_method_bind_get_method("JavaScript", "eval");
}

Variant JavaScript::eval(const String code, const bool use_global_execution_context) {
	return ___godot_icall_Variant_String_bool(___mb.mb_eval, (const Object *) this, code, use_global_execution_context);
}

}