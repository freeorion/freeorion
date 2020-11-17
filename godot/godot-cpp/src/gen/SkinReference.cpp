#include "SkinReference.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Skin.hpp"


namespace godot {


SkinReference::___method_bindings SkinReference::___mb = {};

void SkinReference::___init_method_bindings() {
	___mb.mb__skin_changed = godot::api->godot_method_bind_get_method("SkinReference", "_skin_changed");
	___mb.mb_get_skeleton = godot::api->godot_method_bind_get_method("SkinReference", "get_skeleton");
	___mb.mb_get_skin = godot::api->godot_method_bind_get_method("SkinReference", "get_skin");
}

void SkinReference::_skin_changed() {
	___godot_icall_void(___mb.mb__skin_changed, (const Object *) this);
}

RID SkinReference::get_skeleton() const {
	return ___godot_icall_RID(___mb.mb_get_skeleton, (const Object *) this);
}

Ref<Skin> SkinReference::get_skin() const {
	return Ref<Skin>::__internal_constructor(___godot_icall_Object(___mb.mb_get_skin, (const Object *) this));
}

}