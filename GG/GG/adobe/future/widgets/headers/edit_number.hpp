/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGETS_EDIT_NUMBER_HPP
#define ADOBE_WIDGETS_EDIT_NUMBER_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/future/debounce.hpp>
#include <GG/adobe/future/number_formatter.hpp>
#include <GG/adobe/future/widgets/headers/platform_edit_text.hpp>
#include <GG/adobe/future/widgets/headers/number_unit.hpp>
#include <GG/adobe/future/widgets/headers/platform_popup.hpp>
#include <GG/adobe/future/widgets/headers/platform_edit_number.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/locale.hpp>
#include <GG/adobe/future/cursor.hpp>

#include <boost/noncopyable.hpp>

#include <cctype>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

/*!
    \relates edit_number_t

    \todo document me
*/
struct edit_number_unit_subwidget_t
{
    typedef std::string model_type;

    explicit edit_number_unit_subwidget_t(edit_number_t& edit_number);

    void display(const model_type& value);

    edit_number_t& edit_number_m;
};

/****************************************************************************************************/

/*!
    \ingroup apl_widgets

    \brief Edit number widget

    An edit text refinement used solely for numeric entry.
*/
struct edit_number_t : boost::noncopyable
{
    typedef double model_type;

    template <typename ForwardIterator>
    edit_number_t(const edit_text_ctor_block_t& block,
                  const ForwardIterator         first,
                  const ForwardIterator         last);

    ~edit_number_t()
    {
        locale_change_connection_m.disconnect();
    }

    /*!
        @name Placeable Concept Operations
        @{

        See the \ref concept_placeable concept and \ref placeable_concept.hpp for more information.
    */
    void measure(extents_t& result);

    void place(const place_data_t& place_data);
    ///@}

    /*!
        \relates edit_number_t

        \todo document me
    */
    struct controller_t
    {
        typedef edit_number_t::model_type      model_type;
        typedef boost::function<void (const model_type&)> setter_proc_t;

        explicit controller_t(edit_number_t* control = 0,
                                  name_t  cell = name_t()) :
            control_m(control),
            enabled_m(true),
            cell_m(cell)
        { }

        /*!
            @name Controller Concept Operations
            @{
    
            See the \ref concept_controller concept and \ref controller_concept.hpp for more information.
        */
        void monitor(const setter_proc_t& value);
        void enable(bool make_enabled);
        ///@}

        name_t cell() const { return cell_m; }

    private:
        friend struct edit_number_t;

        edit_number_t* control_m;
        setter_proc_t  setter_m;
        bool           enabled_m;
        name_t  cell_m;
    };

    /*!
        \relates edit_number_t

        \todo document me
    */
    struct view_t
    {
        typedef edit_number_t::model_type model_type;

        explicit view_t(edit_number_t* control = 0,
                        name_t  cell = name_t()) :
            control_m(control),
            cell_m(cell)
        { }

        /*!
            @name View Concept Operations
            @{
    
            See the \ref concept_view concept and \ref view_concept.hpp for more information.
        */
        void display(const model_type& value);
        ///@}

        name_t cell() const { return cell_m; }

    private:
        friend struct edit_number_t;

        edit_number_t* control_m;
        name_t  cell_m;
    };

#ifndef ADOBE_NO_DOCUMENTATION
    any_regular_t underlying_handler() { return any_regular_t(edit_text().control_m); }

    bool handle_key(key_type key, bool pressed, modifiers_t modifiers);

    typedef view_t                 view_type;
    typedef std::vector<view_type> view_set_t;
    typedef view_set_t::iterator   view_iterator;

    view_iterator view_begin() { return view_set_m.begin(); }
    view_iterator view_end()   { return view_set_m.end(); }

    typedef controller_t                 controller_type;
    typedef std::vector<controller_type> controller_set_t;
    typedef controller_set_t::iterator   controller_iterator;

    controller_iterator controller_begin() { return controller_set_m.begin(); }
    controller_iterator controller_end()   { return controller_set_m.end(); }

    inline bool using_popup() const
        { return unit_set_m.size() > 1; }

    struct base_unit_t
    {
        base_unit_t(name_t name, double min, double max) :
            name_m(name), min_m(min), max_m(max)
        { }

        name_t name_m;
        double        min_m;
        double        max_m;
    };

    typedef std::vector<base_unit_t> base_unit_set_t;

    // REVISIT (fbrereto) : I'm not happy about these, but we need them
    //                      for the time being so the template specialization
    //                      of insert(display, etc.) can get to them
    inline edit_text_t& edit_text()  { return edit_text_m; }
    inline popup_t&     popup()      { return popup_m; }
    void        initialize();
    void        monitor_locale(const dictionary_t& value);

private:
    void        monitor_text(const std::string& new_value, bool display_was_updated=true);
    void        monitor_popup(const any_regular_t& new_value);
    void        field_text_filter(const std::string& candidate, bool& squelch);
    void        increment(bool up);
    void        increment_n(long n); // in current units
    void        refresh_enabled_state(edit_number_t::controller_t* src);
    void        refresh_view(edit_number_t::view_t* src, const model_type& new_value, bool force = false);
    std::size_t current_base_unit_index();

    static const adobe_cursor_t scrubby_cursor();

    // these are for edit_number_unit_subwidget_t
    friend struct edit_number_unit_subwidget_t;
    void display_unit(const edit_number_unit_subwidget_t::model_type& unit_name);

    // the debounce type is double because if it is the formatted
    // string value the real value will be clipped by the formatter.
    // this will cause the value to be converted to an incorrect
    // scaled value should it be needed by a scaled unit. This was
    // found when incrementing a value stored in units celsius by
    // single units fahrenheit: the celsius value gets clipped and
    // the round-tripping of the fahrenheit value is then incorrect.

    typedef double                     debounce_type;
    typedef std::vector<debounce_type> debounce_set_t;
    typedef std::vector<unit_t>        unit_set_t;

    friend struct edit_number_platform_data_t;

    edit_text_t                 edit_text_m;
    popup_t                     popup_m;
    number_formatter_t          number_formatter_m;
    std::size_t                 edit_text_width_m;
    std::size_t                 unit_index_m; // this is the current index of the *popup*
    unit_set_t                  unit_set_m;
    base_unit_set_t             base_unit_set_m;
    view_set_t                  view_set_m;
    controller_set_t            controller_set_m;
    debounce_set_t              debounce_set_m;
    double                      min_m;
    double                      max_m;
    edit_number_platform_data_t platform_m;
public:
    boost::signals::connection  locale_change_connection_m;
#endif
};

/****************************************************************************************************/

template <typename ForwardIterator>
edit_number_t::edit_number_t(const edit_text_ctor_block_t& block,
                             const ForwardIterator         first,
                             const ForwardIterator         last) :
    edit_text_m(block),
    popup_m(std::string(), block.alt_text_m, std::string(), 0, 0, block.theme_m),
    edit_text_width_m(0),
    unit_index_m(0),
    unit_set_m(first, last),
    platform_m(0)
{
    platform_m = edit_number_platform_data_t(this);
}

/****************************************************************************************************/

inline edit_number_unit_subwidget_t::edit_number_unit_subwidget_t(edit_number_t& edit_number) :
    edit_number_m(edit_number)
{ }

inline void edit_number_unit_subwidget_t::display(const model_type& value)
{
    edit_number_m.display_unit(value);
}

/****************************************************************************************************/

} //namespace adobe

/****************************************************************************************************/

// ADOBE_WIDGETS_EDIT_NUMBER_HPP
#endif

/****************************************************************************************************/
