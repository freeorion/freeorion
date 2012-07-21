/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// presets.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_presets.hpp>
#include <GG/adobe/future/widgets/headers/presets_common.hpp>

#include <GG/adobe/future/widgets/headers/popup_common.hpp>
#include <GG/adobe/future/widgets/headers/presets_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <cassert>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void imbue_preset(sheet_t& model, const any_regular_t& value)
{
    // when a preset is selected from the list, you're going to get a dictionary of the
    // actual property model values that need to get propagated to the property model.

    if (value.type_info() != type_info<dictionary_t>())
        return;

    model.set(value.cast<dictionary_t>());
        model.update();
}

/****************************************************************************************************/

struct additional_presets_view_t
{
    typedef array_t model_type;

    explicit additional_presets_view_t(presets_t& popup) :
        preset_m(popup)
    { }

    void display(const model_type& value)
    { preset_m.display_additional_preset_set(value); }

    presets_t& preset_m;
};

/****************************************************************************************************/

struct preset_popup_menu_item_set_view_t
{
    typedef array_t model_type;

    preset_popup_menu_item_set_view_t(presets_t& preset, sheet_t& sheet) :
        preset_m(preset),
        sheet_m(sheet)
    { }

    void display(const model_type& value)
    {
        popup_t::menu_item_set_t set(array_to_menu_item_set(value));

        preset_m.popup_m.reset_menu_item_set(set);

        if (set.empty())
            return;

        imbue_preset(sheet_m, set[0].second);
    }

    presets_t& preset_m;
    sheet_t&   sheet_m;
};

/****************************************************************************************************/

void attach_preset_popup_menu_item_set(presets_t&         control,
                                       name_t             cell,
                                       sheet_t&           sheet,
                                       basic_sheet_t&     layout_sheet,
                                       assemblage_t&      assemblage,
                                       eve_client_holder& /*client_holder*/)
{
    typedef preset_popup_menu_item_set_view_t view_type;

    view_type* tmp = new view_type(control, sheet);

    assemblage.cleanup(boost::bind(delete_ptr<view_type*>(),tmp));

    attach_view(assemblage, cell, *tmp, layout_sheet);
}

/****************************************************************************************************/

namespace implementation {
    
/****************************************************************************************************/

dictionary_t model_snapshot(sheet_t& model, const dictionary_t& mark)
{
    dictionary_t result;

    result.insert(std::make_pair(key_preset_value, model.contributing(mark)));

    return result;
}

/****************************************************************************************************/

} //namespace implementation

/*************************************************************************************************/

void create_widget(const dictionary_t&   parameters,
                   size_enum_t           size,
                   presets_t*&           widget)
{
    std::string         name;
    std::string         domain;
    std::string         alt_text;
    array_t      bind_set;
    dictionary_t localization_set;
    theme_t      theme(implementation::size_to_theme(size));

    get_value(parameters, key_name, name);
    get_value(parameters, key_domain, domain);
    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, key_bind, bind_set);
    get_value(parameters, key_localization_set, localization_set);

    widget = new presets_t(name, domain, alt_text, bind_set, localization_set, theme);
}

/****************************************************************************************************/

template <>
void attach_view_and_controller(presets_t&             control,
                                const dictionary_t&    parameters,
                                const factory_token_t& token,
                                adobe::name_t,
                                adobe::name_t,
                                adobe::name_t)
{
    basic_sheet_t& layout_sheet(token.client_holder_m.layout_sheet_m);
    assemblage_t&  assemblage(token.client_holder_m.assemblage_m);

    // the first thing we do is create a state cell for the preset to communicate info
    // back and forth between its two popups. This is done in the basic sheet.

    name_t state_cell(state_cell_unique_name());

    {
        // create the cell and set it to an empty array (menu item set) for now
        layout_sheet.add_interface(state_cell, any_regular_t(array_t()));

        // attach the category popup as a bonafide view/controller to the state cell
        attach_monitor(control.category_popup_m, state_cell, 
            layout_sheet, token, parameters);
        attach_view(assemblage,
                           state_cell,
                           control.category_popup_m,
                           layout_sheet);

        // attaches the preset popup's menu item set as a view to the state cell
        attach_preset_popup_menu_item_set(control, state_cell, token.sheet_m, 
                                          layout_sheet, assemblage,
                                          token.client_holder_m);
    }

    if (parameters.count(key_bind))
    {
        // next set up the snapshot proc: the proc that will be used by the preset widget to
        // get the currently contributing values to the property model when the user wants to
        // add a preset to their list.
        control.snapshot_callback(boost::bind(implementation::model_snapshot,
                                              boost::ref(token.sheet_m),
                                              token.sheet_m.contributing()));

        // next set up the controller portion of the preset: the part that, when the user
        // selects a preset from the list, will imbue the model with the values in the preset.
        control.monitor(boost::bind(imbue_preset, boost::ref(token.sheet_m), _1));
    }

    if (parameters.count(key_bind_output))
    {
        name_t cell(get_value(parameters, key_bind_output).cast<name_t>());

        if (layout_sheet.count_interface(cell) != 0)
        {
            attach_view(assemblage, cell, control, layout_sheet);
        }
        else
        {
            assemblage.cleanup(boost::bind(&boost::signals::connection::disconnect, token.sheet_m.monitor_invariant_dependent(cell, 
                boost::bind(static_cast<void (*)(presets_t&, bool)>(&enable), 
                    boost::ref(control), _1))));

            attach_view(assemblage, cell, control, token.sheet_m);
        }
    }

    if (parameters.count(key_bind_additional))
    {
        // next set up the view portion of the preset: the part that, should the property
        // model house "metapresets" (runtime-generated presets) within, allows for the
        // widget to obtain these metapresets and modify its display.

        name_t              cell(get_value(parameters, key_bind_additional).cast<name_t>());
        additional_presets_view_t* tmp = new additional_presets_view_t(control);

        assemblage.cleanup(boost::bind(delete_ptr<additional_presets_view_t*>(), tmp));

        if (layout_sheet.count_interface(cell) != 0)
            attach_view(assemblage, cell, *tmp, layout_sheet);
        else
            attach_view(assemblage, cell, *tmp, token.sheet_m);
    }

    implementation::reload_preset_widget(control);
}
    
/****************************************************************************************************/

widget_node_t make_presets(const dictionary_t&     parameters,
                           const widget_node_t&    parent,
                           const factory_token_t&  token,
                           const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<presets_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("preset")), 
        factory.layout_attributes(static_name_t("preset"))); 
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
