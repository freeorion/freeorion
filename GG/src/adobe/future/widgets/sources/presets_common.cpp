/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#define ADOBE_CLOSED_HASH_MAP_INDEX 1

#include <GG/adobe/future/widgets/headers/platform_presets.hpp>
#include <GG/adobe/future/widgets/headers/presets_common.hpp>

#include <sstream>

#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <GG/adobe/once.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/virtual_machine.hpp>
#include <GG/adobe/future/resources.hpp>
#include <GG/adobe/future/modal_dialog_interface.hpp>
#include <GG/adobe/future/widgets/headers/alert.hpp>
#include <GG/adobe/future/widgets/headers/popup_common.hpp>
#include <GG/adobe/implementation/expression_parser.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/next_prior.hpp>

#if defined(ADOBE_STD_SERIALIZATION)
    #include <GG/adobe/iomanip_asl_cel.hpp>
#endif

#include <GG/Button.h>


/****************************************************************************************************/

ADOBE_ONCE_DECLARATION(presets_once)

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

ADOBE_THREAD_LOCAL_STORAGE(adobe::virtual_machine_t, presets_vm)

/*************************************************************************************************/

void init_presets_once()
{
    // initialize the thread-specific virtual machine

    ADOBE_THREAD_LOCAL_STORAGE_INITIALIZE(presets_vm);
}

/****************************************************************************************************/

inline bool always_break(adobe::name_t, const adobe::any_regular_t&)
    { return true; }

/****************************************************************************************************/

std::string temp_cell_name()
{
    // this takes an increasing counter and converts it to base 26,
    // turning it into a string, and returning the string

    static long count(0);
    std::string result;
    long        tmp(++count);

    while(tmp != 0)
    {
        long n(tmp / 26);
        long r(tmp % 26);

        tmp = n;        

        result += static_cast<char>('a' + r);
    }

    return result;
}

/****************************************************************************************************/

void append_definitions(const adobe::array_t& element,
                        std::string&          preset_dialog_model,
                        std::string&          preset_layout,
                        std::string&          preset_dialog_model_result_footer)
{
    if (element.size() != 2)
        return;
    else if (element[1].type_info() != adobe::type_info<adobe::string_t>())
        return;

    adobe::array_t cell_array;

    if (element[0].type_info() == adobe::type_info<adobe::name_t>())
        cell_array.push_back(element[0]);
    else if (element[0].type_info() == adobe::type_info(cell_array))
        cell_array = element[0].cast<adobe::array_t>();
    else
        throw std::runtime_error("Preset: Expected either a name or an array of names in item");

    std::string              cell(temp_cell_name());
    static const std::string model_interface_cell_decl_suffix(" : true;\n");
    static const std::string comma_space(", ");
    static const std::string spaced_colon(" : ");

    preset_dialog_model << cell + model_interface_cell_decl_suffix;
    preset_layout << "checkbox(name: \"" << element[1].cast<std::string>() << "\", bind: @" << cell << ");\n";

    for (adobe::array_t::const_iterator first(cell_array.begin()), last(cell_array.end()); first != last; ++first)
        preset_dialog_model_result_footer << comma_space << first->cast<adobe::name_t>().c_str() << spaced_colon << cell;
}

/****************************************************************************************************/

adobe::array_t preset_stream_to_array(std::istream& instream, const std::string& stream_name)
{
    adobe::array_t result;

    try
    {
        if (instream.fail())
            return result;

        ADOBE_ONCE_INSTANCE(presets_once);

#if defined(ADOBE_STD_SERIALIZATION)
        adobe::expression_parser parser(instream, adobe::line_position_t(stream_name.c_str()));
        adobe::array_t           expression;
    
        if (!parser.is_expression(expression))
            return result;
    
        ADOBE_THREAD_LOCAL_STORAGE_ACCESS(presets_vm).evaluate(expression);
    
        ADOBE_THREAD_LOCAL_STORAGE_ACCESS(presets_vm).back().cast(result);
    
        ADOBE_THREAD_LOCAL_STORAGE_ACCESS(presets_vm).pop_back();
#endif
    }
    catch(...)
    {
        // REVISIT (fbrereto) : silent failure. This, however, is better than 
        //                      the preset widget preventing the dialog from
        //                      coming up at all...
    }

    return result;
}

/****************************************************************************************************/

void populate_category_popup(adobe::presets_t& control, const adobe::array_t& contents)
{
    typedef adobe::array_t::const_iterator              iterator;
    typedef adobe::popup_t::menu_item_set_t::value_type value_type;

    adobe::popup_t::menu_item_set_t menu_item_set;

    // populate items in the popup with the presets found
    for (iterator first(contents.begin()), last(contents.end()); first != last; ++first)
    {
        const adobe::dictionary_t& cur_category(first->cast<adobe::dictionary_t>());
        const std::string&         cur_preset_name(get_value(cur_category, adobe::key_preset_name).cast<std::string>());

        if (cur_preset_name == std::string())
            continue;

        adobe::any_regular_t preset_set; // could be empty (e.g., in a separator)

        get_value(cur_category, adobe::key_preset_items, preset_set);

        menu_item_set.push_back(value_type(cur_preset_name, preset_set));
    }

    control.type_2_debounce_m = true;

    control.category_popup_m.reset_menu_item_set(menu_item_set);

    control.type_2_debounce_m = false;
}

/******************************************************************************/

const adobe::dictionary_t& preset_menu_item_separator()
{
    static adobe::dictionary_t separator_s;
    static bool                inited(false);

    if (!inited)
    {
                adobe::any_regular_t unlikely((std::numeric_limits<double>::min)());
                adobe::array_t       unlikely_array;

                unlikely_array.push_back(unlikely);

        separator_s.insert(std::make_pair(adobe::key_preset_name, adobe::any_regular_t(std::string("-"))));

        separator_s.insert(std::make_pair(adobe::key_preset_items, adobe::any_regular_t(unlikely_array)));

        inited = true;
    }

    return separator_s;
}

/****************************************************************************************************/

boost::filesystem::path user_preset_filepath(adobe::presets_t& control)
{
    std::string user_preset_file_extension(adobe::implementation::localization_value(control, adobe::key_preset_file_extension, ".presets"));
    std::string user_preset_filename;

    user_preset_filename << control.name_m << user_preset_file_extension;

    boost::filesystem::path result(adobe::implementation::preset_directory(control) / user_preset_filename);

    return result;
}

/****************************************************************************************************/

adobe::array_t compose_category_menu(adobe::presets_t& control)
{
    if (control.default_preset_set_m.empty())
        control.default_preset_set_m = adobe::implementation::load_default_preset_set(control);

    adobe::array_t result;
    
    if (control.dynamic_preset_set_m.empty() == false)
    {
        result.insert(result.end(), boost::begin(control.dynamic_preset_set_m),
                                    boost::end(control.dynamic_preset_set_m));

        result.push_back(adobe::any_regular_t(preset_menu_item_separator()));
    }

    if (control.default_preset_set_m.empty() == false)
        result.insert(result.end(), boost::begin(control.default_preset_set_m),
                                    boost::end(control.default_preset_set_m));

    control.user_preset_set_m = adobe::implementation::load_user_preset_set(control);

    if (!control.default_preset_set_m.empty() && !control.user_preset_set_m.empty())
        result.push_back(adobe::any_regular_t(preset_menu_item_separator()));

    if (control.user_preset_set_m.empty() == false)
        result.insert(result.end(), boost::begin(control.user_preset_set_m),
                                    boost::end(control.user_preset_set_m));

    return result;
}

/****************************************************************************************************/

bool has_collisions(const adobe::dictionary_t& x, const adobe::dictionary_t& y)
{
    adobe::dictionary_t::const_iterator iter(x.begin());
    adobe::dictionary_t::const_iterator last(x.end());

    for (; iter != last; ++iter)
    {
        if (y.count(iter->first) == 0)
            continue;

        if (get_value(y, iter->first) != iter->second)
            return true;
    }

    return false;
}

/****************************************************************************************************/

bool is_subset(const adobe::dictionary_t& x, const adobe::dictionary_t& y)
{
    // tests to see if y is a subset within x

    adobe::dictionary_t::const_iterator iter(y.begin());
    adobe::dictionary_t::const_iterator last(y.end());

    for (; iter != last; ++iter)
    {
        // if x doesn't have a key that's in y, y isn't a subset of x
        if (x.count(iter->first) == 0)
            return false;

        // if x has the key but the value isn't the same, y isn't a subset of x
        if (get_value(x, iter->first) != iter->second)
            return false;
    }

    return true;
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void presets_t::display_additional_preset_set(const array_t& value)
{
    if (value == dynamic_preset_set_m)
        return;

    dynamic_preset_set_m = value;

    implementation::reload_preset_widget(*this); // refresh the control with the updated preset set
    display(last_m);                             // redisplay the last preset we had selected
}

/****************************************************************************************************/

void presets_t::display(const model_type& value)
{
    // last_m is not intended to do type 1 debouncing; it is used so
    // we can refresh the display if/when the preset set has been
    // modified.
    last_m = value;

    array_t::const_iterator cat_iter(composite_m.begin());
    array_t::const_iterator cat_last(composite_m.end());
    array_t                 match_category;
    dictionary_t            match_preset;
    std::size_t                    match_value(0);

    for (; cat_iter != cat_last; ++cat_iter)
    {
        dictionary_t category_dictionary;
        array_t      category_items;

        cat_iter->cast(category_dictionary);

        get_value(category_dictionary, key_preset_items, category_items);

        array_t::const_iterator preset_iter(category_items.begin());
        array_t::const_iterator preset_last(category_items.end());

#ifndef NDEBUG
        std::string _debug_category_name;

        get_value(category_dictionary, key_preset_name, _debug_category_name);
#endif

        for (; preset_iter != preset_last; ++preset_iter)
        {
            dictionary_t preset_dictionary;
            dictionary_t preset_value;

            preset_iter->cast(preset_dictionary);

            get_value(preset_dictionary, key_preset_value, preset_value);

#ifndef NDEBUG
            std::string _debug_preset_name;

            get_value(preset_dictionary, key_preset_name, _debug_preset_name);
#endif

            if (preset_value.empty())
                continue;

            if (has_collisions(preset_value, value))
                continue;

            if (!is_subset(value, preset_value))
                continue;

            if (preset_value.size() > match_value)
            {
                match_value = preset_value.size();
                match_preset = preset_value;
                match_category = category_items;
            }
        }
    }

    if (match_value != 0)
    {
        custom_m = false;

        type_2_debounce_m = true;

        category_popup_m.display(any_regular_t(match_category));

        popup_m.reset_menu_item_set(array_to_menu_item_set(match_category));

        popup_m.display(any_regular_t(match_preset));

        type_2_debounce_m = false;
    }
    else
    {
        // Could not find a preset that matched the state given.
        // Show the custom state.
        display_custom();
    }
}

/****************************************************************************************************/

void presets_t::display_custom()
{
    // already showing the custom state -- no need to proceed.
    if (custom_m)
        return;

    type_2_debounce_m = true;

    category_popup_m.display(any_regular_t((std::numeric_limits<double>::min)()));

    popup_m.display(any_regular_t((std::numeric_limits<double>::min)()));

    popup_m.reset_menu_item_set(popup_t::menu_item_set_t());

    type_2_debounce_m = false;
}

/****************************************************************************************************/

void presets_t::do_imbue(const popup_t::model_type& value)
{
    if (type_2_debounce_m)
        return;

    if (proc_m)
        proc_m(value);
}

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

void reload_preset_widget(presets_t& control)
{
    control.custom_m = false;

    control.composite_m = compose_category_menu(control);

    populate_category_popup(control, control.composite_m);
}

/****************************************************************************************************/

void append_user_preset(presets_t& control, const dictionary_t& snapshot)
{
    std::string adam_header;
    std::string eve_header;
    std::string eve_footer;

    adam_header <<
        "sheet save_preset\n"
        "{\n"
        "interface:\n"
        "   preset__name__ : '" << localization_value(control, key_preset_add_subdialog_default_preset_name, "My Preset") << "';\n"
        ;

    const char* adam_footer =
        "}\n"
        ;

    eve_header <<
        "layout save_preset\n"
        "{\n"
        "    view dialog(name: '" << localization_value(control, key_preset_add_dialog_name, "Add A Preset") << "', placement: place_row)\n"
        "    {\n"
        "        column()\n"
        "        {\n"
        "            edit_text(name: 'Name:', bind: @preset__name__, horizontal: align_fill);\n"
        "            group(name: '" << localization_value(control, key_preset_add_dialog_group_name, "Include in Preset") << "', placement: place_row)\n"
        "            {\n"
        "               column()\n"
        "               {\n"
        ;

    eve_footer <<
        "               }\n"
        "            }\n"
        "            static_text(name: '" << localization_value(control, key_preset_add_dialog_message, "Values not included in the saved preset will default to their last used value.") << "', characters: 10, horizontal: align_fill);\n"
        "        }\n"
        "        column(vertical: align_fill, child_horizontal: align_fill)\n"
        "        {\n"
        "            button(name: '" << localization_value(control, key_preset_subdialog_ok_button_name, "OK") << "',"
        "                   action: @ok, default: true, alt: '" << localization_value(control, key_preset_add_subdialog_ok_button_alt_text, "Save a preset with the selected settings") << "');\n"
        "            button(name: '" << localization_value(control, key_preset_subdialog_cancel_button_name, "Cancel") << "',"
        "                   action: @cancel, cancel: true, alt: '" << localization_value(control, key_preset_add_subdialog_cancel_button_alt_text, "Do not save a preset") << "');\n"
        "        }\n"
        "    }\n"
        "}\n"
        ;

    std::string preset_dialog_model(adam_header);
    std::string preset_dialog_model_result_footer("\noutput: result <== { preset__name__ : preset__name__ \n");
    std::string preset_layout(eve_header);

    std::size_t item_count = control.bind_set_m.size();
    std::size_t half_count = (item_count + 1) / 2;

    for (array_t::const_iterator first(control.bind_set_m.begin()), last(first + half_count); first != last; ++first)
    {
        if (first->type_info() != type_info<array_t>())
            throw std::runtime_error("Preset: expected an array_t for the item in the preset");

        append_definitions(first->cast<array_t>(), preset_dialog_model, preset_layout, preset_dialog_model_result_footer);
    }

    preset_layout += "\n}\ncolumn()\n{\n";

    for (array_t::const_iterator first(control.bind_set_m.begin() + half_count), last(control.bind_set_m.end()); first != last; ++first)
    {
        if (first->type_info() != type_info<array_t>())
            throw std::runtime_error("Preset: expected an array_t for the item in the preset");

        append_definitions(first->cast<array_t>(), preset_dialog_model, preset_layout, preset_dialog_model_result_footer);
    }

    preset_dialog_model += preset_dialog_model_result_footer + " };\n" + adam_footer;
    preset_layout += eve_footer;

    std::istringstream     model(preset_dialog_model);
    std::istringstream     layout(preset_layout);
    dialog_result_t result;

    try
    {
        result = handle_dialog(dictionary_t(),
                                      dictionary_t(),
                                      dictionary_t(),
                                      dialog_display_s,
                                      layout,
                                      model,
                                      &always_break,
                                      boost::filesystem::path(),
                                      get_top_level_window(control.control_m));

        if (result.terminating_action_m == static_name_t("cancel"))
            return;
    }
    catch (const stream_error_t& err)
    {
        throw std::runtime_error(format_stream_error(err));
    }
    catch (...)
    {
        throw;
    }

    static_name_t preset_name("preset__name__");

    assert(result.command_m.count(preset_name));
    assert(snapshot.count(key_preset_value));
    assert(get_value(snapshot, key_preset_value).type_info() == type_info<dictionary_t>());

    std::string the_preset_name(get_value(result.command_m, preset_name).cast<std::string>());

    result.command_m.erase(preset_name);

    // For each cell in the result set that exists as being true,
    // copy them to the confirmed_result; finally, set the cell_set
    // in the snapshot to the confirmed_result.

    dictionary_t                  snapshot_cell_set(get_value(snapshot, key_preset_value).cast<dictionary_t>());
    dictionary_t                  confirmed_result;

    for (dictionary_t::const_iterator first(result.command_m.begin()), last(result.command_m.end());
         first != last; ++first)
    {
        assert(first->second.type_info() == type_info<bool>());

        name_t preset_attribute(first->first);
        bool          should_add(first->second.cast<bool>());

        if (should_add && snapshot_cell_set.count(preset_attribute))
            confirmed_result.insert(std::make_pair(preset_attribute, get_value(snapshot_cell_set, preset_attribute)));
    }

    dictionary_t final_preset(snapshot);

    final_preset.insert(std::make_pair(key_preset_value, confirmed_result));
    final_preset.insert(std::make_pair(key_preset_name, the_preset_name));

    array_t& items(control.user_preset_set_m[0].cast<dictionary_t>()[key_preset_items].cast<array_t>());

    items.push_back(any_regular_t(final_preset));

    save_user_preset_set(control);   // save the user preset file to disk
    reload_preset_widget(control);   // refresh the control with the updated preset set
    control.display(control.last_m); // redisplay the last preset we had selected
}

/****************************************************************************************************/

void delete_user_preset(presets_t& control)
{
    static const std::string model_definition =
        "sheet delete_preset\n"
        "{\n"
        "interface:\n"
        "   preset__name__ : 0;"
        "output:\n"
        "   result <== { preset__name__ : preset__name__ };"
        "}\n"
        ;

    std::string eve_header;
    std::string eve_footer;

    eve_header <<
        "layout delete_preset\n"
        "{\n"
        "    view dialog(name: '" << localization_value(control, key_preset_delete_dialog_name, "Delete A Preset") << "', placement: place_row)\n"
        "    {\n"
        "        column()\n"
        "        {\n"
        "            static_text(name: '" << localization_value(control, key_preset_delete_dialog_message, "Please select a preset you would like to delete. This operation cannot be undone.") << "', characters: 10, horizontal: align_fill);\n"
        "            popup(name: '" << localization_value(control, key_preset_preset_popup_name, "Preset:") << "', bind: @preset__name__, horizontal: align_fill, items: [\n"
        ;

    eve_footer <<
        "                  ]);\n"
        "        }\n"
        "        column(vertical: align_fill, child_horizontal: align_fill)\n"
        "        {\n"
        "            button(name: '" << localization_value(control, key_preset_subdialog_ok_button_name, "OK") << "', "
        "                   action: @ok, default: true, alt: '" << localization_value(control, key_preset_delete_subdialog_ok_button_alt_text, "Delete the selected preset") << "');\n"
        "            button(name: '" << localization_value(control, key_preset_subdialog_cancel_button_name, "Cancel") << "',"
        "                   action: @cancel, cancel: true, alt: '" << localization_value(control, key_preset_delete_subdialog_cancel_button_alt_text, "Do not delete a preset") << "');\n"
        "        }\n"
        "    }\n"
        "}\n"
        ;

    std::string     layout_definition(eve_header);
    array_t& items(control.user_preset_set_m[0].cast<dictionary_t>()[key_preset_items].cast<array_t>());

    if (items.empty())
    {
        alert(localization_value(control, key_preset_user_preset_list_empty_warning, "The custom preset list is empty.").c_str());

        return;
    }

    for (array_t::iterator first(items.begin()), last(items.end()); first != last; ++first)
    {
        std::stringstream appendage;

        appendage << "{ name: '"
                  << get_value(first->cast<dictionary_t>(), key_preset_name).cast<std::string>()
                  << "', value: "
                  << static_cast<long>(std::distance(items.begin(), first))
                  << "}";

        if (boost::next(first) != last)
            appendage << ",";

        appendage << "\n";

        layout_definition += appendage.str();
    }

    layout_definition += eve_footer;

    std::istringstream     model(model_definition);
    std::istringstream     layout(layout_definition);
    dialog_result_t result;

    try
    {
        result = handle_dialog(dictionary_t(),
                                      dictionary_t(),
                                      dictionary_t(),
                                      dialog_display_s,
                                      layout,
                                      model,
                                      &always_break,
                                      boost::filesystem::path(),
                                      get_top_level_window(control.control_m));

        if (result.terminating_action_m == static_name_t("cancel"))
            return;
    }
    catch (const stream_error_t& err)
    {
        throw std::runtime_error(format_stream_error(err));
    }
    catch (...)
    {
        throw;
    }

    static_name_t result_name("preset__name__");

    assert(result.command_m.count(result_name));

    long index = get_value(result.command_m, result_name).cast<long>();

    items.erase(items.begin() + index);

    save_user_preset_set(control);   // save the user preset file to disk
    reload_preset_widget(control);   // refresh the control with the updated preset set
    control.display(control.last_m); // redisplay the last preset we had selected
}

/****************************************************************************************************/

void save_user_preset_set(presets_t& control)
{
    boost::filesystem::path     user_preset_path(user_preset_filepath(control));
    boost::filesystem::ofstream outfile(user_preset_path);

    if (outfile.fail())
        return;

#if defined(ADOBE_STD_SERIALIZATION)
    outfile << begin_asl_cel << control.user_preset_set_m << end_asl_cel;
#endif
}

/****************************************************************************************************/

void append_user_preset_set(presets_t& control)
{
    boost::filesystem::path path;

    if (!implementation::pick_file(path))
        return;

    // read in and parse presets from file
    boost::filesystem::ifstream input(path);

    input.unsetf(std::ios_base::skipws);

    array_t new_user_preset_set(preset_stream_to_array(input, path.string().c_str()));
    
    control.user_preset_set_m.insert(control.user_preset_set_m.end(),
            boost::begin(new_user_preset_set),
            boost::end(new_user_preset_set));

    save_user_preset_set(control);   // save the user preset file to disk
    reload_preset_widget(control);   // refresh the control with the updated preset set
    control.display(control.last_m); // redisplay the last preset we had selected
}

/****************************************************************************************************/

array_t load_user_preset_set(presets_t& control)
{
    boost::filesystem::path     user_preset_path(user_preset_filepath(control));
    boost::filesystem::ifstream user_stream(user_preset_path);

    user_stream.unsetf(std::ios_base::skipws);

    array_t set(preset_stream_to_array(user_stream, "user preset file"));

    if (set.empty())
    {
        // add a barebones user preset category

        dictionary_t user_preset_category;
        array_t      items;
        std::string         name(implementation::localization_value(control, key_preset_user_presets_category_name, "User Presets"));

        user_preset_category.insert(std::make_pair(key_preset_items, items));
        user_preset_category.insert(std::make_pair(key_preset_name, name));

        push_back(set, user_preset_category);
    }

    return set;
}

/****************************************************************************************************/

array_t load_default_preset_set(presets_t& control)
{
    std::string                 user_preset_file_extension(implementation::localization_value(control, key_preset_file_extension, ".presets"));
    std::string                 user_preset_filename;

    user_preset_filename << control.name_m << user_preset_file_extension;

    try
    {
        boost::filesystem::path     defaults_file(find_resource(user_preset_filename));
        boost::filesystem::ifstream defaults_stream(defaults_file);
    
        if (defaults_stream.fail())
            return array_t();
    
        defaults_stream.unsetf(std::ios_base::skipws);
    
        return preset_stream_to_array(defaults_stream, defaults_file.string());
    }
    catch(...)
    {
    }

    return array_t();
}

/****************************************************************************************************/

std::string localization_value(const dictionary_t& set, name_t key, const std::string& default_string)
{
    return set.count(key) && get_value(set, key).type_info() == type_info<adobe::string_t>() ?
        std::string(get_value(set, key).cast<adobe::string_t>()) :
        default_string;
}

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

bool operator==(const presets_t& /*x*/, const presets_t& /*y*/)
{ return true; }

/****************************************************************************************************/

// these are for the localization set to be found in the eve description of the widget
aggregate_name_t key_preset_add_dialog_group_name = { "add_dialog_group_name" };
aggregate_name_t key_preset_add_dialog_message = { "add_dialog_message" };
aggregate_name_t key_preset_add_dialog_name = { "add_dialog_name" };
aggregate_name_t key_preset_add_subdialog_cancel_button_alt_text = { "add_subdialog_cancel_button_alt_text" };
aggregate_name_t key_preset_add_subdialog_default_preset_name = { "add_subdialog_default_preset_name" };
aggregate_name_t key_preset_add_subdialog_ok_button_alt_text = { "add_subdialog_ok_button_alt_text" };
aggregate_name_t key_preset_category_popup_alt_text = { "category_popup_alt_text" };
aggregate_name_t key_preset_category_popup_name = { "category_popup_name" };
aggregate_name_t key_preset_custom_category_name = { "custom_category_name" };
aggregate_name_t key_preset_delete_dialog_message = { "delete_dialog_message" };
aggregate_name_t key_preset_delete_dialog_name = { "delete_dialog_name" };
aggregate_name_t key_preset_delete_subdialog_cancel_button_alt_text = { "delete_subdialog_cancel_button_alt_text" };
aggregate_name_t key_preset_delete_subdialog_ok_button_alt_text = { "delete_subdialog_ok_button_alt_text" };
aggregate_name_t key_preset_file_extension = { "file_extension" };
aggregate_name_t key_preset_menu_item_add_preset = { "menu_item_add_preset" };
aggregate_name_t key_preset_menu_item_append_preset = { "menu_item_append_preset" };
aggregate_name_t key_preset_menu_item_delete_preset = { "menu_item_delete_preset" };
aggregate_name_t key_preset_preset_popup_alt_text = { "preset_popup_alt_text" };
aggregate_name_t key_preset_preset_popup_name = { "preset_popup_name" };
aggregate_name_t key_preset_subdialog_cancel_button_name = { "subdialog_cancel_button_name" };
aggregate_name_t key_preset_subdialog_ok_button_name = { "subdialog_ok_button_name" };
aggregate_name_t key_preset_top_folder_name = { "top_folder_name" };
aggregate_name_t key_preset_user_preset_list_empty_warning = { "user_preset_list_empty_warning" };
aggregate_name_t key_preset_user_presets_category_name = { "user_presets_category_name" };

aggregate_name_t key_preset_value = { "value" };
aggregate_name_t key_preset_name = { "name" };
aggregate_name_t key_preset_items = { "items" };

/****************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

ADOBE_ONCE_DEFINITION(presets_once, init_presets_once)

/****************************************************************************************************/

