#include "GradientTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Gradient.hpp"


namespace godot {


GradientTexture::___method_bindings GradientTexture::___mb = {};

void GradientTexture::___init_method_bindings() {
	___mb.mb__update = godot::api->godot_method_bind_get_method("GradientTexture", "_update");
	___mb.mb_get_gradient = godot::api->godot_method_bind_get_method("GradientTexture", "get_gradient");
	___mb.mb_set_gradient = godot::api->godot_method_bind_get_method("GradientTexture", "set_gradient");
	___mb.mb_set_width = godot::api->godot_method_bind_get_method("GradientTexture", "set_width");
}

GradientTexture *GradientTexture::_new()
{
	return (GradientTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"GradientTexture")());
}
void GradientTexture::_update() {
	___godot_icall_void(___mb.mb__update, (const Object *) this);
}

Ref<Gradient> GradientTexture::get_gradient() const {
	return Ref<Gradient>::__internal_constructor(___godot_icall_Object(___mb.mb_get_gradient, (const Object *) this));
}

void GradientTexture::set_gradient(const Ref<Gradient> gradient) {
	___godot_icall_void_Object(___mb.mb_set_gradient, (const Object *) this, gradient.ptr());
}

void GradientTexture::set_width(const int64_t width) {
	___godot_icall_void_int(___mb.mb_set_width, (const Object *) this, width);
}

}