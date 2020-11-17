#ifndef GODOT_CPP_PCKPACKER_HPP
#define GODOT_CPP_PCKPACKER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class PCKPacker : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_add_file;
		godot_method_bind *mb_flush;
		godot_method_bind *mb_pck_start;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PCKPacker"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static PCKPacker *_new();

	// methods
	Error add_file(const String pck_path, const String source_path);
	Error flush(const bool verbose = false);
	Error pck_start(const String pck_name, const int64_t alignment = 0);

};

}

#endif