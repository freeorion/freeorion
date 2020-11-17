#include "BackBufferCopy.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


BackBufferCopy::___method_bindings BackBufferCopy::___mb = {};

void BackBufferCopy::___init_method_bindings() {
	___mb.mb_get_copy_mode = godot::api->godot_method_bind_get_method("BackBufferCopy", "get_copy_mode");
	___mb.mb_get_rect = godot::api->godot_method_bind_get_method("BackBufferCopy", "get_rect");
	___mb.mb_set_copy_mode = godot::api->godot_method_bind_get_method("BackBufferCopy", "set_copy_mode");
	___mb.mb_set_rect = godot::api->godot_method_bind_get_method("BackBufferCopy", "set_rect");
}

BackBufferCopy *BackBufferCopy::_new()
{
	return (BackBufferCopy *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"BackBufferCopy")());
}
BackBufferCopy::CopyMode BackBufferCopy::get_copy_mode() const {
	return (BackBufferCopy::CopyMode) ___godot_icall_int(___mb.mb_get_copy_mode, (const Object *) this);
}

Rect2 BackBufferCopy::get_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_rect, (const Object *) this);
}

void BackBufferCopy::set_copy_mode(const int64_t copy_mode) {
	___godot_icall_void_int(___mb.mb_set_copy_mode, (const Object *) this, copy_mode);
}

void BackBufferCopy::set_rect(const Rect2 rect) {
	___godot_icall_void_Rect2(___mb.mb_set_rect, (const Object *) this, rect);
}

}