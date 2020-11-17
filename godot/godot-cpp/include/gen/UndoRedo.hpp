#ifndef GODOT_CPP_UNDOREDO_HPP
#define GODOT_CPP_UNDOREDO_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class Object;

class UndoRedo : public Object {
	struct ___method_bindings {
		godot_method_bind *mb_add_do_method;
		godot_method_bind *mb_add_do_property;
		godot_method_bind *mb_add_do_reference;
		godot_method_bind *mb_add_undo_method;
		godot_method_bind *mb_add_undo_property;
		godot_method_bind *mb_add_undo_reference;
		godot_method_bind *mb_clear_history;
		godot_method_bind *mb_commit_action;
		godot_method_bind *mb_create_action;
		godot_method_bind *mb_get_current_action_name;
		godot_method_bind *mb_get_version;
		godot_method_bind *mb_has_redo;
		godot_method_bind *mb_has_undo;
		godot_method_bind *mb_is_commiting_action;
		godot_method_bind *mb_redo;
		godot_method_bind *mb_undo;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "UndoRedo"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum MergeMode {
		MERGE_DISABLE = 0,
		MERGE_ENDS = 1,
		MERGE_ALL = 2,
	};

	// constants


	static UndoRedo *_new();

	// methods
	void add_do_method(const Object *object, const String method, const Array& __var_args = Array());
	void add_do_property(const Object *object, const String property, const Variant value);
	void add_do_reference(const Object *object);
	void add_undo_method(const Object *object, const String method, const Array& __var_args = Array());
	void add_undo_property(const Object *object, const String property, const Variant value);
	void add_undo_reference(const Object *object);
	void clear_history(const bool increase_version = true);
	void commit_action();
	void create_action(const String name, const int64_t merge_mode = 0);
	String get_current_action_name() const;
	int64_t get_version() const;
	bool has_redo();
	bool has_undo();
	bool is_commiting_action() const;
	bool redo();
	bool undo();
	template <class... Args> void add_do_method(const Object *object, const String method, Args... args){
		return add_do_method(object,method, Array::make(args...));
	}
	template <class... Args> void add_undo_method(const Object *object, const String method, Args... args){
		return add_undo_method(object,method, Array::make(args...));
	}

};

}

#endif