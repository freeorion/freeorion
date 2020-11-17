#ifndef GODOT_CPP_SCRIPTCREATEDIALOG_HPP
#define GODOT_CPP_SCRIPTCREATEDIALOG_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "ConfirmationDialog.hpp"
namespace godot {


class ScriptCreateDialog : public ConfirmationDialog {
	struct ___method_bindings {
		godot_method_bind *mb__browse_class_in_tree;
		godot_method_bind *mb__browse_path;
		godot_method_bind *mb__built_in_pressed;
		godot_method_bind *mb__class_name_changed;
		godot_method_bind *mb__create;
		godot_method_bind *mb__file_selected;
		godot_method_bind *mb__lang_changed;
		godot_method_bind *mb__parent_name_changed;
		godot_method_bind *mb__path_changed;
		godot_method_bind *mb__path_entered;
		godot_method_bind *mb__path_hbox_sorted;
		godot_method_bind *mb__template_changed;
		godot_method_bind *mb_config;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ScriptCreateDialog"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _browse_class_in_tree();
	void _browse_path(const bool arg0, const bool arg1);
	void _built_in_pressed();
	void _class_name_changed(const String arg0);
	void _create();
	void _file_selected(const String arg0);
	void _lang_changed(const int64_t arg0);
	void _parent_name_changed(const String arg0);
	void _path_changed(const String arg0);
	void _path_entered(const String arg0);
	void _path_hbox_sorted();
	void _template_changed(const int64_t arg0);
	void config(const String inherits, const String path, const bool built_in_enabled = true, const bool load_enabled = true);

};

}

#endif