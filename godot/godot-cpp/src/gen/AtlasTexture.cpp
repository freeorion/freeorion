#include "AtlasTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


AtlasTexture::___method_bindings AtlasTexture::___mb = {};

void AtlasTexture::___init_method_bindings() {
	___mb.mb_get_atlas = godot::api->godot_method_bind_get_method("AtlasTexture", "get_atlas");
	___mb.mb_get_margin = godot::api->godot_method_bind_get_method("AtlasTexture", "get_margin");
	___mb.mb_get_region = godot::api->godot_method_bind_get_method("AtlasTexture", "get_region");
	___mb.mb_has_filter_clip = godot::api->godot_method_bind_get_method("AtlasTexture", "has_filter_clip");
	___mb.mb_set_atlas = godot::api->godot_method_bind_get_method("AtlasTexture", "set_atlas");
	___mb.mb_set_filter_clip = godot::api->godot_method_bind_get_method("AtlasTexture", "set_filter_clip");
	___mb.mb_set_margin = godot::api->godot_method_bind_get_method("AtlasTexture", "set_margin");
	___mb.mb_set_region = godot::api->godot_method_bind_get_method("AtlasTexture", "set_region");
}

AtlasTexture *AtlasTexture::_new()
{
	return (AtlasTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AtlasTexture")());
}
Ref<Texture> AtlasTexture::get_atlas() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_atlas, (const Object *) this));
}

Rect2 AtlasTexture::get_margin() const {
	return ___godot_icall_Rect2(___mb.mb_get_margin, (const Object *) this);
}

Rect2 AtlasTexture::get_region() const {
	return ___godot_icall_Rect2(___mb.mb_get_region, (const Object *) this);
}

bool AtlasTexture::has_filter_clip() const {
	return ___godot_icall_bool(___mb.mb_has_filter_clip, (const Object *) this);
}

void AtlasTexture::set_atlas(const Ref<Texture> atlas) {
	___godot_icall_void_Object(___mb.mb_set_atlas, (const Object *) this, atlas.ptr());
}

void AtlasTexture::set_filter_clip(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_filter_clip, (const Object *) this, enable);
}

void AtlasTexture::set_margin(const Rect2 margin) {
	___godot_icall_void_Rect2(___mb.mb_set_margin, (const Object *) this, margin);
}

void AtlasTexture::set_region(const Rect2 region) {
	___godot_icall_void_Rect2(___mb.mb_set_region, (const Object *) this, region);
}

}