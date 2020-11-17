#include "PluginScript.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PluginScript::___method_bindings PluginScript::___mb = {};

void PluginScript::___init_method_bindings() {
	___mb.mb_new = godot::api->godot_method_bind_get_method("PluginScript", "new");
}

PluginScript *PluginScript::_new()
{
	return (PluginScript *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PluginScript")());
}
Variant PluginScript::new_(const Array& __var_args) {


	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 0));


	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 0] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_new, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 0), nullptr);


	return __result;
}

}