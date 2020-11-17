#include "RandomNumberGenerator.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


RandomNumberGenerator::___method_bindings RandomNumberGenerator::___mb = {};

void RandomNumberGenerator::___init_method_bindings() {
	___mb.mb_get_seed = godot::api->godot_method_bind_get_method("RandomNumberGenerator", "get_seed");
	___mb.mb_randf = godot::api->godot_method_bind_get_method("RandomNumberGenerator", "randf");
	___mb.mb_randf_range = godot::api->godot_method_bind_get_method("RandomNumberGenerator", "randf_range");
	___mb.mb_randfn = godot::api->godot_method_bind_get_method("RandomNumberGenerator", "randfn");
	___mb.mb_randi = godot::api->godot_method_bind_get_method("RandomNumberGenerator", "randi");
	___mb.mb_randi_range = godot::api->godot_method_bind_get_method("RandomNumberGenerator", "randi_range");
	___mb.mb_randomize = godot::api->godot_method_bind_get_method("RandomNumberGenerator", "randomize");
	___mb.mb_set_seed = godot::api->godot_method_bind_get_method("RandomNumberGenerator", "set_seed");
}

RandomNumberGenerator *RandomNumberGenerator::_new()
{
	return (RandomNumberGenerator *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"RandomNumberGenerator")());
}
int64_t RandomNumberGenerator::get_seed() {
	return ___godot_icall_int(___mb.mb_get_seed, (const Object *) this);
}

real_t RandomNumberGenerator::randf() {
	return ___godot_icall_float(___mb.mb_randf, (const Object *) this);
}

real_t RandomNumberGenerator::randf_range(const real_t from, const real_t to) {
	return ___godot_icall_float_float_float(___mb.mb_randf_range, (const Object *) this, from, to);
}

real_t RandomNumberGenerator::randfn(const real_t mean, const real_t deviation) {
	return ___godot_icall_float_float_float(___mb.mb_randfn, (const Object *) this, mean, deviation);
}

int64_t RandomNumberGenerator::randi() {
	return ___godot_icall_int(___mb.mb_randi, (const Object *) this);
}

int64_t RandomNumberGenerator::randi_range(const int64_t from, const int64_t to) {
	return ___godot_icall_int_int_int(___mb.mb_randi_range, (const Object *) this, from, to);
}

void RandomNumberGenerator::randomize() {
	___godot_icall_void(___mb.mb_randomize, (const Object *) this);
}

void RandomNumberGenerator::set_seed(const int64_t seed) {
	___godot_icall_void_int(___mb.mb_set_seed, (const Object *) this, seed);
}

}