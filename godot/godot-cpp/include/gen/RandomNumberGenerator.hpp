#ifndef GODOT_CPP_RANDOMNUMBERGENERATOR_HPP
#define GODOT_CPP_RANDOMNUMBERGENERATOR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class RandomNumberGenerator : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_seed;
		godot_method_bind *mb_randf;
		godot_method_bind *mb_randf_range;
		godot_method_bind *mb_randfn;
		godot_method_bind *mb_randi;
		godot_method_bind *mb_randi_range;
		godot_method_bind *mb_randomize;
		godot_method_bind *mb_set_seed;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "RandomNumberGenerator"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static RandomNumberGenerator *_new();

	// methods
	int64_t get_seed();
	real_t randf();
	real_t randf_range(const real_t from, const real_t to);
	real_t randfn(const real_t mean = 0, const real_t deviation = 1);
	int64_t randi();
	int64_t randi_range(const int64_t from, const int64_t to);
	void randomize();
	void set_seed(const int64_t seed);

};

}

#endif