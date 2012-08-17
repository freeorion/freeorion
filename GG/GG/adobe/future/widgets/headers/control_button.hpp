/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_CONTROL_BUTTON_HPP
#define ADOBE_CONTROL_BUTTON_HPP

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/future/widgets/headers/platform_button.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct control_button_t
{
    typedef any_regular_t                                              model_type;
    typedef boost::function<void (const model_type&)>                  setter_proc_type;
    typedef boost::function<any_regular_t (const array_t& expression)> expression_eval_proc;

    control_button_t(const std::string&          name,
                     const std::string&          alt_text,
                     const expression_eval_proc& eval_proc,
                     const array_t&              expression,
                     theme_t                     theme);

    /*!
        @name Placeable Concept Operations
        @{

        See the \ref concept_placeable concept and \ref placeable_concept.hpp for more information.
    */

    void measure(extents_t& result);

    void place(const place_data_t& place_data);

    ///@}

    /*!
        @name Controller Concept Operations
        @{

        See the \ref concept_controller concept and \ref controller_concept.hpp for more information.
    */

    void monitor(const setter_proc_type& proc);

    void enable(bool make_enabled);

    ///@}

    adobe::auto_ptr<button_t> button_m;

private:
    void button_fire(const any_regular_t&, const dictionary_t&);

    setter_proc_type          proc_m;
    expression_eval_proc      eval_proc_m;
    array_t                   expression_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
