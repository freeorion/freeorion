#include "CurveTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Curve.hpp"


namespace godot {


CurveTexture::___method_bindings CurveTexture::___mb = {};

void CurveTexture::___init_method_bindings() {
	___mb.mb__update = godot::api->godot_method_bind_get_method("CurveTexture", "_update");
	___mb.mb_get_curve = godot::api->godot_method_bind_get_method("CurveTexture", "get_curve");
	___mb.mb_set_curve = godot::api->godot_method_bind_get_method("CurveTexture", "set_curve");
	___mb.mb_set_width = godot::api->godot_method_bind_get_method("CurveTexture", "set_width");
}

CurveTexture *CurveTexture::_new()
{
	return (CurveTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CurveTexture")());
}
void CurveTexture::_update() {
	___godot_icall_void(___mb.mb__update, (const Object *) this);
}

Ref<Curve> CurveTexture::get_curve() const {
	return Ref<Curve>::__internal_constructor(___godot_icall_Object(___mb.mb_get_curve, (const Object *) this));
}

void CurveTexture::set_curve(const Ref<Curve> curve) {
	___godot_icall_void_Object(___mb.mb_set_curve, (const Object *) this, curve.ptr());
}

void CurveTexture::set_width(const int64_t width) {
	___godot_icall_void_int(___mb.mb_set_width, (const Object *) this, width);
}

}