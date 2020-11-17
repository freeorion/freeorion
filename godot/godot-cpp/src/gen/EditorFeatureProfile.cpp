#include "EditorFeatureProfile.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


EditorFeatureProfile::___method_bindings EditorFeatureProfile::___mb = {};

void EditorFeatureProfile::___init_method_bindings() {
	___mb.mb_get_feature_name = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "get_feature_name");
	___mb.mb_is_class_disabled = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "is_class_disabled");
	___mb.mb_is_class_editor_disabled = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "is_class_editor_disabled");
	___mb.mb_is_class_property_disabled = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "is_class_property_disabled");
	___mb.mb_is_feature_disabled = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "is_feature_disabled");
	___mb.mb_load_from_file = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "load_from_file");
	___mb.mb_save_to_file = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "save_to_file");
	___mb.mb_set_disable_class = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "set_disable_class");
	___mb.mb_set_disable_class_editor = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "set_disable_class_editor");
	___mb.mb_set_disable_class_property = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "set_disable_class_property");
	___mb.mb_set_disable_feature = godot::api->godot_method_bind_get_method("EditorFeatureProfile", "set_disable_feature");
}

String EditorFeatureProfile::get_feature_name(const int64_t feature) {
	return ___godot_icall_String_int(___mb.mb_get_feature_name, (const Object *) this, feature);
}

bool EditorFeatureProfile::is_class_disabled(const String class_name) const {
	return ___godot_icall_bool_String(___mb.mb_is_class_disabled, (const Object *) this, class_name);
}

bool EditorFeatureProfile::is_class_editor_disabled(const String class_name) const {
	return ___godot_icall_bool_String(___mb.mb_is_class_editor_disabled, (const Object *) this, class_name);
}

bool EditorFeatureProfile::is_class_property_disabled(const String class_name, const String property) const {
	return ___godot_icall_bool_String_String(___mb.mb_is_class_property_disabled, (const Object *) this, class_name, property);
}

bool EditorFeatureProfile::is_feature_disabled(const int64_t feature) const {
	return ___godot_icall_bool_int(___mb.mb_is_feature_disabled, (const Object *) this, feature);
}

Error EditorFeatureProfile::load_from_file(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_load_from_file, (const Object *) this, path);
}

Error EditorFeatureProfile::save_to_file(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_save_to_file, (const Object *) this, path);
}

void EditorFeatureProfile::set_disable_class(const String class_name, const bool disable) {
	___godot_icall_void_String_bool(___mb.mb_set_disable_class, (const Object *) this, class_name, disable);
}

void EditorFeatureProfile::set_disable_class_editor(const String class_name, const bool disable) {
	___godot_icall_void_String_bool(___mb.mb_set_disable_class_editor, (const Object *) this, class_name, disable);
}

void EditorFeatureProfile::set_disable_class_property(const String class_name, const String property, const bool disable) {
	___godot_icall_void_String_String_bool(___mb.mb_set_disable_class_property, (const Object *) this, class_name, property, disable);
}

void EditorFeatureProfile::set_disable_feature(const int64_t feature, const bool disable) {
	___godot_icall_void_int_bool(___mb.mb_set_disable_feature, (const Object *) this, feature, disable);
}

}