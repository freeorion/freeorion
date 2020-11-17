#ifndef GODOT_CPP_VISUALSCRIPTFUNCTIONSTATE_HPP
#define GODOT_CPP_VISUALSCRIPTFUNCTIONSTATE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Object;

class VisualScriptFunctionState : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb__signal_callback;
		godot_method_bind *mb_connect_to_signal;
		godot_method_bind *mb_is_valid;
		godot_method_bind *mb_resume;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptFunctionState"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisualScriptFunctionState *_new();

	// methods
	Variant _signal_callback(const Array& __var_args = Array());
	void connect_to_signal(const Object *obj, const String signals, const Array args);
	bool is_valid() const;
	Variant resume(const Array args = Array());
	template <class... Args> Variant _signal_callback(Args... args){
		return _signal_callback(Array::make(args...));
	}

};

}

#endif