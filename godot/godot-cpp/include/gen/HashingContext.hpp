#ifndef GODOT_CPP_HASHINGCONTEXT_HPP
#define GODOT_CPP_HASHINGCONTEXT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class HashingContext : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_finish;
		godot_method_bind *mb_start;
		godot_method_bind *mb_update;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "HashingContext"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum HashType {
		HASH_MD5 = 0,
		HASH_SHA1 = 1,
		HASH_SHA256 = 2,
	};

	// constants


	static HashingContext *_new();

	// methods
	PoolByteArray finish();
	Error start(const int64_t type);
	Error update(const PoolByteArray chunk);

};

}

#endif