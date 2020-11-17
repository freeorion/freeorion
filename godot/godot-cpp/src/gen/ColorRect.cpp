#include "ColorRect.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ColorRect::___method_bindings ColorRect::___mb = {};

void ColorRect::___init_method_bindings() {
	___mb.mb_get_frame_color = godot::api->godot_method_bind_get_method("ColorRect", "get_frame_color");
	___mb.mb_set_frame_color = godot::api->godot_method_bind_get_method("ColorRect", "set_frame_color");
}

ColorRect *ColorRect::_new()
{
	return (ColorRect *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ColorRect")());
}
Color ColorRect::get_frame_color() const {
	return ___godot_icall_Color(___mb.mb_get_frame_color, (const Object *) this);
}

void ColorRect::set_frame_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_frame_color, (const Object *) this, color);
}

}