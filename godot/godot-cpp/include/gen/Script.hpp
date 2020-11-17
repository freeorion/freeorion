#ifndef GODOT_CPP_SCRIPT_HPP
#define GODOT_CPP_SCRIPT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class Script;
class Object;

class Script : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_can_instance;
		godot_method_bind *mb_get_base_script;
		godot_method_bind *mb_get_instance_base_type;
		godot_method_bind *mb_get_property_default_value;
		godot_method_bind *mb_get_script_constant_map;
		godot_method_bind *mb_get_script_method_list;
		godot_method_bind *mb_get_script_property_list;
		godot_method_bind *mb_get_script_signal_list;
		godot_method_bind *mb_get_source_code;
		godot_method_bind *mb_has_script_signal;
		godot_method_bind *mb_has_source_code;
		godot_method_bind *mb_instance_has;
		godot_method_bind *mb_is_tool;
		godot_method_bind *mb_reload;
		godot_method_bind *mb_set_source_code;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Script"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	bool can_instance() const;
	Ref<Script> get_base_script() const;
	String get_instance_base_type() const;
	Variant get_property_default_value(const String property);
	Dictionary get_script_constant_map();
	Array get_script_method_list();
	Array get_script_property_list();
	Array get_script_signal_list();
	String get_source_code() const;
	bool has_script_signal(const String signal_name) const;
	bool has_source_code() const;
	bool instance_has(const Object *base_object) const;
	bool is_tool() const;
	Error reload(const bool keep_state = false);
	void set_source_code(const String source);

};

}

#endif