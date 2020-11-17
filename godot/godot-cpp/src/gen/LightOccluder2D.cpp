#include "LightOccluder2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "OccluderPolygon2D.hpp"


namespace godot {


LightOccluder2D::___method_bindings LightOccluder2D::___mb = {};

void LightOccluder2D::___init_method_bindings() {
	___mb.mb__poly_changed = godot::api->godot_method_bind_get_method("LightOccluder2D", "_poly_changed");
	___mb.mb_get_occluder_light_mask = godot::api->godot_method_bind_get_method("LightOccluder2D", "get_occluder_light_mask");
	___mb.mb_get_occluder_polygon = godot::api->godot_method_bind_get_method("LightOccluder2D", "get_occluder_polygon");
	___mb.mb_set_occluder_light_mask = godot::api->godot_method_bind_get_method("LightOccluder2D", "set_occluder_light_mask");
	___mb.mb_set_occluder_polygon = godot::api->godot_method_bind_get_method("LightOccluder2D", "set_occluder_polygon");
}

LightOccluder2D *LightOccluder2D::_new()
{
	return (LightOccluder2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"LightOccluder2D")());
}
void LightOccluder2D::_poly_changed() {
	___godot_icall_void(___mb.mb__poly_changed, (const Object *) this);
}

int64_t LightOccluder2D::get_occluder_light_mask() const {
	return ___godot_icall_int(___mb.mb_get_occluder_light_mask, (const Object *) this);
}

Ref<OccluderPolygon2D> LightOccluder2D::get_occluder_polygon() const {
	return Ref<OccluderPolygon2D>::__internal_constructor(___godot_icall_Object(___mb.mb_get_occluder_polygon, (const Object *) this));
}

void LightOccluder2D::set_occluder_light_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_occluder_light_mask, (const Object *) this, mask);
}

void LightOccluder2D::set_occluder_polygon(const Ref<OccluderPolygon2D> polygon) {
	___godot_icall_void_Object(___mb.mb_set_occluder_polygon, (const Object *) this, polygon.ptr());
}

}