#ifndef GODOT_CPP_VISUALSCRIPTYIELD_HPP
#define GODOT_CPP_VISUALSCRIPTYIELD_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualScriptYield.hpp"

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptYield : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_wait_time;
		godot_method_bind *mb_get_yield_mode;
		godot_method_bind *mb_set_wait_time;
		godot_method_bind *mb_set_yield_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptYield"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum YieldMode {
		YIELD_FRAME = 1,
		YIELD_PHYSICS_FRAME = 2,
		YIELD_WAIT = 3,
	};

	// constants


	static VisualScriptYield *_new();

	// methods
	real_t get_wait_time();
	VisualScriptYield::YieldMode get_yield_mode();
	void set_wait_time(const real_t sec);
	void set_yield_mode(const int64_t mode);

};

}

#endif