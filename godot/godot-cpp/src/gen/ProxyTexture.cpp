#include "ProxyTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


ProxyTexture::___method_bindings ProxyTexture::___mb = {};

void ProxyTexture::___init_method_bindings() {
	___mb.mb_get_base = godot::api->godot_method_bind_get_method("ProxyTexture", "get_base");
	___mb.mb_set_base = godot::api->godot_method_bind_get_method("ProxyTexture", "set_base");
}

ProxyTexture *ProxyTexture::_new()
{
	return (ProxyTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ProxyTexture")());
}
Ref<Texture> ProxyTexture::get_base() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_base, (const Object *) this));
}

void ProxyTexture::set_base(const Ref<Texture> base) {
	___godot_icall_void_Object(___mb.mb_set_base, (const Object *) this, base.ptr());
}

}