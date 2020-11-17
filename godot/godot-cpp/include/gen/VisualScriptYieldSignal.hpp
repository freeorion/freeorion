#ifndef GODOT_CPP_VISUALSCRIPTYIELDSIGNAL_HPP
#define GODOT_CPP_VISUALSCRIPTYIELDSIGNAL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualScriptYieldSignal.hpp"

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptYieldSignal : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_base_path;
		godot_method_bind *mb_get_base_type;
		godot_method_bind *mb_get_call_mode;
		godot_method_bind *mb_get_signal;
		godot_method_bind *mb_set_base_path;
		godot_method_bind *mb_set_base_type;
		godot_method_bind *mb_set_call_mode;
		godot_method_bind *mb_set_signal;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptYieldSignal"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum CallMode {
		CALL_MODE_SELF = 0,
		CALL_MODE_NODE_PATH = 1,
		CALL_MODE_INSTANCE = 2,
	};

	// constants


	static VisualScriptYieldSignal *_new();

	// methods
	NodePath get_base_path() const;
	String get_base_type() const;
	VisualScriptYieldSignal::CallMode get_call_mode() const;
	String get_signal() const;
	void set_base_path(const NodePath base_path);
	void set_base_type(const String base_type);
	void set_call_mode(const int64_t mode);
	void set_signal(const String signal);

};

}

#endif