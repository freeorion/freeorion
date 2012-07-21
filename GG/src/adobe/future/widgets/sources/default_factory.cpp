/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/adam.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/cmath.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/empty.hpp>
#include <GG/adobe/eve_evaluate.hpp>
#include <GG/adobe/eve_parser.hpp>
#include <GG/adobe/eve.hpp>
#include <GG/adobe/enum_ops.hpp>
#include <GG/adobe/future/resources.hpp>
#include <GG/adobe/future/widgets/headers/button_factory.hpp>
#include <GG/adobe/future/widgets/headers/checkbox_factory.hpp>
#include <GG/adobe/future/widgets/headers/control_button_factory.hpp>
#include <GG/adobe/future/widgets/headers/display_number_factory.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/edit_number_factory.hpp>
#include <GG/adobe/future/widgets/headers/edit_text_factory.hpp>
#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/group_factory.hpp>
#include <GG/adobe/future/widgets/headers/image_factory.hpp>
#include <GG/adobe/future/widgets/headers/label_factory.hpp>
#include <GG/adobe/future/widgets/headers/optional_panel_factory.hpp>
#include <GG/adobe/future/widgets/headers/panel_factory.hpp>
#include <GG/adobe/future/widgets/headers/popup_factory.hpp>
#include <GG/adobe/future/widgets/headers/presets_factory.hpp>
#include <GG/adobe/future/widgets/headers/preview_factory.hpp>
#include <GG/adobe/future/widgets/headers/progress_bar_factory.hpp>
#include <GG/adobe/future/widgets/headers/radio_button_factory.hpp>
#include <GG/adobe/future/widgets/headers/reveal_factory.hpp>
#include <GG/adobe/future/widgets/headers/separator_factory.hpp>
#include <GG/adobe/future/widgets/headers/slider_factory.hpp>
#include <GG/adobe/future/widgets/headers/tab_group_factory.hpp>
#include <GG/adobe/future/widgets/headers/toggle_factory.hpp>
#include <GG/adobe/future/widgets/headers/value_range_format.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/future/widgets/headers/window_factory.hpp>
#include <GG/adobe/istream.hpp>
#include <GG/adobe/memory.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/poly_placeable.hpp>
#include <GG/adobe/static_table.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/view_concept.hpp>

#ifdef ADOBE_STD_SERIALIZATION
    #include <GG/adobe/iomanip.hpp>
#endif

#include <boost/optional.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

struct noncreating_placeable_t {
    void measure(adobe::extents_t&) { }
    void place(const adobe::place_data_t&) const { }
};

inline bool operator==(const noncreating_placeable_t& x, 
                       const noncreating_placeable_t& y)
{
    return &x == &y; // Temporary hack until equality is implemented
} 

/*************************************************************************************************/

inline const adobe::layout_attributes_t& row_layout_attributes()
{
    static bool inited(false);
    static adobe::layout_attributes_t result;

    if (!inited)
    {
        result.placement_m = adobe::eve_t::place_row;
        result.vertical().suppress_m = false; // Allow baselines to propagate
        result.create_m = false;

        inited = true;
    }

    return result;
}

/*************************************************************************************************/

inline const adobe::layout_attributes_t& column_layout_attributes()
{
    static bool inited(false);
    static adobe::layout_attributes_t result;

    if (!inited)
    {
        result.placement_m = adobe::eve_t::place_column;
        result.vertical().suppress_m = true;
        result.create_m = false;

        inited = true;
    }

    return result;
}

/*************************************************************************************************/

inline const adobe::layout_attributes_t& overlay_layout_attributes()
{
    static bool inited(false);
    static adobe::layout_attributes_t result;

    if (!inited)
    {
        result.placement_m = adobe::eve_t::place_overlay;
        result.vertical().suppress_m = false; // Allow baselines to propagate
        result.create_m = false;

        inited = true;
    }

    return result;
}

/*************************************************************************************************/

adobe::widget_node_t wire_to_eve_noncreating(const adobe::factory_token_t&     token,
                                             const adobe::widget_node_t&       parent,
                                             const adobe::dictionary_t&        parameters,
                                             bool                              is_container,
                                             const adobe::layout_attributes_t& layout_attributes)
{
    adobe::poly_placeable_t *p(new adobe::poly_placeable_t(noncreating_placeable_t()));
    
    adobe::layout_attributes_t attributes = layout_attributes;
    apply_layout_parameters(attributes, parameters);
    
    adobe::eve_t::iterator eve_token =
        token.client_holder_m.eve_m.add_placeable(parent.eve_token_m,
                            attributes,
                            is_container,
                            boost::ref(*p));

    token.client_holder_m.assemblage_m.cleanup(boost::bind(adobe::delete_ptr<adobe::poly_placeable_t *>(), p));

    return adobe::widget_node_t(parent.size_m, eve_token, parent.display_token_m, parent.keyboard_token_m);
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

widget_node_t row_factory(const dictionary_t&     parameters,
                          const widget_node_t&    parent,
                          const factory_token_t&  token,
                          const widget_factory_t& factory)
{
    return wire_to_eve_noncreating(token, parent, parameters,
                                   factory.is_container(static_name_t("row")),
                                   factory.layout_attributes(static_name_t("row")));
}

widget_node_t column_factory(const dictionary_t&     parameters,
                             const widget_node_t&    parent,
                             const factory_token_t&  token,
                             const widget_factory_t& factory)
{
    return wire_to_eve_noncreating(token, parent, parameters,
                                   factory.is_container(static_name_t("column")),
                                   factory.layout_attributes(static_name_t("column")));
}

widget_node_t overlay_factory(const dictionary_t&     parameters,
                              const widget_node_t&    parent,
                              const factory_token_t&  token,
                              const widget_factory_t& factory)
{
    return wire_to_eve_noncreating(token, parent, parameters,
                                   factory.is_container(static_name_t("overlay")),
                                   factory.layout_attributes(static_name_t("overlay")));
}

/*************************************************************************************************/

const widget_factory_t& default_asl_widget_factory()
{
    static bool             inited(false);
    static widget_factory_t default_factory_s;

    if (!inited)
    {
        default_factory_s.reg(name_column, &column_factory, true, column_layout_attributes());
        default_factory_s.reg(name_overlay, &overlay_factory, true, overlay_layout_attributes());
        default_factory_s.reg(name_row, &row_factory, true, row_layout_attributes());

        default_factory_s.reg(name_button, &make_button);
        default_factory_s.reg(name_checkbox, &make_checkbox);
        default_factory_s.reg(name_control_button, &make_control_button);
        default_factory_s.reg(name_dialog, &make_window, true, window_layout_attributes());
        default_factory_s.reg(name_display_number, &implementation::make_display_number);
        default_factory_s.reg(name_edit_number, &make_edit_number);
        default_factory_s.reg(name_edit_text, &implementation::make_edit_text);
        default_factory_s.reg(name_group, &make_group, true, group_layout_attributes());
        default_factory_s.reg(name_image, &implementation::make_image_hack);
        default_factory_s.reg(name_toggle, &make_toggle);
        default_factory_s.reg(name_label, &implementation::make_label_hack);
        default_factory_s.reg(name_optional, &make_optional_panel, true, optional_panel_layout_attributes());
        default_factory_s.reg(name_panel, &make_panel, true, panel_layout_attributes());
        default_factory_s.reg(name_popup, &make_popup);
        default_factory_s.reg(name_preset, &make_presets);
        default_factory_s.reg(name_preview, &make_preview);
        default_factory_s.reg(name_progress_bar, &make_progress_bar);
        default_factory_s.reg(name_radio_button, &make_radio_button);
        default_factory_s.reg(name_reveal, &make_reveal);
        default_factory_s.reg(name_separator, &make_separator, false, separator_layout_attributes());
        default_factory_s.reg(name_slider, &make_slider);
        default_factory_s.reg(name_static_text, &implementation::make_label_hack);
        default_factory_s.reg(name_tab_group, &make_tab_group, true, tab_group_layout_attributes());

        inited = true;
    }

    return default_factory_s;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

boost::any client_assembler(adobe::factory_token_t&             token,
                            const boost::any&                   parent,
                            adobe::name_t                       class_name,
                            const adobe::dictionary_t&          arguments,
                            const adobe::widget_factory_proc_t& proc)
{
    //
    // Notice that we use the supplied factory to create widgets.
    //
    return proc(class_name, arguments, boost::any_cast<adobe::widget_node_t>(parent), token);
}

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

auto_ptr<eve_client_holder> make_view(name_t                                 file_path,
                                      const line_position_t::getline_proc_t& getline_proc,
                                      std::istream&                          stream,
                                      sheet_t&                               sheet,
                                      behavior_t&                            root_behavior,
                                      const button_notifier_t&               notifier,
                                      size_enum_t                            dialog_size,
                                      const widget_factory_proc_t&           proc,
                                      platform_display_type                  display_root)
{
    adobe::auto_ptr<eve_client_holder>  result(new eve_client_holder(root_behavior));
    factory_token_t                     token(get_main_display(),
                                              sheet,
                                              *(result.get()),
                                              notifier);

    virtual_machine_t evaluator;
    vm_lookup_t       lookup;

    lookup.attach_to(evaluator);

    /*
        We set the initial parent to be the root of the main display, an
        empty eve iterator and the given dialog size.
    */
    get_main_display().set_root(display_root);
    parse(stream,
                 line_position_t(file_path, getline_proc),
                 widget_node_t(dialog_size,
                                      eve_t::iterator(),
                                      get_main_display().root(),
                                      keyboard_t::iterator()),
                 bind_layout(boost::bind(&client_assembler,
                                         boost::ref(token), _1, _2, _3, boost::cref(proc)),
                             result->layout_sheet_m,
                             evaluator));
    
    result->contributing_m = sheet.contributing();

    return result;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
