#ifndef GODOT_CPP_PHYSICS2DSHAPEQUERYRESULT_HPP
#define GODOT_CPP_PHYSICS2DSHAPEQUERYRESULT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Object;

class Physics2DShapeQueryResult : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_result_count;
		godot_method_bind *mb_get_result_object;
		godot_method_bind *mb_get_result_object_id;
		godot_method_bind *mb_get_result_object_shape;
		godot_method_bind *mb_get_result_rid;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Physics2DShapeQueryResult"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	int64_t get_result_count() const;
	Object *get_result_object(const int64_t idx) const;
	int64_t get_result_object_id(const int64_t idx) const;
	int64_t get_result_object_shape(const int64_t idx) const;
	RID get_result_rid(const int64_t idx) const;

};

}

#endif