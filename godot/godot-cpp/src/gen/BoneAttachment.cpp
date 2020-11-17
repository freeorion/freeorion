#include "BoneAttachment.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


BoneAttachment::___method_bindings BoneAttachment::___mb = {};

void BoneAttachment::___init_method_bindings() {
	___mb.mb_get_bone_name = godot::api->godot_method_bind_get_method("BoneAttachment", "get_bone_name");
	___mb.mb_set_bone_name = godot::api->godot_method_bind_get_method("BoneAttachment", "set_bone_name");
}

BoneAttachment *BoneAttachment::_new()
{
	return (BoneAttachment *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"BoneAttachment")());
}
String BoneAttachment::get_bone_name() const {
	return ___godot_icall_String(___mb.mb_get_bone_name, (const Object *) this);
}

void BoneAttachment::set_bone_name(const String bone_name) {
	___godot_icall_void_String(___mb.mb_set_bone_name, (const Object *) this, bone_name);
}

}