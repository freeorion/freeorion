#ifndef GODOT_CPP_CSGPOLYGON_HPP
#define GODOT_CPP_CSGPOLYGON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "CSGPolygon.hpp"

#include "CSGPrimitive.hpp"
namespace godot {

class Material;

class CSGPolygon : public CSGPrimitive {
	struct ___method_bindings {
		godot_method_bind *mb__has_editable_3d_polygon_no_depth;
		godot_method_bind *mb__is_editable_3d_polygon;
		godot_method_bind *mb__path_changed;
		godot_method_bind *mb__path_exited;
		godot_method_bind *mb_get_depth;
		godot_method_bind *mb_get_material;
		godot_method_bind *mb_get_mode;
		godot_method_bind *mb_get_path_interval;
		godot_method_bind *mb_get_path_node;
		godot_method_bind *mb_get_path_rotation;
		godot_method_bind *mb_get_polygon;
		godot_method_bind *mb_get_smooth_faces;
		godot_method_bind *mb_get_spin_degrees;
		godot_method_bind *mb_get_spin_sides;
		godot_method_bind *mb_is_path_continuous_u;
		godot_method_bind *mb_is_path_joined;
		godot_method_bind *mb_is_path_local;
		godot_method_bind *mb_set_depth;
		godot_method_bind *mb_set_material;
		godot_method_bind *mb_set_mode;
		godot_method_bind *mb_set_path_continuous_u;
		godot_method_bind *mb_set_path_interval;
		godot_method_bind *mb_set_path_joined;
		godot_method_bind *mb_set_path_local;
		godot_method_bind *mb_set_path_node;
		godot_method_bind *mb_set_path_rotation;
		godot_method_bind *mb_set_polygon;
		godot_method_bind *mb_set_smooth_faces;
		godot_method_bind *mb_set_spin_degrees;
		godot_method_bind *mb_set_spin_sides;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CSGPolygon"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum PathRotation {
		PATH_ROTATION_POLYGON = 0,
		PATH_ROTATION_PATH = 1,
		PATH_ROTATION_PATH_FOLLOW = 2,
	};
	enum Mode {
		MODE_DEPTH = 0,
		MODE_SPIN = 1,
		MODE_PATH = 2,
	};

	// constants


	static CSGPolygon *_new();

	// methods
	bool _has_editable_3d_polygon_no_depth() const;
	bool _is_editable_3d_polygon() const;
	void _path_changed();
	void _path_exited();
	real_t get_depth() const;
	Ref<Material> get_material() const;
	CSGPolygon::Mode get_mode() const;
	real_t get_path_interval() const;
	NodePath get_path_node() const;
	CSGPolygon::PathRotation get_path_rotation() const;
	PoolVector2Array get_polygon() const;
	bool get_smooth_faces() const;
	real_t get_spin_degrees() const;
	int64_t get_spin_sides() const;
	bool is_path_continuous_u() const;
	bool is_path_joined() const;
	bool is_path_local() const;
	void set_depth(const real_t depth);
	void set_material(const Ref<Material> material);
	void set_mode(const int64_t mode);
	void set_path_continuous_u(const bool enable);
	void set_path_interval(const real_t distance);
	void set_path_joined(const bool enable);
	void set_path_local(const bool enable);
	void set_path_node(const NodePath path);
	void set_path_rotation(const int64_t mode);
	void set_polygon(const PoolVector2Array polygon);
	void set_smooth_faces(const bool smooth_faces);
	void set_spin_degrees(const real_t degrees);
	void set_spin_sides(const int64_t spin_sides);

};

}

#endif