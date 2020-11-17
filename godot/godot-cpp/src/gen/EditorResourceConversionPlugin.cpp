#include "EditorResourceConversionPlugin.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Resource.hpp"


namespace godot {


EditorResourceConversionPlugin::___method_bindings EditorResourceConversionPlugin::___mb = {};

void EditorResourceConversionPlugin::___init_method_bindings() {
	___mb.mb__convert = godot::api->godot_method_bind_get_method("EditorResourceConversionPlugin", "_convert");
	___mb.mb__converts_to = godot::api->godot_method_bind_get_method("EditorResourceConversionPlugin", "_converts_to");
}

Ref<Resource> EditorResourceConversionPlugin::_convert(const Ref<Resource> resource) {
	return Ref<Resource>::__internal_constructor(___godot_icall_Object_Object(___mb.mb__convert, (const Object *) this, resource.ptr()));
}

String EditorResourceConversionPlugin::_converts_to() {
	return ___godot_icall_String(___mb.mb__converts_to, (const Object *) this);
}

}