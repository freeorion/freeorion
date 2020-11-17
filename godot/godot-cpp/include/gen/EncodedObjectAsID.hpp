#ifndef GODOT_CPP_ENCODEDOBJECTASID_HPP
#define GODOT_CPP_ENCODEDOBJECTASID_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class EncodedObjectAsID : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_object_id;
		godot_method_bind *mb_set_object_id;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EncodedObjectAsID"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static EncodedObjectAsID *_new();

	// methods
	int64_t get_object_id() const;
	void set_object_id(const int64_t id);

};

}

#endif