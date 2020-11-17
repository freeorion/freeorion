#include "NoiseTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Image.hpp"
#include "OpenSimplexNoise.hpp"


namespace godot {


NoiseTexture::___method_bindings NoiseTexture::___mb = {};

void NoiseTexture::___init_method_bindings() {
	___mb.mb__generate_texture = godot::api->godot_method_bind_get_method("NoiseTexture", "_generate_texture");
	___mb.mb__queue_update = godot::api->godot_method_bind_get_method("NoiseTexture", "_queue_update");
	___mb.mb__thread_done = godot::api->godot_method_bind_get_method("NoiseTexture", "_thread_done");
	___mb.mb__update_texture = godot::api->godot_method_bind_get_method("NoiseTexture", "_update_texture");
	___mb.mb_get_bump_strength = godot::api->godot_method_bind_get_method("NoiseTexture", "get_bump_strength");
	___mb.mb_get_noise = godot::api->godot_method_bind_get_method("NoiseTexture", "get_noise");
	___mb.mb_get_seamless = godot::api->godot_method_bind_get_method("NoiseTexture", "get_seamless");
	___mb.mb_is_normalmap = godot::api->godot_method_bind_get_method("NoiseTexture", "is_normalmap");
	___mb.mb_set_as_normalmap = godot::api->godot_method_bind_get_method("NoiseTexture", "set_as_normalmap");
	___mb.mb_set_bump_strength = godot::api->godot_method_bind_get_method("NoiseTexture", "set_bump_strength");
	___mb.mb_set_height = godot::api->godot_method_bind_get_method("NoiseTexture", "set_height");
	___mb.mb_set_noise = godot::api->godot_method_bind_get_method("NoiseTexture", "set_noise");
	___mb.mb_set_seamless = godot::api->godot_method_bind_get_method("NoiseTexture", "set_seamless");
	___mb.mb_set_width = godot::api->godot_method_bind_get_method("NoiseTexture", "set_width");
}

NoiseTexture *NoiseTexture::_new()
{
	return (NoiseTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"NoiseTexture")());
}
Ref<Image> NoiseTexture::_generate_texture() {
	return Ref<Image>::__internal_constructor(___godot_icall_Object(___mb.mb__generate_texture, (const Object *) this));
}

void NoiseTexture::_queue_update() {
	___godot_icall_void(___mb.mb__queue_update, (const Object *) this);
}

void NoiseTexture::_thread_done(const Ref<Image> image) {
	___godot_icall_void_Object(___mb.mb__thread_done, (const Object *) this, image.ptr());
}

void NoiseTexture::_update_texture() {
	___godot_icall_void(___mb.mb__update_texture, (const Object *) this);
}

real_t NoiseTexture::get_bump_strength() {
	return ___godot_icall_float(___mb.mb_get_bump_strength, (const Object *) this);
}

Ref<OpenSimplexNoise> NoiseTexture::get_noise() {
	return Ref<OpenSimplexNoise>::__internal_constructor(___godot_icall_Object(___mb.mb_get_noise, (const Object *) this));
}

bool NoiseTexture::get_seamless() {
	return ___godot_icall_bool(___mb.mb_get_seamless, (const Object *) this);
}

bool NoiseTexture::is_normalmap() {
	return ___godot_icall_bool(___mb.mb_is_normalmap, (const Object *) this);
}

void NoiseTexture::set_as_normalmap(const bool as_normalmap) {
	___godot_icall_void_bool(___mb.mb_set_as_normalmap, (const Object *) this, as_normalmap);
}

void NoiseTexture::set_bump_strength(const real_t bump_strength) {
	___godot_icall_void_float(___mb.mb_set_bump_strength, (const Object *) this, bump_strength);
}

void NoiseTexture::set_height(const int64_t height) {
	___godot_icall_void_int(___mb.mb_set_height, (const Object *) this, height);
}

void NoiseTexture::set_noise(const Ref<OpenSimplexNoise> noise) {
	___godot_icall_void_Object(___mb.mb_set_noise, (const Object *) this, noise.ptr());
}

void NoiseTexture::set_seamless(const bool seamless) {
	___godot_icall_void_bool(___mb.mb_set_seamless, (const Object *) this, seamless);
}

void NoiseTexture::set_width(const int64_t width) {
	___godot_icall_void_int(___mb.mb_set_width, (const Object *) this, width);
}

}