/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_EVE_HPP
#define ADOBE_EVE_HPP

#include <GG/adobe/config.hpp>

#include <utility>

#include <boost/noncopyable.hpp>

#include <GG/adobe/forest.hpp>
#include <GG/adobe/extents.hpp>
#include <GG/adobe/poly_placeable.hpp>
#include <GG/adobe/layout_attributes.hpp>

/*************************************************************************************************/

namespace adobe {
namespace implementation {

/*************************************************************************************************/

struct view_proxy_t;

/*************************************************************************************************/

} // namespace implementation
} // namespace adobe

/*************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)
namespace boost {
namespace detail {
    template <>
    struct is_pod_impl<adobe::implementation::view_proxy_t>
    {
        BOOST_STATIC_CONSTANT(bool, value = true);
    };
}
}
#endif

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/


/*!

\ingroup eve_engine

\brief Eve engine.
*/

class eve_t
#if !defined(ADOBE_NO_DOCUMENTATION)
    : boost::noncopyable, public extents_slices_t, public layout_attributes_placement_t
#endif
{
 public:

#if !defined(ADOBE_NO_DOCUMENTATION)
    typedef forest<implementation::view_proxy_t> proxy_tree_t;
    typedef proxy_tree_t::iterator               iterator;
#endif




/*!
Specifies the coordinate system origin for the place_data_t of a given view.
*/


    enum evaluate_options_t
    {
        evaluate_nested,    /*!< Specifies the origin of the
        * coordinate system to be the top left point of the parent
        * view*/
        evaluate_flat       /*!< Specifies the origin of the
        * coordinate system to be the top left point of the topmost
        * view in the hierarchy.*/
    };
    
/*
    REVISIT (sparent) : I'm just starting a long cleanup of the Eve API - the steps should include
    the following:
    
    * eliminate the container default proc in favor of an inital extents setting.
    * rename calculate to calculate_horizontal - all calculation passes are optional (initial should
        often suffice).
*/


#if !defined(ADOBE_NO_DOCUMENTATION)
    explicit eve_t();
    ~eve_t();
#endif

/*!

\brief Adds a new view element to the view hierarchy being formed for
layout. 

\param parent the parent of this view. Specify a default-constructed
adobe::eve_t::iterator if this view is to be the topmost view. There
can only be <i>one</i> topmost view when Eve goes to solve for the
layout.

\param initial initial layout attributes

\param is_container_type whether or not the new node is to be a
container view or not, irrespective of whether or not it will actually
contain subviews.

\par Available Parameter Keys
<table width="100%" border="1">
    <tr><th>Name</th><th>Possible Values</th><th>Default</th><th>Notes</th></tr>
    <tr>
        <td><code>child_horizontal</code></td>
        <td>Any of the adobe::layout_attributes_alignment_t::alignment_t enumeration labels as an adobe::name_t</td>
        <td><code>align_default</code></td>
        <td></td>
    </tr>
    <tr>
        <td><code>child_vertical</code></td>
        <td>Any of the adobe::layout_attributes_alignment_t::alignment_t enumeration labels as an adobe::name_t</td>
        <td><code>align_default</code></td>
        <td></td>
    </tr>
    <tr>
        <td><code>guide_mask</code></td>
        <td>array containing any of [ guide_baseline, guide_label ]</td>
        <td>empty (no suppression)</td>
        <td>Suppression of horizontal and/or vertical guides</td>
    </tr>
    <tr>
        <td><code>horizontal</code></td>
        <td>Any of the adobe::layout_attributes_alignment_t::alignment_t enumeration labels as an adobe::name_t</td>
        <td><code>align_default</code></td>
        <td></td>
    </tr>
    <tr>
        <td><code>indent</code></td>
        <td>integer</td>
        <td>0</td>
        <td></td>
    </tr>
    <tr>
        <td><code>placement</code></td>
        <td>Any of the adobe::layout_attributes_placement_t enumeration labels as an adobe::name_t</td>
        <td><code>place_leaf</code></td>
        <td></td>
    </tr>
    <tr>
        <td><code>spacing</code></td>
        <td>array of integers</td>
        <td><code>[ 0 ]</code></td>
        <td>A single value here will propagate for all needed spacing values</td>
    </tr>
    <tr>
        <td><code>vertical</code></td>
        <td>Any of the adobe::layout_attributes_alignment_t::alignment_t enumeration labels as an adobe::name_t</td>
        <td><code>align_default</code></td>
        <td></td>
    </tr>
</table>

\param placeable must be a \ref poly_placeable_t (which might be
castable via \ref adobe::poly_cast a \ref poly_placeable_twopass_t).

\param reverse if \c false (default), this element should be added as
last child of parent. If true, then the add as first child of
parent. \c reverse is intended for implementing support for
right-to-left languages. See \ref right_to_left_layout.

\return An opaque adobe::eve_t::iterator, which can be used as the
<code>parent</code> parameter in another call to add_placeable to
place subviews inside this one.
*/

    iterator add_placeable( iterator parent,
                            const layout_attributes_t&  initial,
                            bool                        is_container_type,
                            poly_placeable_t&           placeable, 
                            bool                        reverse=false);

/*!
\brief set_visible
*/

    void set_visible(iterator, bool);


/*!
\brief This call performs the layout, it will call each element to get its dimentions, solve the layout, and place each item. Specifying a width and height less than the solved width and height will give undefined results. To resize a view, call \c evaluate() to get the minimum size then use \c adjust().

\param options options to be passed to the solution engine.
\param width if not zero, \c width is used for the width of the layout rather than the solved width.
\param height if not zero, \c height is used for the height of the layout rather than the solved height.

\sa
    \ref adobe::eve_t::evaluate_options_t
*/

    std::pair<long, long> evaluate(evaluate_options_t options, long width = 0, long height = 0);


/*!

\brief Adjusts the solved view layout to fit within the newly
specified dimensions. Eve will relay the new solved layout information
to individual placeable objects through thier place()
functions. Specifying a width or hight less than that returned by
evaluate() results in undefined behavior.

\param width if not zero, the new width for the view layout.
\param height if not zero, the new height for the view layout.
\param options options to be passed to the solution engine.

\sa
    \ref adobe::eve_t::evaluate_options_t
*/

    std::pair<long, long> adjust(evaluate_options_t options, long width, long height);
  
 private:
    friend struct implementation::view_proxy_t;

    class implementation_t;
    implementation_t* object_m;
};


/*************************************************************************************************/

void set_margin(layout_attributes_t& container, long x);

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/

