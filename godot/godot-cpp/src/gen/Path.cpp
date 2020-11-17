#include "Path.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Curve3D.hpp"


namespace godot {


Path::___method_bindings Path::___mb = {};

void Path::___init_method_bindings() {
	___mb.mb__curve_changed = godot::api->godot_method_bind_get_method("Path", "_curve_changed");
	___mb.mb_get_curve = godot::api->godot_method_bind_get_method("Path", "get_curve");
	___mb.mb_set_curve = godot::api->godot_method_bind_get_method("Path", "set_curve");
}

Path *Path::_new()
{
	return (Path *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Path")());
}
void Path::_curve_changed() {
	___godot_icall_void(___mb.mb__curve_changed, (const Object *) this);
}

Ref<Curve3D> Path::get_curve() const {
	return Ref<Curve3D>::__internal_constructor(___godot_icall_Object(___mb.mb_get_curve, (const Object *) this));
}

void Path::set_curve(const Ref<Curve3D> curve) {
	___godot_icall_void_Object(___mb.mb_set_curve, (const Object *) this, curve.ptr());
}

}