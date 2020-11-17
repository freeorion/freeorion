#include "ImmediateGeometry.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


ImmediateGeometry::___method_bindings ImmediateGeometry::___mb = {};

void ImmediateGeometry::___init_method_bindings() {
	___mb.mb_add_sphere = godot::api->godot_method_bind_get_method("ImmediateGeometry", "add_sphere");
	___mb.mb_add_vertex = godot::api->godot_method_bind_get_method("ImmediateGeometry", "add_vertex");
	___mb.mb_begin = godot::api->godot_method_bind_get_method("ImmediateGeometry", "begin");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("ImmediateGeometry", "clear");
	___mb.mb_end = godot::api->godot_method_bind_get_method("ImmediateGeometry", "end");
	___mb.mb_set_color = godot::api->godot_method_bind_get_method("ImmediateGeometry", "set_color");
	___mb.mb_set_normal = godot::api->godot_method_bind_get_method("ImmediateGeometry", "set_normal");
	___mb.mb_set_tangent = godot::api->godot_method_bind_get_method("ImmediateGeometry", "set_tangent");
	___mb.mb_set_uv = godot::api->godot_method_bind_get_method("ImmediateGeometry", "set_uv");
	___mb.mb_set_uv2 = godot::api->godot_method_bind_get_method("ImmediateGeometry", "set_uv2");
}

ImmediateGeometry *ImmediateGeometry::_new()
{
	return (ImmediateGeometry *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ImmediateGeometry")());
}
void ImmediateGeometry::add_sphere(const int64_t lats, const int64_t lons, const real_t radius, const bool add_uv) {
	___godot_icall_void_int_int_float_bool(___mb.mb_add_sphere, (const Object *) this, lats, lons, radius, add_uv);
}

void ImmediateGeometry::add_vertex(const Vector3 position) {
	___godot_icall_void_Vector3(___mb.mb_add_vertex, (const Object *) this, position);
}

void ImmediateGeometry::begin(const int64_t primitive, const Ref<Texture> texture) {
	___godot_icall_void_int_Object(___mb.mb_begin, (const Object *) this, primitive, texture.ptr());
}

void ImmediateGeometry::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void ImmediateGeometry::end() {
	___godot_icall_void(___mb.mb_end, (const Object *) this);
}

void ImmediateGeometry::set_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_color, (const Object *) this, color);
}

void ImmediateGeometry::set_normal(const Vector3 normal) {
	___godot_icall_void_Vector3(___mb.mb_set_normal, (const Object *) this, normal);
}

void ImmediateGeometry::set_tangent(const Plane tangent) {
	___godot_icall_void_Plane(___mb.mb_set_tangent, (const Object *) this, tangent);
}

void ImmediateGeometry::set_uv(const Vector2 uv) {
	___godot_icall_void_Vector2(___mb.mb_set_uv, (const Object *) this, uv);
}

void ImmediateGeometry::set_uv2(const Vector2 uv) {
	___godot_icall_void_Vector2(___mb.mb_set_uv2, (const Object *) this, uv);
}

}