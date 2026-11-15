#include "GodotOptionsDB.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/binder_common.hpp>

#include "../../util/OptionsDB.h"

void GodotOptionsDB::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("get_bool"), &GodotOptionsDB::get_bool);
    godot::ClassDB::bind_method(godot::D_METHOD("get_string"), &GodotOptionsDB::get_string);
    godot::ClassDB::bind_method(godot::D_METHOD("find_options", "prefix", "allow_unrecognized"), &GodotOptionsDB::find_options, DEFVAL(false));
    godot::ClassDB::bind_method(godot::D_METHOD("option_exists"), &GodotOptionsDB::option_exists);
    godot::ClassDB::bind_method(godot::D_METHOD("add_string"), &GodotOptionsDB::add_string);
    godot::ClassDB::bind_method(godot::D_METHOD("set_string"), &GodotOptionsDB::set_string);
    godot::ClassDB::bind_method(godot::D_METHOD("commit", "only_if_dirty", "only_non_default"),
                                &GodotOptionsDB::commit,
                                DEFVAL(true),
                                DEFVAL(true));
}

bool GodotOptionsDB::get_bool(godot::String option) const
{ return GetOptionsDB().Get<bool>(option.utf8().get_data()); }

godot::String GodotOptionsDB::get_string(godot::String option) const
{ return godot::String(GetOptionsDB().Get<std::string>(option.utf8().get_data()).c_str()); }

godot::PackedStringArray GodotOptionsDB::find_options(godot::String prefix, bool allow_unrecognized) const {
    godot::PackedStringArray options;
    for (const auto& option : GetOptionsDB().FindOptions(prefix.utf8().get_data(), allow_unrecognized))
        options.push_back(godot::String::utf8(option.data(), option.size()));
    return options;
}

bool GodotOptionsDB::option_exists(godot::String option) const
{ return GetOptionsDB().OptionExists(option.utf8().get_data()); }

void GodotOptionsDB::add_string(godot::String option, godot::String description, godot::String value) {
    GetOptionsDB().Add<std::string>(option.utf8().get_data(),
        description.utf8().get_data(),
        value.utf8().get_data());
}

void GodotOptionsDB::set_string(godot::String option, godot::String value)
{ GetOptionsDB().Set<std::string>(option.utf8().get_data(), value.utf8().get_data()); }

bool GodotOptionsDB::commit(bool only_if_dirty, bool only_non_default)
{ return GetOptionsDB().Commit(only_if_dirty, only_non_default); }



