#ifndef GODOT_CPP_XMLPARSER_HPP
#define GODOT_CPP_XMLPARSER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "XMLParser.hpp"

#include "Reference.hpp"
namespace godot {


class XMLParser : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_attribute_count;
		godot_method_bind *mb_get_attribute_name;
		godot_method_bind *mb_get_attribute_value;
		godot_method_bind *mb_get_current_line;
		godot_method_bind *mb_get_named_attribute_value;
		godot_method_bind *mb_get_named_attribute_value_safe;
		godot_method_bind *mb_get_node_data;
		godot_method_bind *mb_get_node_name;
		godot_method_bind *mb_get_node_offset;
		godot_method_bind *mb_get_node_type;
		godot_method_bind *mb_has_attribute;
		godot_method_bind *mb_is_empty;
		godot_method_bind *mb_open;
		godot_method_bind *mb_open_buffer;
		godot_method_bind *mb_read;
		godot_method_bind *mb_seek;
		godot_method_bind *mb_skip_section;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "XMLParser"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum NodeType {
		NODE_NONE = 0,
		NODE_ELEMENT = 1,
		NODE_ELEMENT_END = 2,
		NODE_TEXT = 3,
		NODE_COMMENT = 4,
		NODE_CDATA = 5,
		NODE_UNKNOWN = 6,
	};

	// constants


	static XMLParser *_new();

	// methods
	int64_t get_attribute_count() const;
	String get_attribute_name(const int64_t idx) const;
	String get_attribute_value(const int64_t idx) const;
	int64_t get_current_line() const;
	String get_named_attribute_value(const String name) const;
	String get_named_attribute_value_safe(const String name) const;
	String get_node_data() const;
	String get_node_name() const;
	int64_t get_node_offset() const;
	XMLParser::NodeType get_node_type();
	bool has_attribute(const String name) const;
	bool is_empty() const;
	Error open(const String file);
	Error open_buffer(const PoolByteArray buffer);
	Error read();
	Error seek(const int64_t position);
	void skip_section();

};

}

#endif