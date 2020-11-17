#include "VisualInstance.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualInstance::___method_bindings VisualInstance::___mb = {};

void VisualInstance::___init_method_bindings() {
	___mb.mb__get_visual_instance_rid = godot::api->godot_method_bind_get_method("VisualInstance", "_get_visual_instance_rid");
	___mb.mb_get_aabb = godot::api->godot_method_bind_get_method("VisualInstance", "get_aabb");
	___mb.mb_get_base = godot::api->godot_method_bind_get_method("VisualInstance", "get_base");
	___mb.mb_get_instance = godot::api->godot_method_bind_get_method("VisualInstance", "get_instance");
	___mb.mb_get_layer_mask = godot::api->godot_method_bind_get_method("VisualInstance", "get_layer_mask");
	___mb.mb_get_layer_mask_bit = godot::api->godot_method_bind_get_method("VisualInstance", "get_layer_mask_bit");
	___mb.mb_get_transformed_aabb = godot::api->godot_method_bind_get_method("VisualInstance", "get_transformed_aabb");
	___mb.mb_set_base = godot::api->godot_method_bind_get_method("VisualInstance", "set_base");
	___mb.mb_set_layer_mask = godot::api->godot_method_bind_get_method("VisualInstance", "set_layer_mask");
	___mb.mb_set_layer_mask_bit = godot::api->godot_method_bind_get_method("VisualInstance", "set_layer_mask_bit");
}

RID VisualInstance::_get_visual_instance_rid() const {
	return ___godot_icall_RID(___mb.mb__get_visual_instance_rid, (const Object *) this);
}

AABB VisualInstance::get_aabb() const {
	return ___godot_icall_AABB(___mb.mb_get_aabb, (const Object *) this);
}

RID VisualInstance::get_base() const {
	return ___godot_icall_RID(___mb.mb_get_base, (const Object *) this);
}

RID VisualInstance::get_instance() const {
	return ___godot_icall_RID(___mb.mb_get_instance, (const Object *) this);
}

int64_t VisualInstance::get_layer_mask() const {
	return ___godot_icall_int(___mb.mb_get_layer_mask, (const Object *) this);
}

bool VisualInstance::get_layer_mask_bit(const int64_t layer) const {
	return ___godot_icall_bool_int(___mb.mb_get_layer_mask_bit, (const Object *) this, layer);
}

AABB VisualInstance::get_transformed_aabb() const {
	return ___godot_icall_AABB(___mb.mb_get_transformed_aabb, (const Object *) this);
}

void VisualInstance::set_base(const RID base) {
	___godot_icall_void_RID(___mb.mb_set_base, (const Object *) this, base);
}

void VisualInstance::set_layer_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_layer_mask, (const Object *) this, mask);
}

void VisualInstance::set_layer_mask_bit(const int64_t layer, const bool enabled) {
	___godot_icall_void_int_bool(___mb.mb_set_layer_mask_bit, (const Object *) this, layer, enabled);
}

}