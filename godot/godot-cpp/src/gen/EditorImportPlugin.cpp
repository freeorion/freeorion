#include "EditorImportPlugin.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


EditorImportPlugin::___method_bindings EditorImportPlugin::___mb = {};

void EditorImportPlugin::___init_method_bindings() {
	___mb.mb_get_import_options = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_import_options");
	___mb.mb_get_import_order = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_import_order");
	___mb.mb_get_importer_name = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_importer_name");
	___mb.mb_get_option_visibility = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_option_visibility");
	___mb.mb_get_preset_count = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_preset_count");
	___mb.mb_get_preset_name = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_preset_name");
	___mb.mb_get_priority = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_priority");
	___mb.mb_get_recognized_extensions = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_recognized_extensions");
	___mb.mb_get_resource_type = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_resource_type");
	___mb.mb_get_save_extension = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_save_extension");
	___mb.mb_get_visible_name = godot::api->godot_method_bind_get_method("EditorImportPlugin", "get_visible_name");
	___mb.mb_import = godot::api->godot_method_bind_get_method("EditorImportPlugin", "import");
}

Array EditorImportPlugin::get_import_options(const int64_t preset) {
	return ___godot_icall_Array_int(___mb.mb_get_import_options, (const Object *) this, preset);
}

int64_t EditorImportPlugin::get_import_order() {
	return ___godot_icall_int(___mb.mb_get_import_order, (const Object *) this);
}

String EditorImportPlugin::get_importer_name() {
	return ___godot_icall_String(___mb.mb_get_importer_name, (const Object *) this);
}

bool EditorImportPlugin::get_option_visibility(const String option, const Dictionary options) {
	return ___godot_icall_bool_String_Dictionary(___mb.mb_get_option_visibility, (const Object *) this, option, options);
}

int64_t EditorImportPlugin::get_preset_count() {
	return ___godot_icall_int(___mb.mb_get_preset_count, (const Object *) this);
}

String EditorImportPlugin::get_preset_name(const int64_t preset) {
	return ___godot_icall_String_int(___mb.mb_get_preset_name, (const Object *) this, preset);
}

real_t EditorImportPlugin::get_priority() {
	return ___godot_icall_float(___mb.mb_get_priority, (const Object *) this);
}

Array EditorImportPlugin::get_recognized_extensions() {
	return ___godot_icall_Array(___mb.mb_get_recognized_extensions, (const Object *) this);
}

String EditorImportPlugin::get_resource_type() {
	return ___godot_icall_String(___mb.mb_get_resource_type, (const Object *) this);
}

String EditorImportPlugin::get_save_extension() {
	return ___godot_icall_String(___mb.mb_get_save_extension, (const Object *) this);
}

String EditorImportPlugin::get_visible_name() {
	return ___godot_icall_String(___mb.mb_get_visible_name, (const Object *) this);
}

int64_t EditorImportPlugin::import(const String source_file, const String save_path, const Dictionary options, const Array platform_variants, const Array gen_files) {
	return ___godot_icall_int_String_String_Dictionary_Array_Array(___mb.mb_import, (const Object *) this, source_file, save_path, options, platform_variants, gen_files);
}

}