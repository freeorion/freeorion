#include "EditorInspector.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "Node.hpp"
#include "Resource.hpp"


namespace godot {


EditorInspector::___method_bindings EditorInspector::___mb = {};

void EditorInspector::___init_method_bindings() {
	___mb.mb__edit_request_change = godot::api->godot_method_bind_get_method("EditorInspector", "_edit_request_change");
	___mb.mb__feature_profile_changed = godot::api->godot_method_bind_get_method("EditorInspector", "_feature_profile_changed");
	___mb.mb__filter_changed = godot::api->godot_method_bind_get_method("EditorInspector", "_filter_changed");
	___mb.mb__multiple_properties_changed = godot::api->godot_method_bind_get_method("EditorInspector", "_multiple_properties_changed");
	___mb.mb__node_removed = godot::api->godot_method_bind_get_method("EditorInspector", "_node_removed");
	___mb.mb__object_id_selected = godot::api->godot_method_bind_get_method("EditorInspector", "_object_id_selected");
	___mb.mb__property_changed = godot::api->godot_method_bind_get_method("EditorInspector", "_property_changed");
	___mb.mb__property_changed_update_all = godot::api->godot_method_bind_get_method("EditorInspector", "_property_changed_update_all");
	___mb.mb__property_checked = godot::api->godot_method_bind_get_method("EditorInspector", "_property_checked");
	___mb.mb__property_keyed = godot::api->godot_method_bind_get_method("EditorInspector", "_property_keyed");
	___mb.mb__property_keyed_with_value = godot::api->godot_method_bind_get_method("EditorInspector", "_property_keyed_with_value");
	___mb.mb__property_selected = godot::api->godot_method_bind_get_method("EditorInspector", "_property_selected");
	___mb.mb__resource_selected = godot::api->godot_method_bind_get_method("EditorInspector", "_resource_selected");
	___mb.mb__vscroll_changed = godot::api->godot_method_bind_get_method("EditorInspector", "_vscroll_changed");
	___mb.mb_refresh = godot::api->godot_method_bind_get_method("EditorInspector", "refresh");
}

void EditorInspector::_edit_request_change(const Object *arg0, const String arg1) {
	___godot_icall_void_Object_String(___mb.mb__edit_request_change, (const Object *) this, arg0, arg1);
}

void EditorInspector::_feature_profile_changed() {
	___godot_icall_void(___mb.mb__feature_profile_changed, (const Object *) this);
}

void EditorInspector::_filter_changed(const String arg0) {
	___godot_icall_void_String(___mb.mb__filter_changed, (const Object *) this, arg0);
}

void EditorInspector::_multiple_properties_changed(const PoolStringArray arg0, const Array arg1) {
	___godot_icall_void_PoolStringArray_Array(___mb.mb__multiple_properties_changed, (const Object *) this, arg0, arg1);
}

void EditorInspector::_node_removed(const Node *arg0) {
	___godot_icall_void_Object(___mb.mb__node_removed, (const Object *) this, arg0);
}

void EditorInspector::_object_id_selected(const String arg0, const int64_t arg1) {
	___godot_icall_void_String_int(___mb.mb__object_id_selected, (const Object *) this, arg0, arg1);
}

void EditorInspector::_property_changed(const String arg0, const Variant arg1, const String arg2, const bool arg3) {
	___godot_icall_void_String_Variant_String_bool(___mb.mb__property_changed, (const Object *) this, arg0, arg1, arg2, arg3);
}

void EditorInspector::_property_changed_update_all(const String arg0, const Variant arg1, const String arg2, const bool arg3) {
	___godot_icall_void_String_Variant_String_bool(___mb.mb__property_changed_update_all, (const Object *) this, arg0, arg1, arg2, arg3);
}

void EditorInspector::_property_checked(const String arg0, const bool arg1) {
	___godot_icall_void_String_bool(___mb.mb__property_checked, (const Object *) this, arg0, arg1);
}

void EditorInspector::_property_keyed(const String arg0, const bool arg1) {
	___godot_icall_void_String_bool(___mb.mb__property_keyed, (const Object *) this, arg0, arg1);
}

void EditorInspector::_property_keyed_with_value(const String arg0, const Variant arg1, const bool arg2) {
	___godot_icall_void_String_Variant_bool(___mb.mb__property_keyed_with_value, (const Object *) this, arg0, arg1, arg2);
}

void EditorInspector::_property_selected(const String arg0, const int64_t arg1) {
	___godot_icall_void_String_int(___mb.mb__property_selected, (const Object *) this, arg0, arg1);
}

void EditorInspector::_resource_selected(const String arg0, const Ref<Resource> arg1) {
	___godot_icall_void_String_Object(___mb.mb__resource_selected, (const Object *) this, arg0, arg1.ptr());
}

void EditorInspector::_vscroll_changed(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__vscroll_changed, (const Object *) this, arg0);
}

void EditorInspector::refresh() {
	___godot_icall_void(___mb.mb_refresh, (const Object *) this);
}

}