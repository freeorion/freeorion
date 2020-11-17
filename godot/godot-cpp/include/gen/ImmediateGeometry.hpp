#ifndef GODOT_CPP_IMMEDIATEGEOMETRY_HPP
#define GODOT_CPP_IMMEDIATEGEOMETRY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "GeometryInstance.hpp"
namespace godot {

class Texture;

class ImmediateGeometry : public GeometryInstance {
	struct ___method_bindings {
		godot_method_bind *mb_add_sphere;
		godot_method_bind *mb_add_vertex;
		godot_method_bind *mb_begin;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_end;
		godot_method_bind *mb_set_color;
		godot_method_bind *mb_set_normal;
		godot_method_bind *mb_set_tangent;
		godot_method_bind *mb_set_uv;
		godot_method_bind *mb_set_uv2;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ImmediateGeometry"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ImmediateGeometry *_new();

	// methods
	void add_sphere(const int64_t lats, const int64_t lons, const real_t radius, const bool add_uv = true);
	void add_vertex(const Vector3 position);
	void begin(const int64_t primitive, const Ref<Texture> texture = nullptr);
	void clear();
	void end();
	void set_color(const Color color);
	void set_normal(const Vector3 normal);
	void set_tangent(const Plane tangent);
	void set_uv(const Vector2 uv);
	void set_uv2(const Vector2 uv);

};

}

#endif