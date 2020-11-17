#include "IP.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


IP *IP::_singleton = NULL;


IP::IP() {
	_owner = godot::api->godot_global_get_singleton((char *) "IP");
}


IP::___method_bindings IP::___mb = {};

void IP::___init_method_bindings() {
	___mb.mb_clear_cache = godot::api->godot_method_bind_get_method("IP", "clear_cache");
	___mb.mb_erase_resolve_item = godot::api->godot_method_bind_get_method("IP", "erase_resolve_item");
	___mb.mb_get_local_addresses = godot::api->godot_method_bind_get_method("IP", "get_local_addresses");
	___mb.mb_get_local_interfaces = godot::api->godot_method_bind_get_method("IP", "get_local_interfaces");
	___mb.mb_get_resolve_item_address = godot::api->godot_method_bind_get_method("IP", "get_resolve_item_address");
	___mb.mb_get_resolve_item_status = godot::api->godot_method_bind_get_method("IP", "get_resolve_item_status");
	___mb.mb_resolve_hostname = godot::api->godot_method_bind_get_method("IP", "resolve_hostname");
	___mb.mb_resolve_hostname_queue_item = godot::api->godot_method_bind_get_method("IP", "resolve_hostname_queue_item");
}

void IP::clear_cache(const String hostname) {
	___godot_icall_void_String(___mb.mb_clear_cache, (const Object *) this, hostname);
}

void IP::erase_resolve_item(const int64_t id) {
	___godot_icall_void_int(___mb.mb_erase_resolve_item, (const Object *) this, id);
}

Array IP::get_local_addresses() const {
	return ___godot_icall_Array(___mb.mb_get_local_addresses, (const Object *) this);
}

Array IP::get_local_interfaces() const {
	return ___godot_icall_Array(___mb.mb_get_local_interfaces, (const Object *) this);
}

String IP::get_resolve_item_address(const int64_t id) const {
	return ___godot_icall_String_int(___mb.mb_get_resolve_item_address, (const Object *) this, id);
}

IP::ResolverStatus IP::get_resolve_item_status(const int64_t id) const {
	return (IP::ResolverStatus) ___godot_icall_int_int(___mb.mb_get_resolve_item_status, (const Object *) this, id);
}

String IP::resolve_hostname(const String host, const int64_t ip_type) {
	return ___godot_icall_String_String_int(___mb.mb_resolve_hostname, (const Object *) this, host, ip_type);
}

int64_t IP::resolve_hostname_queue_item(const String host, const int64_t ip_type) {
	return ___godot_icall_int_String_int(___mb.mb_resolve_hostname_queue_item, (const Object *) this, host, ip_type);
}

}