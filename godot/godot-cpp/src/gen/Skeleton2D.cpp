#include "Skeleton2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Bone2D.hpp"


namespace godot {


Skeleton2D::___method_bindings Skeleton2D::___mb = {};

void Skeleton2D::___init_method_bindings() {
	___mb.mb__update_bone_setup = godot::api->godot_method_bind_get_method("Skeleton2D", "_update_bone_setup");
	___mb.mb__update_transform = godot::api->godot_method_bind_get_method("Skeleton2D", "_update_transform");
	___mb.mb_get_bone = godot::api->godot_method_bind_get_method("Skeleton2D", "get_bone");
	___mb.mb_get_bone_count = godot::api->godot_method_bind_get_method("Skeleton2D", "get_bone_count");
	___mb.mb_get_skeleton = godot::api->godot_method_bind_get_method("Skeleton2D", "get_skeleton");
}

Skeleton2D *Skeleton2D::_new()
{
	return (Skeleton2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Skeleton2D")());
}
void Skeleton2D::_update_bone_setup() {
	___godot_icall_void(___mb.mb__update_bone_setup, (const Object *) this);
}

void Skeleton2D::_update_transform() {
	___godot_icall_void(___mb.mb__update_transform, (const Object *) this);
}

Bone2D *Skeleton2D::get_bone(const int64_t idx) {
	return (Bone2D *) ___godot_icall_Object_int(___mb.mb_get_bone, (const Object *) this, idx);
}

int64_t Skeleton2D::get_bone_count() const {
	return ___godot_icall_int(___mb.mb_get_bone_count, (const Object *) this);
}

RID Skeleton2D::get_skeleton() const {
	return ___godot_icall_RID(___mb.mb_get_skeleton, (const Object *) this);
}

}