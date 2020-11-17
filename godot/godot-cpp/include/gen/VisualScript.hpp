#ifndef GODOT_CPP_VISUALSCRIPT_HPP
#define GODOT_CPP_VISUALSCRIPT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Variant.hpp"

#include "Script.hpp"
namespace godot {

class VisualScriptNode;

class VisualScript : public Script {
	struct ___method_bindings {
		godot_method_bind *mb__get_data;
		godot_method_bind *mb__node_ports_changed;
		godot_method_bind *mb__set_data;
		godot_method_bind *mb_add_custom_signal;
		godot_method_bind *mb_add_function;
		godot_method_bind *mb_add_node;
		godot_method_bind *mb_add_variable;
		godot_method_bind *mb_custom_signal_add_argument;
		godot_method_bind *mb_custom_signal_get_argument_count;
		godot_method_bind *mb_custom_signal_get_argument_name;
		godot_method_bind *mb_custom_signal_get_argument_type;
		godot_method_bind *mb_custom_signal_remove_argument;
		godot_method_bind *mb_custom_signal_set_argument_name;
		godot_method_bind *mb_custom_signal_set_argument_type;
		godot_method_bind *mb_custom_signal_swap_argument;
		godot_method_bind *mb_data_connect;
		godot_method_bind *mb_data_disconnect;
		godot_method_bind *mb_get_function_node_id;
		godot_method_bind *mb_get_function_scroll;
		godot_method_bind *mb_get_node;
		godot_method_bind *mb_get_node_position;
		godot_method_bind *mb_get_variable_default_value;
		godot_method_bind *mb_get_variable_export;
		godot_method_bind *mb_get_variable_info;
		godot_method_bind *mb_has_custom_signal;
		godot_method_bind *mb_has_data_connection;
		godot_method_bind *mb_has_function;
		godot_method_bind *mb_has_node;
		godot_method_bind *mb_has_sequence_connection;
		godot_method_bind *mb_has_variable;
		godot_method_bind *mb_remove_custom_signal;
		godot_method_bind *mb_remove_function;
		godot_method_bind *mb_remove_node;
		godot_method_bind *mb_remove_variable;
		godot_method_bind *mb_rename_custom_signal;
		godot_method_bind *mb_rename_function;
		godot_method_bind *mb_rename_variable;
		godot_method_bind *mb_sequence_connect;
		godot_method_bind *mb_sequence_disconnect;
		godot_method_bind *mb_set_function_scroll;
		godot_method_bind *mb_set_instance_base_type;
		godot_method_bind *mb_set_node_position;
		godot_method_bind *mb_set_variable_default_value;
		godot_method_bind *mb_set_variable_export;
		godot_method_bind *mb_set_variable_info;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScript"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisualScript *_new();

	// methods
	Dictionary _get_data() const;
	void _node_ports_changed(const int64_t arg0);
	void _set_data(const Dictionary data);
	void add_custom_signal(const String name);
	void add_function(const String name);
	void add_node(const String func, const int64_t id, const Ref<VisualScriptNode> node, const Vector2 position = Vector2(0, 0));
	void add_variable(const String name, const Variant default_value = Variant(), const bool _export = false);
	void custom_signal_add_argument(const String name, const int64_t type, const String argname, const int64_t index = -1);
	int64_t custom_signal_get_argument_count(const String name) const;
	String custom_signal_get_argument_name(const String name, const int64_t argidx) const;
	Variant::Type custom_signal_get_argument_type(const String name, const int64_t argidx) const;
	void custom_signal_remove_argument(const String name, const int64_t argidx);
	void custom_signal_set_argument_name(const String name, const int64_t argidx, const String argname);
	void custom_signal_set_argument_type(const String name, const int64_t argidx, const int64_t type);
	void custom_signal_swap_argument(const String name, const int64_t argidx, const int64_t withidx);
	void data_connect(const String func, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port);
	void data_disconnect(const String func, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port);
	int64_t get_function_node_id(const String name) const;
	Vector2 get_function_scroll(const String name) const;
	Ref<VisualScriptNode> get_node(const String func, const int64_t id) const;
	Vector2 get_node_position(const String func, const int64_t id) const;
	Variant get_variable_default_value(const String name) const;
	bool get_variable_export(const String name) const;
	Dictionary get_variable_info(const String name) const;
	bool has_custom_signal(const String name) const;
	bool has_data_connection(const String func, const int64_t from_node, const int64_t from_port, const int64_t to_node, const int64_t to_port) const;
	bool has_function(const String name) const;
	bool has_node(const String func, const int64_t id) const;
	bool has_sequence_connection(const String func, const int64_t from_node, const int64_t from_output, const int64_t to_node) const;
	bool has_variable(const String name) const;
	void remove_custom_signal(const String name);
	void remove_function(const String name);
	void remove_node(const String func, const int64_t id);
	void remove_variable(const String name);
	void rename_custom_signal(const String name, const String new_name);
	void rename_function(const String name, const String new_name);
	void rename_variable(const String name, const String new_name);
	void sequence_connect(const String func, const int64_t from_node, const int64_t from_output, const int64_t to_node);
	void sequence_disconnect(const String func, const int64_t from_node, const int64_t from_output, const int64_t to_node);
	void set_function_scroll(const String name, const Vector2 ofs);
	void set_instance_base_type(const String type);
	void set_node_position(const String func, const int64_t id, const Vector2 position);
	void set_variable_default_value(const String name, const Variant value);
	void set_variable_export(const String name, const bool enable);
	void set_variable_info(const String name, const Dictionary value);

};

}

#endif