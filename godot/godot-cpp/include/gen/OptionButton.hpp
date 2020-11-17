#ifndef GODOT_CPP_OPTIONBUTTON_HPP
#define GODOT_CPP_OPTIONBUTTON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Button.hpp"
namespace godot {

class Texture;
class PopupMenu;

class OptionButton : public Button {
	struct ___method_bindings {
		godot_method_bind *mb__focused;
		godot_method_bind *mb__get_items;
		godot_method_bind *mb__select_int;
		godot_method_bind *mb__selected;
		godot_method_bind *mb__set_items;
		godot_method_bind *mb_add_icon_item;
		godot_method_bind *mb_add_item;
		godot_method_bind *mb_add_separator;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_get_item_count;
		godot_method_bind *mb_get_item_icon;
		godot_method_bind *mb_get_item_id;
		godot_method_bind *mb_get_item_index;
		godot_method_bind *mb_get_item_metadata;
		godot_method_bind *mb_get_item_text;
		godot_method_bind *mb_get_popup;
		godot_method_bind *mb_get_selected;
		godot_method_bind *mb_get_selected_id;
		godot_method_bind *mb_get_selected_metadata;
		godot_method_bind *mb_is_item_disabled;
		godot_method_bind *mb_remove_item;
		godot_method_bind *mb_select;
		godot_method_bind *mb_set_item_disabled;
		godot_method_bind *mb_set_item_icon;
		godot_method_bind *mb_set_item_id;
		godot_method_bind *mb_set_item_metadata;
		godot_method_bind *mb_set_item_text;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "OptionButton"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static OptionButton *_new();

	// methods
	void _focused(const int64_t arg0);
	Array _get_items() const;
	void _select_int(const int64_t arg0);
	void _selected(const int64_t arg0);
	void _set_items(const Array arg0);
	void add_icon_item(const Ref<Texture> texture, const String label, const int64_t id = -1);
	void add_item(const String label, const int64_t id = -1);
	void add_separator();
	void clear();
	int64_t get_item_count() const;
	Ref<Texture> get_item_icon(const int64_t idx) const;
	int64_t get_item_id(const int64_t idx) const;
	int64_t get_item_index(const int64_t id) const;
	Variant get_item_metadata(const int64_t idx) const;
	String get_item_text(const int64_t idx) const;
	PopupMenu *get_popup() const;
	int64_t get_selected() const;
	int64_t get_selected_id() const;
	Variant get_selected_metadata() const;
	bool is_item_disabled(const int64_t idx) const;
	void remove_item(const int64_t idx);
	void select(const int64_t idx);
	void set_item_disabled(const int64_t idx, const bool disabled);
	void set_item_icon(const int64_t idx, const Ref<Texture> texture);
	void set_item_id(const int64_t idx, const int64_t id);
	void set_item_metadata(const int64_t idx, const Variant metadata);
	void set_item_text(const int64_t idx, const String text);

};

}

#endif