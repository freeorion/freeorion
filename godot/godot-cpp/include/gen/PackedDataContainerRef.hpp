#ifndef GODOT_CPP_PACKEDDATACONTAINERREF_HPP
#define GODOT_CPP_PACKEDDATACONTAINERREF_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class PackedDataContainerRef : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb__is_dictionary;
		godot_method_bind *mb__iter_get;
		godot_method_bind *mb__iter_init;
		godot_method_bind *mb__iter_next;
		godot_method_bind *mb_size;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PackedDataContainerRef"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	bool _is_dictionary() const;
	Variant _iter_get(const Variant arg0);
	Variant _iter_init(const Array arg0);
	Variant _iter_next(const Array arg0);
	int64_t size() const;

};

}

#endif