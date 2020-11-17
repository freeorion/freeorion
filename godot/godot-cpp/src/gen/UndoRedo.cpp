#include "UndoRedo.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


UndoRedo::___method_bindings UndoRedo::___mb = {};

void UndoRedo::___init_method_bindings() {
	___mb.mb_add_do_method = godot::api->godot_method_bind_get_method("UndoRedo", "add_do_method");
	___mb.mb_add_do_property = godot::api->godot_method_bind_get_method("UndoRedo", "add_do_property");
	___mb.mb_add_do_reference = godot::api->godot_method_bind_get_method("UndoRedo", "add_do_reference");
	___mb.mb_add_undo_method = godot::api->godot_method_bind_get_method("UndoRedo", "add_undo_method");
	___mb.mb_add_undo_property = godot::api->godot_method_bind_get_method("UndoRedo", "add_undo_property");
	___mb.mb_add_undo_reference = godot::api->godot_method_bind_get_method("UndoRedo", "add_undo_reference");
	___mb.mb_clear_history = godot::api->godot_method_bind_get_method("UndoRedo", "clear_history");
	___mb.mb_commit_action = godot::api->godot_method_bind_get_method("UndoRedo", "commit_action");
	___mb.mb_create_action = godot::api->godot_method_bind_get_method("UndoRedo", "create_action");
	___mb.mb_get_current_action_name = godot::api->godot_method_bind_get_method("UndoRedo", "get_current_action_name");
	___mb.mb_get_version = godot::api->godot_method_bind_get_method("UndoRedo", "get_version");
	___mb.mb_has_redo = godot::api->godot_method_bind_get_method("UndoRedo", "has_redo");
	___mb.mb_has_undo = godot::api->godot_method_bind_get_method("UndoRedo", "has_undo");
	___mb.mb_is_commiting_action = godot::api->godot_method_bind_get_method("UndoRedo", "is_commiting_action");
	___mb.mb_redo = godot::api->godot_method_bind_get_method("UndoRedo", "redo");
	___mb.mb_undo = godot::api->godot_method_bind_get_method("UndoRedo", "undo");
}

UndoRedo *UndoRedo::_new()
{
	return (UndoRedo *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"UndoRedo")());
}
void UndoRedo::add_do_method(const Object *object, const String method, const Array& __var_args) {
	Variant __given_args[2];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[1]);

	__given_args[0] = object;
	__given_args[1] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 2));

	__args[0] = (godot_variant *) &__given_args[0];
	__args[1] = (godot_variant *) &__given_args[1];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 2] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_add_do_method, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 2), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_destroy((godot_variant *) &__given_args[1]);

}

void UndoRedo::add_do_property(const Object *object, const String property, const Variant value) {
	___godot_icall_void_Object_String_Variant(___mb.mb_add_do_property, (const Object *) this, object, property, value);
}

void UndoRedo::add_do_reference(const Object *object) {
	___godot_icall_void_Object(___mb.mb_add_do_reference, (const Object *) this, object);
}

void UndoRedo::add_undo_method(const Object *object, const String method, const Array& __var_args) {
	Variant __given_args[2];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[1]);

	__given_args[0] = object;
	__given_args[1] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 2));

	__args[0] = (godot_variant *) &__given_args[0];
	__args[1] = (godot_variant *) &__given_args[1];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 2] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_add_undo_method, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 2), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_destroy((godot_variant *) &__given_args[1]);

}

void UndoRedo::add_undo_property(const Object *object, const String property, const Variant value) {
	___godot_icall_void_Object_String_Variant(___mb.mb_add_undo_property, (const Object *) this, object, property, value);
}

void UndoRedo::add_undo_reference(const Object *object) {
	___godot_icall_void_Object(___mb.mb_add_undo_reference, (const Object *) this, object);
}

void UndoRedo::clear_history(const bool increase_version) {
	___godot_icall_void_bool(___mb.mb_clear_history, (const Object *) this, increase_version);
}

void UndoRedo::commit_action() {
	___godot_icall_void(___mb.mb_commit_action, (const Object *) this);
}

void UndoRedo::create_action(const String name, const int64_t merge_mode) {
	___godot_icall_void_String_int(___mb.mb_create_action, (const Object *) this, name, merge_mode);
}

String UndoRedo::get_current_action_name() const {
	return ___godot_icall_String(___mb.mb_get_current_action_name, (const Object *) this);
}

int64_t UndoRedo::get_version() const {
	return ___godot_icall_int(___mb.mb_get_version, (const Object *) this);
}

bool UndoRedo::has_redo() {
	return ___godot_icall_bool(___mb.mb_has_redo, (const Object *) this);
}

bool UndoRedo::has_undo() {
	return ___godot_icall_bool(___mb.mb_has_undo, (const Object *) this);
}

bool UndoRedo::is_commiting_action() const {
	return ___godot_icall_bool(___mb.mb_is_commiting_action, (const Object *) this);
}

bool UndoRedo::redo() {
	return ___godot_icall_bool(___mb.mb_redo, (const Object *) this);
}

bool UndoRedo::undo() {
	return ___godot_icall_bool(___mb.mb_undo, (const Object *) this);
}

}