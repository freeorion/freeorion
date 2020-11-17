#ifndef GODOT_CPP_EDITORSETTINGS_HPP
#define GODOT_CPP_EDITORSETTINGS_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {


class EditorSettings : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_add_property_info;
		godot_method_bind *mb_erase;
		godot_method_bind *mb_get_favorites;
		godot_method_bind *mb_get_project_metadata;
		godot_method_bind *mb_get_project_settings_dir;
		godot_method_bind *mb_get_recent_dirs;
		godot_method_bind *mb_get_setting;
		godot_method_bind *mb_get_settings_dir;
		godot_method_bind *mb_has_setting;
		godot_method_bind *mb_property_can_revert;
		godot_method_bind *mb_property_get_revert;
		godot_method_bind *mb_set_favorites;
		godot_method_bind *mb_set_initial_value;
		godot_method_bind *mb_set_project_metadata;
		godot_method_bind *mb_set_recent_dirs;
		godot_method_bind *mb_set_setting;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorSettings"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants
	const static int NOTIFICATION_EDITOR_SETTINGS_CHANGED = 10000;

	// methods
	void add_property_info(const Dictionary info);
	void erase(const String property);
	PoolStringArray get_favorites() const;
	Variant get_project_metadata(const String section, const String key, const Variant _default = Variant()) const;
	String get_project_settings_dir() const;
	PoolStringArray get_recent_dirs() const;
	Variant get_setting(const String name) const;
	String get_settings_dir() const;
	bool has_setting(const String name) const;
	bool property_can_revert(const String name);
	Variant property_get_revert(const String name);
	void set_favorites(const PoolStringArray dirs);
	void set_initial_value(const String name, const Variant value, const bool update_current);
	void set_project_metadata(const String section, const String key, const Variant data);
	void set_recent_dirs(const PoolStringArray dirs);
	void set_setting(const String name, const Variant value);

};

}

#endif