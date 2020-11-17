#include "NativeScript.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "GDNativeLibrary.hpp"


namespace godot {


NativeScript::___method_bindings NativeScript::___mb = {};

void NativeScript::___init_method_bindings() {
	___mb.mb_get_class_documentation = godot::api->godot_method_bind_get_method("NativeScript", "get_class_documentation");
	___mb.mb_get_class_name = godot::api->godot_method_bind_get_method("NativeScript", "get_class_name");
	___mb.mb_get_library = godot::api->godot_method_bind_get_method("NativeScript", "get_library");
	___mb.mb_get_method_documentation = godot::api->godot_method_bind_get_method("NativeScript", "get_method_documentation");
	___mb.mb_get_property_documentation = godot::api->godot_method_bind_get_method("NativeScript", "get_property_documentation");
	___mb.mb_get_script_class_icon_path = godot::api->godot_method_bind_get_method("NativeScript", "get_script_class_icon_path");
	___mb.mb_get_script_class_name = godot::api->godot_method_bind_get_method("NativeScript", "get_script_class_name");
	___mb.mb_get_signal_documentation = godot::api->godot_method_bind_get_method("NativeScript", "get_signal_documentation");
	___mb.mb_new = godot::api->godot_method_bind_get_method("NativeScript", "new");
	___mb.mb_set_class_name = godot::api->godot_method_bind_get_method("NativeScript", "set_class_name");
	___mb.mb_set_library = godot::api->godot_method_bind_get_method("NativeScript", "set_library");
	___mb.mb_set_script_class_icon_path = godot::api->godot_method_bind_get_method("NativeScript", "set_script_class_icon_path");
	___mb.mb_set_script_class_name = godot::api->godot_method_bind_get_method("NativeScript", "set_script_class_name");
}

NativeScript *NativeScript::_new()
{
	return (NativeScript *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"NativeScript")());
}
String NativeScript::get_class_documentation() const {
	return ___godot_icall_String(___mb.mb_get_class_documentation, (const Object *) this);
}

String NativeScript::get_class_name() const {
	return ___godot_icall_String(___mb.mb_get_class_name, (const Object *) this);
}

Ref<GDNativeLibrary> NativeScript::get_library() const {
	return Ref<GDNativeLibrary>::__internal_constructor(___godot_icall_Object(___mb.mb_get_library, (const Object *) this));
}

String NativeScript::get_method_documentation(const String method) const {
	return ___godot_icall_String_String(___mb.mb_get_method_documentation, (const Object *) this, method);
}

String NativeScript::get_property_documentation(const String path) const {
	return ___godot_icall_String_String(___mb.mb_get_property_documentation, (const Object *) this, path);
}

String NativeScript::get_script_class_icon_path() const {
	return ___godot_icall_String(___mb.mb_get_script_class_icon_path, (const Object *) this);
}

String NativeScript::get_script_class_name() const {
	return ___godot_icall_String(___mb.mb_get_script_class_name, (const Object *) this);
}

String NativeScript::get_signal_documentation(const String signal_name) const {
	return ___godot_icall_String_String(___mb.mb_get_signal_documentation, (const Object *) this, signal_name);
}

Variant NativeScript::new_(const Array& __var_args) {


	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 0));


	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 0] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_new, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 0), nullptr);


	return __result;
}

void NativeScript::set_class_name(const String class_name) {
	___godot_icall_void_String(___mb.mb_set_class_name, (const Object *) this, class_name);
}

void NativeScript::set_library(const Ref<GDNativeLibrary> library) {
	___godot_icall_void_Object(___mb.mb_set_library, (const Object *) this, library.ptr());
}

void NativeScript::set_script_class_icon_path(const String icon_path) {
	___godot_icall_void_String(___mb.mb_set_script_class_icon_path, (const Object *) this, icon_path);
}

void NativeScript::set_script_class_name(const String class_name) {
	___godot_icall_void_String(___mb.mb_set_script_class_name, (const Object *) this, class_name);
}

}