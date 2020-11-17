#ifndef GODOT_CPP_VISUALSCRIPTNODE_HPP
#define GODOT_CPP_VISUALSCRIPTNODE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class VisualScript;

class VisualScriptNode : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_default_input_values;
		godot_method_bind *mb__set_default_input_values;
		godot_method_bind *mb_get_default_input_value;
		godot_method_bind *mb_get_visual_script;
		godot_method_bind *mb_ports_changed_notify;
		godot_method_bind *mb_set_default_input_value;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptNode"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	Array _get_default_input_values() const;
	void _set_default_input_values(const Array values);
	Variant get_default_input_value(const int64_t port_idx) const;
	Ref<VisualScript> get_visual_script() const;
	void ports_changed_notify();
	void set_default_input_value(const int64_t port_idx, const Variant value);

};

}

#endif