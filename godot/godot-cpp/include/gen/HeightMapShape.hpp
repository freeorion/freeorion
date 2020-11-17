#ifndef GODOT_CPP_HEIGHTMAPSHAPE_HPP
#define GODOT_CPP_HEIGHTMAPSHAPE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Shape.hpp"
namespace godot {


class HeightMapShape : public Shape {
	struct ___method_bindings {
		godot_method_bind *mb_get_map_data;
		godot_method_bind *mb_get_map_depth;
		godot_method_bind *mb_get_map_width;
		godot_method_bind *mb_set_map_data;
		godot_method_bind *mb_set_map_depth;
		godot_method_bind *mb_set_map_width;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "HeightMapShape"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static HeightMapShape *_new();

	// methods
	PoolRealArray get_map_data() const;
	int64_t get_map_depth() const;
	int64_t get_map_width() const;
	void set_map_data(const PoolRealArray data);
	void set_map_depth(const int64_t height);
	void set_map_width(const int64_t width);

};

}

#endif