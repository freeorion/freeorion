#ifndef GODOT_CPP_EDITORFEATUREPROFILE_HPP
#define GODOT_CPP_EDITORFEATUREPROFILE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class EditorFeatureProfile : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_feature_name;
		godot_method_bind *mb_is_class_disabled;
		godot_method_bind *mb_is_class_editor_disabled;
		godot_method_bind *mb_is_class_property_disabled;
		godot_method_bind *mb_is_feature_disabled;
		godot_method_bind *mb_load_from_file;
		godot_method_bind *mb_save_to_file;
		godot_method_bind *mb_set_disable_class;
		godot_method_bind *mb_set_disable_class_editor;
		godot_method_bind *mb_set_disable_class_property;
		godot_method_bind *mb_set_disable_feature;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorFeatureProfile"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Feature {
		FEATURE_3D = 0,
		FEATURE_SCRIPT = 1,
		FEATURE_ASSET_LIB = 2,
		FEATURE_SCENE_TREE = 3,
		FEATURE_IMPORT_DOCK = 4,
		FEATURE_NODE_DOCK = 5,
		FEATURE_FILESYSTEM_DOCK = 6,
		FEATURE_MAX = 7,
	};

	// constants

	// methods
	String get_feature_name(const int64_t feature);
	bool is_class_disabled(const String class_name) const;
	bool is_class_editor_disabled(const String class_name) const;
	bool is_class_property_disabled(const String class_name, const String property) const;
	bool is_feature_disabled(const int64_t feature) const;
	Error load_from_file(const String path);
	Error save_to_file(const String path);
	void set_disable_class(const String class_name, const bool disable);
	void set_disable_class_editor(const String class_name, const bool disable);
	void set_disable_class_property(const String class_name, const String property, const bool disable);
	void set_disable_feature(const int64_t feature, const bool disable);

};

}

#endif