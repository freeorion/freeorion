#include "ScriptCreateDialog.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ScriptCreateDialog::___method_bindings ScriptCreateDialog::___mb = {};

void ScriptCreateDialog::___init_method_bindings() {
	___mb.mb__browse_class_in_tree = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_browse_class_in_tree");
	___mb.mb__browse_path = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_browse_path");
	___mb.mb__built_in_pressed = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_built_in_pressed");
	___mb.mb__class_name_changed = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_class_name_changed");
	___mb.mb__create = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_create");
	___mb.mb__file_selected = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_file_selected");
	___mb.mb__lang_changed = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_lang_changed");
	___mb.mb__parent_name_changed = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_parent_name_changed");
	___mb.mb__path_changed = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_path_changed");
	___mb.mb__path_entered = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_path_entered");
	___mb.mb__path_hbox_sorted = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_path_hbox_sorted");
	___mb.mb__template_changed = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "_template_changed");
	___mb.mb_config = godot::api->godot_method_bind_get_method("ScriptCreateDialog", "config");
}

void ScriptCreateDialog::_browse_class_in_tree() {
	___godot_icall_void(___mb.mb__browse_class_in_tree, (const Object *) this);
}

void ScriptCreateDialog::_browse_path(const bool arg0, const bool arg1) {
	___godot_icall_void_bool_bool(___mb.mb__browse_path, (const Object *) this, arg0, arg1);
}

void ScriptCreateDialog::_built_in_pressed() {
	___godot_icall_void(___mb.mb__built_in_pressed, (const Object *) this);
}

void ScriptCreateDialog::_class_name_changed(const String arg0) {
	___godot_icall_void_String(___mb.mb__class_name_changed, (const Object *) this, arg0);
}

void ScriptCreateDialog::_create() {
	___godot_icall_void(___mb.mb__create, (const Object *) this);
}

void ScriptCreateDialog::_file_selected(const String arg0) {
	___godot_icall_void_String(___mb.mb__file_selected, (const Object *) this, arg0);
}

void ScriptCreateDialog::_lang_changed(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__lang_changed, (const Object *) this, arg0);
}

void ScriptCreateDialog::_parent_name_changed(const String arg0) {
	___godot_icall_void_String(___mb.mb__parent_name_changed, (const Object *) this, arg0);
}

void ScriptCreateDialog::_path_changed(const String arg0) {
	___godot_icall_void_String(___mb.mb__path_changed, (const Object *) this, arg0);
}

void ScriptCreateDialog::_path_entered(const String arg0) {
	___godot_icall_void_String(___mb.mb__path_entered, (const Object *) this, arg0);
}

void ScriptCreateDialog::_path_hbox_sorted() {
	___godot_icall_void(___mb.mb__path_hbox_sorted, (const Object *) this);
}

void ScriptCreateDialog::_template_changed(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__template_changed, (const Object *) this, arg0);
}

void ScriptCreateDialog::config(const String inherits, const String path, const bool built_in_enabled, const bool load_enabled) {
	___godot_icall_void_String_String_bool_bool(___mb.mb_config, (const Object *) this, inherits, path, built_in_enabled, load_enabled);
}

}