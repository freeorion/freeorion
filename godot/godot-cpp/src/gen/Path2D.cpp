#include "Path2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Curve2D.hpp"


namespace godot {


Path2D::___method_bindings Path2D::___mb = {};

void Path2D::___init_method_bindings() {
	___mb.mb__curve_changed = godot::api->godot_method_bind_get_method("Path2D", "_curve_changed");
	___mb.mb_get_curve = godot::api->godot_method_bind_get_method("Path2D", "get_curve");
	___mb.mb_set_curve = godot::api->godot_method_bind_get_method("Path2D", "set_curve");
}

Path2D *Path2D::_new()
{
	return (Path2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Path2D")());
}
void Path2D::_curve_changed() {
	___godot_icall_void(___mb.mb__curve_changed, (const Object *) this);
}

Ref<Curve2D> Path2D::get_curve() const {
	return Ref<Curve2D>::__internal_constructor(___godot_icall_Object(___mb.mb_get_curve, (const Object *) this));
}

void Path2D::set_curve(const Ref<Curve2D> curve) {
	___godot_icall_void_Object(___mb.mb_set_curve, (const Object *) this, curve.ptr());
}

}