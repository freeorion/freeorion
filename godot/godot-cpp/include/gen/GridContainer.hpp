#ifndef GODOT_CPP_GRIDCONTAINER_HPP
#define GODOT_CPP_GRIDCONTAINER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Container.hpp"
namespace godot {


class GridContainer : public Container {
	struct ___method_bindings {
		godot_method_bind *mb_get_columns;
		godot_method_bind *mb_set_columns;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "GridContainer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static GridContainer *_new();

	// methods
	int64_t get_columns() const;
	void set_columns(const int64_t columns);

};

}

#endif