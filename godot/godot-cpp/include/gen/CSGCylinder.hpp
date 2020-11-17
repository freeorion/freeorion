#ifndef GODOT_CPP_CSGCYLINDER_HPP
#define GODOT_CPP_CSGCYLINDER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "CSGPrimitive.hpp"
namespace godot {

class Material;

class CSGCylinder : public CSGPrimitive {
	struct ___method_bindings {
		godot_method_bind *mb_get_height;
		godot_method_bind *mb_get_material;
		godot_method_bind *mb_get_radius;
		godot_method_bind *mb_get_sides;
		godot_method_bind *mb_get_smooth_faces;
		godot_method_bind *mb_is_cone;
		godot_method_bind *mb_set_cone;
		godot_method_bind *mb_set_height;
		godot_method_bind *mb_set_material;
		godot_method_bind *mb_set_radius;
		godot_method_bind *mb_set_sides;
		godot_method_bind *mb_set_smooth_faces;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CSGCylinder"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CSGCylinder *_new();

	// methods
	real_t get_height() const;
	Ref<Material> get_material() const;
	real_t get_radius() const;
	int64_t get_sides() const;
	bool get_smooth_faces() const;
	bool is_cone() const;
	void set_cone(const bool cone);
	void set_height(const real_t height);
	void set_material(const Ref<Material> material);
	void set_radius(const real_t radius);
	void set_sides(const int64_t sides);
	void set_smooth_faces(const bool smooth_faces);

};

}

#endif