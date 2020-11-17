#include "SegmentShape2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


SegmentShape2D::___method_bindings SegmentShape2D::___mb = {};

void SegmentShape2D::___init_method_bindings() {
	___mb.mb_get_a = godot::api->godot_method_bind_get_method("SegmentShape2D", "get_a");
	___mb.mb_get_b = godot::api->godot_method_bind_get_method("SegmentShape2D", "get_b");
	___mb.mb_set_a = godot::api->godot_method_bind_get_method("SegmentShape2D", "set_a");
	___mb.mb_set_b = godot::api->godot_method_bind_get_method("SegmentShape2D", "set_b");
}

SegmentShape2D *SegmentShape2D::_new()
{
	return (SegmentShape2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SegmentShape2D")());
}
Vector2 SegmentShape2D::get_a() const {
	return ___godot_icall_Vector2(___mb.mb_get_a, (const Object *) this);
}

Vector2 SegmentShape2D::get_b() const {
	return ___godot_icall_Vector2(___mb.mb_get_b, (const Object *) this);
}

void SegmentShape2D::set_a(const Vector2 a) {
	___godot_icall_void_Vector2(___mb.mb_set_a, (const Object *) this, a);
}

void SegmentShape2D::set_b(const Vector2 b) {
	___godot_icall_void_Vector2(___mb.mb_set_b, (const Object *) this, b);
}

}