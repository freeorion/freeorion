#ifndef GODOT_CPP_IP_HPP
#define GODOT_CPP_IP_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "IP.hpp"

#include "Object.hpp"
namespace godot {


class IP : public Object {
	static IP *_singleton;

	IP();

	struct ___method_bindings {
		godot_method_bind *mb_clear_cache;
		godot_method_bind *mb_erase_resolve_item;
		godot_method_bind *mb_get_local_addresses;
		godot_method_bind *mb_get_local_interfaces;
		godot_method_bind *mb_get_resolve_item_address;
		godot_method_bind *mb_get_resolve_item_status;
		godot_method_bind *mb_resolve_hostname;
		godot_method_bind *mb_resolve_hostname_queue_item;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline IP *get_singleton()
	{
		if (!IP::_singleton) {
			IP::_singleton = new IP;
		}
		return IP::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "IP"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum ResolverStatus {
		RESOLVER_STATUS_NONE = 0,
		RESOLVER_STATUS_WAITING = 1,
		RESOLVER_STATUS_DONE = 2,
		RESOLVER_STATUS_ERROR = 3,
	};
	enum Type {
		TYPE_NONE = 0,
		TYPE_IPV4 = 1,
		TYPE_IPV6 = 2,
		TYPE_ANY = 3,
	};

	// constants
	const static int RESOLVER_INVALID_ID = -1;
	const static int RESOLVER_MAX_QUERIES = 32;

	// methods
	void clear_cache(const String hostname = "");
	void erase_resolve_item(const int64_t id);
	Array get_local_addresses() const;
	Array get_local_interfaces() const;
	String get_resolve_item_address(const int64_t id) const;
	IP::ResolverStatus get_resolve_item_status(const int64_t id) const;
	String resolve_hostname(const String host, const int64_t ip_type = 3);
	int64_t resolve_hostname_queue_item(const String host, const int64_t ip_type = 3);

};

}

#endif