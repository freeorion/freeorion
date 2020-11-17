#include "UPNPDevice.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


UPNPDevice::___method_bindings UPNPDevice::___mb = {};

void UPNPDevice::___init_method_bindings() {
	___mb.mb_add_port_mapping = godot::api->godot_method_bind_get_method("UPNPDevice", "add_port_mapping");
	___mb.mb_delete_port_mapping = godot::api->godot_method_bind_get_method("UPNPDevice", "delete_port_mapping");
	___mb.mb_get_description_url = godot::api->godot_method_bind_get_method("UPNPDevice", "get_description_url");
	___mb.mb_get_igd_control_url = godot::api->godot_method_bind_get_method("UPNPDevice", "get_igd_control_url");
	___mb.mb_get_igd_our_addr = godot::api->godot_method_bind_get_method("UPNPDevice", "get_igd_our_addr");
	___mb.mb_get_igd_service_type = godot::api->godot_method_bind_get_method("UPNPDevice", "get_igd_service_type");
	___mb.mb_get_igd_status = godot::api->godot_method_bind_get_method("UPNPDevice", "get_igd_status");
	___mb.mb_get_service_type = godot::api->godot_method_bind_get_method("UPNPDevice", "get_service_type");
	___mb.mb_is_valid_gateway = godot::api->godot_method_bind_get_method("UPNPDevice", "is_valid_gateway");
	___mb.mb_query_external_address = godot::api->godot_method_bind_get_method("UPNPDevice", "query_external_address");
	___mb.mb_set_description_url = godot::api->godot_method_bind_get_method("UPNPDevice", "set_description_url");
	___mb.mb_set_igd_control_url = godot::api->godot_method_bind_get_method("UPNPDevice", "set_igd_control_url");
	___mb.mb_set_igd_our_addr = godot::api->godot_method_bind_get_method("UPNPDevice", "set_igd_our_addr");
	___mb.mb_set_igd_service_type = godot::api->godot_method_bind_get_method("UPNPDevice", "set_igd_service_type");
	___mb.mb_set_igd_status = godot::api->godot_method_bind_get_method("UPNPDevice", "set_igd_status");
	___mb.mb_set_service_type = godot::api->godot_method_bind_get_method("UPNPDevice", "set_service_type");
}

UPNPDevice *UPNPDevice::_new()
{
	return (UPNPDevice *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"UPNPDevice")());
}
int64_t UPNPDevice::add_port_mapping(const int64_t port, const int64_t port_internal, const String desc, const String proto, const int64_t duration) const {
	return ___godot_icall_int_int_int_String_String_int(___mb.mb_add_port_mapping, (const Object *) this, port, port_internal, desc, proto, duration);
}

int64_t UPNPDevice::delete_port_mapping(const int64_t port, const String proto) const {
	return ___godot_icall_int_int_String(___mb.mb_delete_port_mapping, (const Object *) this, port, proto);
}

String UPNPDevice::get_description_url() const {
	return ___godot_icall_String(___mb.mb_get_description_url, (const Object *) this);
}

String UPNPDevice::get_igd_control_url() const {
	return ___godot_icall_String(___mb.mb_get_igd_control_url, (const Object *) this);
}

String UPNPDevice::get_igd_our_addr() const {
	return ___godot_icall_String(___mb.mb_get_igd_our_addr, (const Object *) this);
}

String UPNPDevice::get_igd_service_type() const {
	return ___godot_icall_String(___mb.mb_get_igd_service_type, (const Object *) this);
}

UPNPDevice::IGDStatus UPNPDevice::get_igd_status() const {
	return (UPNPDevice::IGDStatus) ___godot_icall_int(___mb.mb_get_igd_status, (const Object *) this);
}

String UPNPDevice::get_service_type() const {
	return ___godot_icall_String(___mb.mb_get_service_type, (const Object *) this);
}

bool UPNPDevice::is_valid_gateway() const {
	return ___godot_icall_bool(___mb.mb_is_valid_gateway, (const Object *) this);
}

String UPNPDevice::query_external_address() const {
	return ___godot_icall_String(___mb.mb_query_external_address, (const Object *) this);
}

void UPNPDevice::set_description_url(const String url) {
	___godot_icall_void_String(___mb.mb_set_description_url, (const Object *) this, url);
}

void UPNPDevice::set_igd_control_url(const String url) {
	___godot_icall_void_String(___mb.mb_set_igd_control_url, (const Object *) this, url);
}

void UPNPDevice::set_igd_our_addr(const String addr) {
	___godot_icall_void_String(___mb.mb_set_igd_our_addr, (const Object *) this, addr);
}

void UPNPDevice::set_igd_service_type(const String type) {
	___godot_icall_void_String(___mb.mb_set_igd_service_type, (const Object *) this, type);
}

void UPNPDevice::set_igd_status(const int64_t status) {
	___godot_icall_void_int(___mb.mb_set_igd_status, (const Object *) this, status);
}

void UPNPDevice::set_service_type(const String type) {
	___godot_icall_void_String(___mb.mb_set_service_type, (const Object *) this, type);
}

}