#include "MultiMeshInstance.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "MultiMesh.hpp"


namespace godot {


MultiMeshInstance::___method_bindings MultiMeshInstance::___mb = {};

void MultiMeshInstance::___init_method_bindings() {
	___mb.mb_get_multimesh = godot::api->godot_method_bind_get_method("MultiMeshInstance", "get_multimesh");
	___mb.mb_set_multimesh = godot::api->godot_method_bind_get_method("MultiMeshInstance", "set_multimesh");
}

MultiMeshInstance *MultiMeshInstance::_new()
{
	return (MultiMeshInstance *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MultiMeshInstance")());
}
Ref<MultiMesh> MultiMeshInstance::get_multimesh() const {
	return Ref<MultiMesh>::__internal_constructor(___godot_icall_Object(___mb.mb_get_multimesh, (const Object *) this));
}

void MultiMeshInstance::set_multimesh(const Ref<MultiMesh> multimesh) {
	___godot_icall_void_Object(___mb.mb_set_multimesh, (const Object *) this, multimesh.ptr());
}

}