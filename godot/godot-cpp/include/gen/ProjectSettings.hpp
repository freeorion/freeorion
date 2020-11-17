#ifndef GODOT_CPP_PROJECTSETTINGS_HPP
#define GODOT_CPP_PROJECTSETTINGS_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {


class ProjectSettings : public Object {
	static ProjectSettings *_singleton;

	ProjectSettings();

	struct ___method_bindings {
		godot_method_bind *mb_add_property_info;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_get_order;
		godot_method_bind *mb_get_setting;
		godot_method_bind *mb_globalize_path;
		godot_method_bind *mb_has_setting;
		godot_method_bind *mb_load_resource_pack;
		godot_method_bind *mb_localize_path;
		godot_method_bind *mb_property_can_revert;
		godot_method_bind *mb_property_get_revert;
		godot_method_bind *mb_save;
		godot_method_bind *mb_save_custom;
		godot_method_bind *mb_set_initial_value;
		godot_method_bind *mb_set_order;
		godot_method_bind *mb_set_setting;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline ProjectSettings *get_singleton()
	{
		if (!ProjectSettings::_singleton) {
			ProjectSettings::_singleton = new ProjectSettings;
		}
		return ProjectSettings::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "ProjectSettings"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void add_property_info(const Dictionary hint);
	void clear(const String name);
	int64_t get_order(const String name) const;
	Variant get_setting(const String name) const;
	String globalize_path(const String path) const;
	bool has_setting(const String name) const;
	bool load_resource_pack(const String pack, const bool replace_files = true);
	String localize_path(const String path) const;
	bool property_can_revert(const String name);
	Variant property_get_revert(const String name);
	Error save();
	Error save_custom(const String file);
	void set_initial_value(const String name, const Variant value);
	void set_order(const String name, const int64_t position);
	void set_setting(const String name, const Variant value);

};

}

#endif