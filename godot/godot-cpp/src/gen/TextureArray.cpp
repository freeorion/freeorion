#include "TextureArray.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


TextureArray::___method_bindings TextureArray::___mb = {};

void TextureArray::___init_method_bindings() {
}

TextureArray *TextureArray::_new()
{
	return (TextureArray *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TextureArray")());
}
}