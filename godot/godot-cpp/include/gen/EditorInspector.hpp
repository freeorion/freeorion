#ifndef GODOT_CPP_EDITORINSPECTOR_HPP
#define GODOT_CPP_EDITORINSPECTOR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "ScrollContainer.hpp"
namespace godot {

class Object;
class Node;
class Resource;

class EditorInspector : public ScrollContainer {
	struct ___method_bindings {
		godot_method_bind *mb__edit_request_change;
		godot_method_bind *mb__feature_profile_changed;
		godot_method_bind *mb__filter_changed;
		godot_method_bind *mb__multiple_properties_changed;
		godot_method_bind *mb__node_removed;
		godot_method_bind *mb__object_id_selected;
		godot_method_bind *mb__property_changed;
		godot_method_bind *mb__property_changed_update_all;
		godot_method_bind *mb__property_checked;
		godot_method_bind *mb__property_keyed;
		godot_method_bind *mb__property_keyed_with_value;
		godot_method_bind *mb__property_selected;
		godot_method_bind *mb__resource_selected;
		godot_method_bind *mb__vscroll_changed;
		godot_method_bind *mb_refresh;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorInspector"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _edit_request_change(const Object *arg0, const String arg1);
	void _feature_profile_changed();
	void _filter_changed(const String arg0);
	void _multiple_properties_changed(const PoolStringArray arg0, const Array arg1);
	void _node_removed(const Node *arg0);
	void _object_id_selected(const String arg0, const int64_t arg1);
	void _property_changed(const String arg0, const Variant arg1, const String arg2 = "", const bool arg3 = false);
	void _property_changed_update_all(const String arg0, const Variant arg1, const String arg2, const bool arg3);
	void _property_checked(const String arg0, const bool arg1);
	void _property_keyed(const String arg0, const bool arg1);
	void _property_keyed_with_value(const String arg0, const Variant arg1, const bool arg2);
	void _property_selected(const String arg0, const int64_t arg1);
	void _resource_selected(const String arg0, const Ref<Resource> arg1);
	void _vscroll_changed(const real_t arg0);
	void refresh();

};

}

#endif