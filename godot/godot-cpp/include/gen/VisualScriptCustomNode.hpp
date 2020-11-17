#ifndef GODOT_CPP_VISUALSCRIPTCUSTOMNODE_HPP
#define GODOT_CPP_VISUALSCRIPTCUSTOMNODE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptCustomNode : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb__get_caption;
		godot_method_bind *mb__get_category;
		godot_method_bind *mb__get_input_value_port_count;
		godot_method_bind *mb__get_input_value_port_name;
		godot_method_bind *mb__get_input_value_port_type;
		godot_method_bind *mb__get_output_sequence_port_count;
		godot_method_bind *mb__get_output_sequence_port_text;
		godot_method_bind *mb__get_output_value_port_count;
		godot_method_bind *mb__get_output_value_port_name;
		godot_method_bind *mb__get_output_value_port_type;
		godot_method_bind *mb__get_text;
		godot_method_bind *mb__get_working_memory_size;
		godot_method_bind *mb__has_input_sequence_port;
		godot_method_bind *mb__script_changed;
		godot_method_bind *mb__step;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptCustomNode"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum StartMode {
		START_MODE_BEGIN_SEQUENCE = 0,
		START_MODE_CONTINUE_SEQUENCE = 1,
		START_MODE_RESUME_YIELD = 2,
	};

	// constants
	const static int STEP_EXIT_FUNCTION_BIT = 134217728;
	const static int STEP_GO_BACK_BIT = 33554432;
	const static int STEP_NO_ADVANCE_BIT = 67108864;
	const static int STEP_PUSH_STACK_BIT = 16777216;
	const static int STEP_YIELD_BIT = 268435456;


	static VisualScriptCustomNode *_new();

	// methods
	String _get_caption();
	String _get_category();
	int64_t _get_input_value_port_count();
	String _get_input_value_port_name(const int64_t idx);
	int64_t _get_input_value_port_type(const int64_t idx);
	int64_t _get_output_sequence_port_count();
	String _get_output_sequence_port_text(const int64_t idx);
	int64_t _get_output_value_port_count();
	String _get_output_value_port_name(const int64_t idx);
	int64_t _get_output_value_port_type(const int64_t idx);
	String _get_text();
	int64_t _get_working_memory_size();
	bool _has_input_sequence_port();
	void _script_changed();
	Variant _step(const Array inputs, const Array outputs, const int64_t start_mode, const Array working_mem);

};

}

#endif