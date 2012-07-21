/*
    Copyright 2005-2006 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_SUBLAYOUT_HPP
#define ADOBE_SUBLAYOUT_HPP

/****************************************************************************************************/

#include <GG/adobe/adam.hpp>
#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

#include <GG/adobe/future/widgets/headers/platform_panel.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

class sublayout_t
{
public:
    explicit sublayout_t(theme_t theme = theme_normal_s) :
        root_m(any_regular_t(true), theme)
    {
        vm_lookup_m.attach_to(sublayout_sheet_m);
        vm_lookup_m.attach_to(sublayout_sheet_m.machine_m);
    }

    sublayout_t(const sublayout_t& rhs) :
        root_m(any_regular_t(true), rhs.root_m.theme_m)
    { }

    sublayout_t& operator=(const sublayout_t& /*rhs*/)
    { return *this; }

    widget_node_t evaluate(const std::string&       sheet_description,
                           const std::string&       layout_description,
                           const dictionary_t&      parameters,
                           const widget_node_t&     parent,
                           const factory_token_t&   token,
                           const widget_factory_t&  factory,
                           const button_notifier_t& notifier,
                           behavior_t&              behavior);

    template <typename T>
    void sublayout_sheet_set(name_t cell, const T& value);

    void sublayout_sheet_set(name_t cell, const any_regular_t& value)
    { sublayout_sheet_m.set(cell, value); }

    void sublayout_sheet_update()
    { sublayout_sheet_m.update(); }

    template <typename T>
    void sublayout_sheet_set_update(name_t cell, const T& value)
    {
        sublayout_sheet_set(cell, value);
        sublayout_sheet_update();
    }

    /*!
        @name Placeable Concept Operations
        @{

        See the \ref concept_placeable concept and \ref placeable_concept.hpp for more information.
    */
        void measure(extents_t& result)
        {
            std::pair<long, long> eval_result =
                sublayout_holder_m->eve_m.evaluate(eve_t::evaluate_nested);

            result.width() = eval_result.first;
            result.height() = eval_result.second;
        }

        void place(const place_data_t& place_data)
        {
            root_m.place(place_data);

            sublayout_holder_m->eve_m.adjust(eve_t::evaluate_nested,
                                             width(place_data),
                                             height(place_data));
        }
    ///@}

private:
    // order of destruction matters here, so please make sure
    // you know what you're doing when you go to reorder these.

    sheet_t                     sublayout_sheet_m;
    auto_ptr<eve_client_holder> sublayout_holder_m;
    panel_t                     root_m;
    vm_lookup_t                 vm_lookup_m;
};

/****************************************************************************************************/

template <typename T>
void sublayout_t::sublayout_sheet_set(name_t cell, const T& value)
{ sublayout_sheet_set(cell, any_regular_t(value)); }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
