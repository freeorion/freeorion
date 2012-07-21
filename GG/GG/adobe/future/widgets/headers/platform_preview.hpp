/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_PREVIEW_HPP
#define ADOBE_WIDGET_PREVIEW_HPP

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/sublayout.hpp>
#include <GG/adobe/view_concept.hpp>
#include <GG/adobe/controller_concept.hpp>

#include <boost/function.hpp>


namespace GG {
    class Texture;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

/*!
    \ingroup asl_widgets_carbon

    \brief Preview widget

    \model_of
        - \ref concept_placeable
        - \ref concept_view
        - \ref concept_controller
*/
struct preview_t
{
    /// model types for this widget
    typedef dictionary_t                                         controller_model_type;
    typedef boost::shared_ptr<GG::Texture>                       view_model_type;
    typedef boost::function<void (const controller_model_type&)> setter_proc_type;

    /// constructor for this widget
    preview_t(const std::string& alt_text,
              theme_t            theme);

    /*!
        @name Placeable Concept Operations
        @{

        See the \ref concept_placeable concept and \ref placeable.hpp for more information.
    */
    void measure(extents_t& result);

    void place(const place_data_t& place_data);
    ///@}

    /*!
        @name View Concept Operations
        @{

        See the \ref concept_view concept and \ref view.hpp for more information.
    */
    void display(const view_model_type& value);
    ///@}

    /*!
        @name Controller Concept Operations
        @{

        See the \ref concept_controller concept and \ref controller.hpp for more information.
    */
    void monitor(const setter_proc_type& proc);

    void enable(bool make_enabled);
    ///@}

    widget_node_t evaluate(const std::string&       sheet_description,
                           const std::string&       layout_description,
                           const dictionary_t&      parameters,
                           const widget_node_t&     parent,
                           const factory_token_t&   token,
                           const widget_factory_t&  factory,
                           const button_notifier_t& notifier,
                                                   behavior_t&                          behavior)
    {
        widget_node_t result(sublayout_m.evaluate(sheet_description,
                                                  layout_description,
                                                  parameters,
                                                  parent,
                                                  token,
                                                  factory,
                                                  notifier,
                                                                                                  behavior));

        sublayout_m.sublayout_sheet_update();

        return result;
    }

#ifndef ADOBE_NO_DOCUMENTATION
    view_model_type            preview_image_m;
    std::string                alt_text_m;
    sublayout_t                sublayout_m;
    theme_t                    theme_m;
#endif
};

template <>
struct controller_model_type<preview_t>
{
    typedef preview_t::controller_model_type type;
};

template <>
struct view_model_type<preview_t>
{
    typedef preview_t::view_model_type type;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
