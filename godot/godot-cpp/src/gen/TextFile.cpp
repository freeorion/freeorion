#include "TextFile.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


TextFile::___method_bindings TextFile::___mb = {};

void TextFile::___init_method_bindings() {
}

TextFile *TextFile::_new()
{
	return (TextFile *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TextFile")());
}
}