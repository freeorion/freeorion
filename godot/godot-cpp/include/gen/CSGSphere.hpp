#ifndef GODOT_CPP_CSGSPHERE_HPP
#define GODOT_CPP_CSGSPHERE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "CSGPrimitive.hpp"
namespace godot {

class Material;

class CSGSphere : public CSGPrimitive {
	struct ___method_bindings {
		godot_method_bind *mb_get_material;
		godot_method_bind *mb_get_radial_segments;
		godot_method_bind *mb_get_radius;
		godot_method_bind *mb_get_rings;
		godot_method_bind *mb_get_smooth_faces;
		godot_method_bind *mb_set_material;
		godot_method_bind *mb_set_radial_segments;
		godot_method_bind *mb_set_radius;
		godot_method_bind *mb_set_rings;
		godot_method_bind *mb_set_smooth_faces;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CSGSphere"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CSGSphere *_new();

	// methods
	Ref<Material> get_material() const;
	int64_t get_radial_segments() const;
	real_t get_radius() const;
	int64_t get_rings() const;
	bool get_smooth_faces() const;
	void set_material(const Ref<Material> material);
	void set_radial_segments(const int64_t radial_segments);
	void set_radius(const real_t radius);
	void set_rings(const int64_t rings);
	void set_smooth_faces(const bool smooth_faces);

};

}

#endif