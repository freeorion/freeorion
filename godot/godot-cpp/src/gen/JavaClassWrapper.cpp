#include "JavaClassWrapper.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "JavaClass.hpp"


namespace godot {


JavaClassWrapper *JavaClassWrapper::_singleton = NULL;


JavaClassWrapper::JavaClassWrapper() {
	_owner = godot::api->godot_global_get_singleton((char *) "JavaClassWrapper");
}


JavaClassWrapper::___method_bindings JavaClassWrapper::___mb = {};

void JavaClassWrapper::___init_method_bindings() {
	___mb.mb_wrap = godot::api->godot_method_bind_get_method("JavaClassWrapper", "wrap");
}

Ref<JavaClass> JavaClassWrapper::wrap(const String name) {
	return Ref<JavaClass>::__internal_constructor(___godot_icall_Object_String(___mb.mb_wrap, (const Object *) this, name));
}

}