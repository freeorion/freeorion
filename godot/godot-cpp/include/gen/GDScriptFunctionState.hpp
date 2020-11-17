#ifndef GODOT_CPP_GDSCRIPTFUNCTIONSTATE_HPP
#define GODOT_CPP_GDSCRIPTFUNCTIONSTATE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class GDScriptFunctionState : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb__signal_callback;
		godot_method_bind *mb_is_valid;
		godot_method_bind *mb_resume;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "GDScriptFunctionState"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	Variant _signal_callback(const Array& __var_args = Array());
	bool is_valid(const bool extended_check = false) const;
	Variant resume(const Variant arg = Variant());
	template <class... Args> Variant _signal_callback(Args... args){
		return _signal_callback(Array::make(args...));
	}

};

}

#endif