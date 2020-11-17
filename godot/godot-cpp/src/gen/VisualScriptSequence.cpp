#include "VisualScriptSequence.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptSequence::___method_bindings VisualScriptSequence::___mb = {};

void VisualScriptSequence::___init_method_bindings() {
	___mb.mb_get_steps = godot::api->godot_method_bind_get_method("VisualScriptSequence", "get_steps");
	___mb.mb_set_steps = godot::api->godot_method_bind_get_method("VisualScriptSequence", "set_steps");
}

VisualScriptSequence *VisualScriptSequence::_new()
{
	return (VisualScriptSequence *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptSequence")());
}
int64_t VisualScriptSequence::get_steps() const {
	return ___godot_icall_int(___mb.mb_get_steps, (const Object *) this);
}

void VisualScriptSequence::set_steps(const int64_t steps) {
	___godot_icall_void_int(___mb.mb_set_steps, (const Object *) this, steps);
}

}