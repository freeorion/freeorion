#ifndef GODOT_CPP_OCCLUDERPOLYGON2D_HPP
#define GODOT_CPP_OCCLUDERPOLYGON2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "OccluderPolygon2D.hpp"

#include "Resource.hpp"
namespace godot {


class OccluderPolygon2D : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_get_cull_mode;
		godot_method_bind *mb_get_polygon;
		godot_method_bind *mb_is_closed;
		godot_method_bind *mb_set_closed;
		godot_method_bind *mb_set_cull_mode;
		godot_method_bind *mb_set_polygon;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "OccluderPolygon2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum CullMode {
		CULL_DISABLED = 0,
		CULL_CLOCKWISE = 1,
		CULL_COUNTER_CLOCKWISE = 2,
	};

	// constants


	static OccluderPolygon2D *_new();

	// methods
	OccluderPolygon2D::CullMode get_cull_mode() const;
	PoolVector2Array get_polygon() const;
	bool is_closed() const;
	void set_closed(const bool closed);
	void set_cull_mode(const int64_t cull_mode);
	void set_polygon(const PoolVector2Array polygon);

};

}

#endif