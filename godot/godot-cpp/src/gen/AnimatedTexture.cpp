#include "AnimatedTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


AnimatedTexture::___method_bindings AnimatedTexture::___mb = {};

void AnimatedTexture::___init_method_bindings() {
	___mb.mb__update_proxy = godot::api->godot_method_bind_get_method("AnimatedTexture", "_update_proxy");
	___mb.mb_get_fps = godot::api->godot_method_bind_get_method("AnimatedTexture", "get_fps");
	___mb.mb_get_frame_delay = godot::api->godot_method_bind_get_method("AnimatedTexture", "get_frame_delay");
	___mb.mb_get_frame_texture = godot::api->godot_method_bind_get_method("AnimatedTexture", "get_frame_texture");
	___mb.mb_get_frames = godot::api->godot_method_bind_get_method("AnimatedTexture", "get_frames");
	___mb.mb_set_fps = godot::api->godot_method_bind_get_method("AnimatedTexture", "set_fps");
	___mb.mb_set_frame_delay = godot::api->godot_method_bind_get_method("AnimatedTexture", "set_frame_delay");
	___mb.mb_set_frame_texture = godot::api->godot_method_bind_get_method("AnimatedTexture", "set_frame_texture");
	___mb.mb_set_frames = godot::api->godot_method_bind_get_method("AnimatedTexture", "set_frames");
}

AnimatedTexture *AnimatedTexture::_new()
{
	return (AnimatedTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimatedTexture")());
}
void AnimatedTexture::_update_proxy() {
	___godot_icall_void(___mb.mb__update_proxy, (const Object *) this);
}

real_t AnimatedTexture::get_fps() const {
	return ___godot_icall_float(___mb.mb_get_fps, (const Object *) this);
}

real_t AnimatedTexture::get_frame_delay(const int64_t frame) const {
	return ___godot_icall_float_int(___mb.mb_get_frame_delay, (const Object *) this, frame);
}

Ref<Texture> AnimatedTexture::get_frame_texture(const int64_t frame) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_frame_texture, (const Object *) this, frame));
}

int64_t AnimatedTexture::get_frames() const {
	return ___godot_icall_int(___mb.mb_get_frames, (const Object *) this);
}

void AnimatedTexture::set_fps(const real_t fps) {
	___godot_icall_void_float(___mb.mb_set_fps, (const Object *) this, fps);
}

void AnimatedTexture::set_frame_delay(const int64_t frame, const real_t delay) {
	___godot_icall_void_int_float(___mb.mb_set_frame_delay, (const Object *) this, frame, delay);
}

void AnimatedTexture::set_frame_texture(const int64_t frame, const Ref<Texture> texture) {
	___godot_icall_void_int_Object(___mb.mb_set_frame_texture, (const Object *) this, frame, texture.ptr());
}

void AnimatedTexture::set_frames(const int64_t frames) {
	___godot_icall_void_int(___mb.mb_set_frames, (const Object *) this, frames);
}

}