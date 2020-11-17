#include "UPNP.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "UPNPDevice.hpp"


namespace godot {


UPNP::___method_bindings UPNP::___mb = {};

void UPNP::___init_method_bindings() {
	___mb.mb_add_device = godot::api->godot_method_bind_get_method("UPNP", "add_device");
	___mb.mb_add_port_mapping = godot::api->godot_method_bind_get_method("UPNP", "add_port_mapping");
	___mb.mb_clear_devices = godot::api->godot_method_bind_get_method("UPNP", "clear_devices");
	___mb.mb_delete_port_mapping = godot::api->godot_method_bind_get_method("UPNP", "delete_port_mapping");
	___mb.mb_discover = godot::api->godot_method_bind_get_method("UPNP", "discover");
	___mb.mb_get_device = godot::api->godot_method_bind_get_method("UPNP", "get_device");
	___mb.mb_get_device_count = godot::api->godot_method_bind_get_method("UPNP", "get_device_count");
	___mb.mb_get_discover_local_port = godot::api->godot_method_bind_get_method("UPNP", "get_discover_local_port");
	___mb.mb_get_discover_multicast_if = godot::api->godot_method_bind_get_method("UPNP", "get_discover_multicast_if");
	___mb.mb_get_gateway = godot::api->godot_method_bind_get_method("UPNP", "get_gateway");
	___mb.mb_is_discover_ipv6 = godot::api->godot_method_bind_get_method("UPNP", "is_discover_ipv6");
	___mb.mb_query_external_address = godot::api->godot_method_bind_get_method("UPNP", "query_external_address");
	___mb.mb_remove_device = godot::api->godot_method_bind_get_method("UPNP", "remove_device");
	___mb.mb_set_device = godot::api->godot_method_bind_get_method("UPNP", "set_device");
	___mb.mb_set_discover_ipv6 = godot::api->godot_method_bind_get_method("UPNP", "set_discover_ipv6");
	___mb.mb_set_discover_local_port = godot::api->godot_method_bind_get_method("UPNP", "set_discover_local_port");
	___mb.mb_set_discover_multicast_if = godot::api->godot_method_bind_get_method("UPNP", "set_discover_multicast_if");
}

UPNP *UPNP::_new()
{
	return (UPNP *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"UPNP")());
}
void UPNP::add_device(const Ref<UPNPDevice> device) {
	___godot_icall_void_Object(___mb.mb_add_device, (const Object *) this, device.ptr());
}

int64_t UPNP::add_port_mapping(const int64_t port, const int64_t port_internal, const String desc, const String proto, const int64_t duration) const {
	return ___godot_icall_int_int_int_String_String_int(___mb.mb_add_port_mapping, (const Object *) this, port, port_internal, desc, proto, duration);
}

void UPNP::clear_devices() {
	___godot_icall_void(___mb.mb_clear_devices, (const Object *) this);
}

int64_t UPNP::delete_port_mapping(const int64_t port, const String proto) const {
	return ___godot_icall_int_int_String(___mb.mb_delete_port_mapping, (const Object *) this, port, proto);
}

int64_t UPNP::discover(const int64_t timeout, const int64_t ttl, const String device_filter) {
	return ___godot_icall_int_int_int_String(___mb.mb_discover, (const Object *) this, timeout, ttl, device_filter);
}

Ref<UPNPDevice> UPNP::get_device(const int64_t index) const {
	return Ref<UPNPDevice>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_device, (const Object *) this, index));
}

int64_t UPNP::get_device_count() const {
	return ___godot_icall_int(___mb.mb_get_device_count, (const Object *) this);
}

int64_t UPNP::get_discover_local_port() const {
	return ___godot_icall_int(___mb.mb_get_discover_local_port, (const Object *) this);
}

String UPNP::get_discover_multicast_if() const {
	return ___godot_icall_String(___mb.mb_get_discover_multicast_if, (const Object *) this);
}

Ref<UPNPDevice> UPNP::get_gateway() const {
	return Ref<UPNPDevice>::__internal_constructor(___godot_icall_Object(___mb.mb_get_gateway, (const Object *) this));
}

bool UPNP::is_discover_ipv6() const {
	return ___godot_icall_bool(___mb.mb_is_discover_ipv6, (const Object *) this);
}

String UPNP::query_external_address() const {
	return ___godot_icall_String(___mb.mb_query_external_address, (const Object *) this);
}

void UPNP::remove_device(const int64_t index) {
	___godot_icall_void_int(___mb.mb_remove_device, (const Object *) this, index);
}

void UPNP::set_device(const int64_t index, const Ref<UPNPDevice> device) {
	___godot_icall_void_int_Object(___mb.mb_set_device, (const Object *) this, index, device.ptr());
}

void UPNP::set_discover_ipv6(const bool ipv6) {
	___godot_icall_void_bool(___mb.mb_set_discover_ipv6, (const Object *) this, ipv6);
}

void UPNP::set_discover_local_port(const int64_t port) {
	___godot_icall_void_int(___mb.mb_set_discover_local_port, (const Object *) this, port);
}

void UPNP::set_discover_multicast_if(const String m_if) {
	___godot_icall_void_String(___mb.mb_set_discover_multicast_if, (const Object *) this, m_if);
}

}