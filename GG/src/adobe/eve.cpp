/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/eve.hpp>

#include <iterator>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/variant.hpp>

#include <GG/adobe/algorithm/for_each.hpp>
#include <GG/adobe/algorithm/for_each_position.hpp>
#include <GG/adobe/algorithm/transform.hpp>
#include <GG/adobe/cmath.hpp>
#include <GG/adobe/forest.hpp>
#include <GG/adobe/functional.hpp>
#include <GG/adobe/iterator.hpp>
#include <GG/adobe/numeric.hpp>

#ifndef NDEBUG
    #include <iostream>
#endif

/*************************************************************************************************/


namespace {

using adobe::eve_t;

struct filter_visible : std::unary_function<adobe::implementation::view_proxy_t, bool>
{
    bool operator() (const adobe::implementation::view_proxy_t& x);
};

typedef adobe::filter_fullorder_iterator<adobe::eve_t::proxy_tree_t::iterator, filter_visible>    cursor;
typedef boost::filter_iterator<filter_visible, adobe::child_iterator<cursor> >          child_iterator;
typedef std::reverse_iterator<child_iterator>                                           reverse_child_iterator;

inline child_iterator child_begin(cursor c)
{ return child_iterator(filter_visible(), adobe::child_begin(c), adobe::child_end(c)); }

inline child_iterator child_end(cursor c)
{ return child_iterator(filter_visible(), adobe::child_end(c), adobe::child_end(c)); }


/*************************************************************************************************/

} // namespace

/*************************************************************************************************/
 

#ifdef ADOBE_HAS_CPLUS0X_CONCEPTS
#include <concepts>
  namespace std {
      concept_map RandomAccessIterator<
          boost::filter_iterator<filter_visible, adobe::child_iterator<cursor> > > {};
  }
#endif

/*
    REVISIT (sparent) : namespace adobe::implementation used instead of unnamed to support friend
    in eve_t.
*/

namespace adobe {
namespace implementation {

/*************************************************************************************************/

struct view_proxy_t : adobe::extents_slices_t
{
    view_proxy_t(const layout_attributes_t&, poly_placeable_t&);

    poly_placeable_t&               placeable_m;

    bool                            visible_m;

    typedef boost::array<guide_set_t, 2>    fr_guide_set_t;

    layout_attributes_t             geometry_m; // REVISIT (sparent) : make const
    place_data_t                    place_m;
    
    long                            space_before_m;      // populated from spacing_m of parent
    boost::array<long, 2>           container_length_m;  // calculated length of container
    boost::array<long, 2>           measured_length_m;   // length of container children only
    
    boost::array<fr_guide_set_t, 2> container_guide_set_m;         // forward/reverse guide set for container

    void calculate();
    void calculate_vertical();
    void place();

    void adjust(::child_iterator, ::child_iterator, slice_select_t);
    void solve_up(::child_iterator, ::child_iterator, slice_select_t slice);
    bool solve_down(::child_iterator, ::child_iterator, slice_select_t slice);
    void layout(::child_iterator, ::child_iterator, slice_select_t slice);
    void flatten(::child_iterator, ::child_iterator, slice_select_t slice, adobe::eve_t::evaluate_options_t);

    void adjust_outsets(::child_iterator, ::child_iterator, slice_select_t);
    void adjust_outsets_with(::child_iterator first, ::child_iterator last, slice_select_t slice);
    void adjust_outsets_cross(::child_iterator first, ::child_iterator last, slice_select_t slice);
    void adjust_with(::child_iterator first, ::child_iterator last, slice_select_t slice);
    void adjust_cross(::child_iterator first, ::child_iterator last, slice_select_t slice);
    void solve_up_with(::child_iterator first, ::child_iterator last, slice_select_t slice);
    void solve_up_cross(::child_iterator first, ::child_iterator last, slice_select_t slice);
    bool solve_down_with(::child_iterator first, ::child_iterator last, slice_select_t slice);
    bool solve_down_cross(::child_iterator first, ::child_iterator last, slice_select_t slice);
    void layout_with(::child_iterator first, ::child_iterator last, slice_select_t slice);
    void layout_cross(::child_iterator first, ::child_iterator last, slice_select_t slice);
};

/*************************************************************************************************/

} // namespace implementation
} // namespace adobe

/*************************************************************************************************/
#if !defined(ADOBE_NO_DOCUMENTATION)
namespace {

inline bool filter_visible::operator() (const adobe::implementation::view_proxy_t& x) { return x.visible_m; }

/*************************************************************************************************/

typedef adobe::implementation::view_proxy_t::slice_select_t slice_select_t;
typedef void (adobe::implementation::view_proxy_t::* apply_member_t)(   ::child_iterator,
                                                                        ::child_iterator, ::slice_select_t);

/*
    REVISIT (sparent) : VC 7.1 cannot handle the apply_member_t as a template parameter.
    Unfortunate, as this form would be more efficient.
*/
#if 0
template<typename BeadIterator, ::apply_member_t Apply>
struct apply_to_children
{
    typedef void result_type;
    
    explicit apply_to_children(::slice_select_t select) : select_m(select) { }

    void operator()(BeadIterator iter)const
    {
        ((*iter).*Apply)(::child_begin(iter.base()), ::child_end(iter.base()), select_m);
    }

private:
    ::slice_select_t select_m;
};
#endif
#if 1
template <typename BeadIterator>
struct apply_to_children
{
    typedef void result_type;
    explicit apply_to_children(::apply_member_t apply, ::slice_select_t select) :
        apply_m(apply),
        select_m(select)
    { }

    void operator()(BeadIterator iter)
    {
        ((*iter).*apply_m)(::child_begin(iter.base()), ::child_end(iter.base()), select_m);
    }

private:
    ::apply_member_t apply_m;
    ::slice_select_t select_m;
};
#endif

/*
    REVISIT (sparent) : This is a quick fix for flatten - I think we could do much of what is
    here more efficiently.
*/

struct apply_flatten_t
{
    typedef void result_type;
    
    explicit apply_flatten_t(slice_select_t select, adobe::eve_t::evaluate_options_t options)
        : options_m(options), select_m(select) { }

    template <typename BeadIterator>
    void operator()(BeadIterator iter)
    {
        iter->flatten(::child_begin(iter.base()), ::child_end(iter.base()), select_m, options_m);
    }

private:
    adobe::eve_t::evaluate_options_t    options_m;
    slice_select_t                      select_m;
};

/*************************************************************************************************/

bool is_with(adobe::eve_t::placement_t placement, adobe::eve_t::slice_select_t select)
{
    return  ((placement == adobe::eve_t::place_column) && (select == adobe::eve_t::vertical)) ||
            ((placement == adobe::eve_t::place_row) && (select == adobe::eve_t::horizontal));
}

/*************************************************************************************************/

} // namespace
#endif
/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

void set_margin(layout_attributes_t& container, long x)
{
    container.slice_m[eve_t::horizontal].margin_m.first = x;
    container.slice_m[eve_t::horizontal].margin_m.second = x;
    container.slice_m[eve_t::vertical].margin_m.first = x;
    container.slice_m[eve_t::vertical].margin_m.second = x;
}

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

class eve_t::implementation_t : private extents_slices_t
{
public:

    implementation_t();

    ~implementation_t();

    std::pair<long, long> evaluate(evaluate_options_t, long width, long height);
    std::pair<long, long> adjust(evaluate_options_t options, long width, long height);
    iterator              add_placeable(iterator parent,
                                           const layout_attributes_t&  initial,
                                           bool                        is_container_type,
                                           poly_placeable_t&           placeable, 
                                           bool                        reverse);
    void                  set_visible(iterator, bool);

private:
    void solve(slice_select_t select);
    void layout(slice_select_t select, long optional_length);
    
    
    typedef edge_iterator<cursor, adobe::forest_trailing_edge>  postorder_iterator;
    typedef edge_iterator<cursor, adobe::forest_leading_edge>   preorder_iterator;
    
    std::pair<postorder_iterator, postorder_iterator> postorder_range()
    { return adobe::postorder_range(filter_fullorder_range(proxies_m, filter_visible())); }
    
    std::pair<preorder_iterator, preorder_iterator> preorder_range()
    { return adobe::preorder_range(filter_fullorder_range(proxies_m, filter_visible())); }

    proxy_tree_t proxies_m;
};

/*************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)
eve_t::eve_t() :
    object_m(new implementation_t())
{ }

eve_t::~eve_t()
{ delete object_m; }
#endif

std::pair<long, long> eve_t::evaluate(evaluate_options_t options, long width, long height)
{ return object_m->evaluate(options, width, height); }

std::pair<long, long> eve_t::adjust(evaluate_options_t options, long width, long height)
{ return object_m->adjust(options, width, height); }

eve_t::iterator eve_t::add_placeable(iterator parent, 
                                        const layout_attributes_t&  initial,
                                        bool                        is_container_type,    // is the element a container?
                                        poly_placeable_t&           placeable,            // signals to call for the element  
                                        bool                        reverse)
{ return object_m->add_placeable(parent, initial, is_container_type, placeable, reverse); }

void eve_t::set_visible(iterator c, bool visible)
{ return object_m->set_visible(c, visible); }

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

eve_t::implementation_t::implementation_t()
{ }

/*************************************************************************************************/

eve_t::implementation_t::~implementation_t()
{ }

/*************************************************************************************************/

/*
    REVISIT (sparent) : This logic would be simplified with a root() function wich could be passed
    as the initial parent.
*/

eve_t::iterator eve_t::implementation_t::add_placeable(
                                          iterator parent,
                                          const layout_attributes_t&  initial,
                                          bool                        is_container_type,
                                          poly_placeable_t&           placeable, 
                                          bool                        reverse)

{
    if (parent == iterator()) parent = proxies_m.end();
    
    parent = proxies_m.insert(reverse ? boost::next(adobe::leading_of(parent)) : adobe::trailing_of(parent),
        implementation::view_proxy_t(initial, placeable));

    if (!is_container_type)
        parent->geometry_m.placement_m = place_leaf;

    return parent;
}

/*************************************************************************************************/

void eve_t::implementation_t::set_visible(iterator c, bool visible) { c->visible_m = visible; }

/*************************************************************************************************/

void eve_t::implementation_t::solve(slice_select_t select)
{
    adobe::for_each_position(postorder_range(),
        apply_to_children<postorder_iterator>(&implementation::view_proxy_t::solve_up, select));
            
    // REVISIT (sparent) : HUGE opportunities to optimize this.
    
    // solve until things settle down.
    
    bool progress (false);
    
    /*
        REVISIT (sparent) : This loop should solve it in rougly log N passes
        We know that we are guaranteed to solve one guide completely per iteration
        and most often it should be several. But, I've been known to make a mistake or two
        (which could cause this loop to solve forever!). So we put a limiter on it and
        an assert.
        
    */
    proxy_tree_t::size_type limiter(proxies_m.size() + 1); // + 1 to account for the empty case

    do
    {
        --limiter;
        progress = false;
        
        for(preorder_iterator first(boost::begin(preorder_range())),
                last(boost::end(preorder_range())); first != last; ++first)
        {
            progress |= first->solve_down(::child_begin(first.base()), ::child_end(first.base()), select);
        }

        adobe::for_each_position(postorder_range(),
            ::apply_to_children<postorder_iterator>(&implementation::view_proxy_t::solve_up, select));
        
    } while (progress && limiter);
    
    assert(limiter); // Failing to make forward progress - aborting.
}

/*************************************************************************************************/

void eve_t::implementation_t::layout(slice_select_t select, long optional_length)
{
    // FILTER VISIBLE - this assumes a visible root
    
    if (!proxies_m.empty())
    {
        place_data_t::slice_t&           pslice(proxies_m.front().place_m.slice_m[select]);
        
        /*
            REVISIT (sparent) : This allows us to go sub-minimum. May revisit for wrapped
            containers.
        */

        pslice.length_m = optional_length ? optional_length : proxies_m.front().container_length_m[select];
    }
    
    adobe::for_each_position(preorder_range(),
            apply_to_children<preorder_iterator>(&implementation::view_proxy_t::layout, select));
}

/*************************************************************************************************/

std::pair<long, long> eve_t::implementation_t::evaluate(evaluate_options_t options, long width, long height)
{
    // Calculate
    
    adobe::for_each(postorder_range(), &proxy_tree_t::value_type::calculate);

    // adjust
    
    return adjust(options, width, height);
}

/*************************************************************************************************/

std::pair<long, long> eve_t::implementation_t::adjust(evaluate_options_t options, long width, long height)
{
    // adjust
    
    adobe::for_each_position(postorder_range(),
            apply_to_children<postorder_iterator>(&implementation::view_proxy_t::adjust, horizontal));

    // solve
    
    solve(horizontal); // Not necessary
    layout(horizontal, width);
    
    // adjust outsets
    
    adobe::for_each_position(postorder_range(),
            apply_to_children<postorder_iterator>(&implementation::view_proxy_t::adjust_outsets, horizontal));
    
    // flatten
    
    adobe::for_each_position(preorder_range(), apply_flatten_t(horizontal, options));
    
    // give the client a crack at adjusting vertical
    
    adobe::for_each(postorder_range(), &proxy_tree_t::value_type::calculate_vertical);
    
    // adjust
    
    adobe::for_each_position(postorder_range(),
            apply_to_children<postorder_iterator>(&implementation::view_proxy_t::adjust, vertical));

    // solve
    
    solve(vertical);
    layout(vertical, height);
    
    // adjust outsets
    
    adobe::for_each_position(postorder_range(),
            apply_to_children<postorder_iterator>(&implementation::view_proxy_t::adjust_outsets, vertical));
    
    // flatten
    
    adobe::for_each_position(preorder_range(), apply_flatten_t(vertical, options));
        
    // place
    
    adobe::for_each(preorder_range(), &proxy_tree_t::value_type::place);

    return std::pair<long, long>(proxies_m.front().place_m.horizontal().length_m,
                                 proxies_m.front().place_m.vertical().length_m);
}

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

namespace implementation {
        
/*************************************************************************************************/

struct calculate : public boost::static_visitor<>
{
    template <typename T>
    void operator()( T & operand ) const
    {
        operand += operand;
    }

};

/*************************************************************************************************/

view_proxy_t::view_proxy_t( const adobe::layout_attributes_t& d,
                            poly_placeable_t& p) :
    placeable_m(p), visible_m(true), geometry_m(d)
{ }

/*************************************************************************************************/

void view_proxy_t::calculate()
{
    /*
        REVISIT (sparent) : What we want is for the data from placeable widgets to be preserved
        unless they are explicity dirtied - rather than calling measure for every update.
        For now, there are several bugs caused by the measuring code in widgets assuming that the
        initial extents it is handed is a defaulted extents. Without that - some code accumulates
        metrics into the extents giving ever increasing growth when a window is resized (as an
        example). This fix is a hack - to just always clear the extents before measuring.
    */
    geometry_m.extents_m = extents_t();

    placeable_m.measure(geometry_m.extents_m);

    extents_t::slice_t& eslice = geometry_m.extents_m.horizontal();
    
    place_m.horizontal().length_m = eslice.length_m;
    place_m.horizontal().outset_m = eslice.outset_m;
    
    // Copy the guides over. Only non-surpressed guides will be updated.
    place_m.horizontal().guide_set_m = eslice.guide_set_m;
    
    /*
        REVISIT (sparent) : It becomes apparent here that we have a number of limitation which
        we should address:
        
        1. Containers cannot have their own guides to align items in their frame.
        2. Leaf nodes effectively have thier guides surpressed if they are aligned by anything other
            than forward or reverse. Should probably allow for fill, but that would make fill
            orthoganal to alignment.
    */
    
    /*
        REVISIT (sparent) : The vertical data should also be handled here to avoid the peformance
        hit in calculate_vertical() during a resize. I'll factor this code later.
    */
    
    container_length_m[horizontal] = eslice.length_m;
}

/*************************************************************************************************/

void view_proxy_t::calculate_vertical()
{
    extents_t::slice_t& eslice = geometry_m.extents_m.vertical();

    if(poly_placeable_twopass_t* p = poly_cast<poly_placeable_twopass_t*>(&placeable_m))
    {
 // We pass a copy of the geometry so client can't modify horizontal properties.
       extents_t vertical_stuff(geometry_m.extents_m);
       p->measure_vertical(vertical_stuff, place_m);
       eslice = vertical_stuff.vertical();
    }
    
    place_m.vertical().length_m = eslice.length_m;
    place_m.vertical().outset_m = eslice.outset_m;
    
    // Copy the guides over. Only non-surpressed guides will be updated.
    place_m.vertical().guide_set_m = eslice.guide_set_m;
    
    container_length_m[vertical] = eslice.length_m;
}

/*************************************************************************************************/

void view_proxy_t::place()
{
    placeable_m.place(place_m);
}

/*************************************************************************************************/

void view_proxy_t::adjust(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    if (geometry_m.placement_m == eve_t::place_leaf) return;

    for (::child_iterator iter(first); iter != last; ++iter)
    {
        layout_attributes_t::alignment_t& child_alignment(iter->geometry_m.slice_m[select].alignment_m);
    
        if (child_alignment == layout_attributes_t::align_default)
        {
            child_alignment = geometry_m.slice_m[select].child_alignment_m;
        }
   
    /*
        Now that we know the final alignment - we can copy the guides to the container guides.
        This is only done on leaf nodes because the guides will be propogated to the containers
        through either adjust_with or _cross.
    */
        if (iter->geometry_m.placement_m == eve_t::place_leaf)
        {
            switch (child_alignment)
            {
             case layout_attributes_t::align_forward:
             case layout_attributes_t::align_forward_fill:
                {
                iter->container_guide_set_m[select][layout_attributes_t::align_forward]
                        = iter->geometry_m.extents_m.slice_m[select].guide_set_m;
                }
                break;
             case layout_attributes_t::align_reverse:
             case layout_attributes_t::align_reverse_fill:
                {
                guide_set_t& guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_reverse]);
                
                guide_set= iter->geometry_m.extents_m.slice_m[select].guide_set_m;
                
                adobe::transform(guide_set, boost::begin(guide_set),
                    boost::bind(std::minus<long>(), iter->geometry_m.extents_m.slice_m[select].length_m, _1));
                adobe::reverse(guide_set);
                }
                break;
             default:
                break;
            }
        }
    }
    
    if (is_with(geometry_m.placement_m, select)) adjust_with(first, last, select);
    else adjust_cross(first, last, select);
}

/*************************************************************************************************/

void view_proxy_t::solve_up(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    if (geometry_m.placement_m == adobe::eve_t::place_leaf) return;

    if (select == vertical)
    {
        if (geometry_m.placement_m == adobe::eve_t::place_column) solve_up_with(first, last, vertical);
        else solve_up_cross(first, last, vertical);
    }
    else
    {
        if (geometry_m.placement_m == adobe::eve_t::place_row) solve_up_with(first, last, horizontal);
        else solve_up_cross(first, last, horizontal);
    }
}

/*************************************************************************************************/

bool view_proxy_t::solve_down(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    bool result (false);

    if (geometry_m.placement_m == adobe::eve_t::place_leaf) return result;

    if (select == vertical)
    {
        if (geometry_m.placement_m == adobe::eve_t::place_column) result |= solve_down_with(first, last, vertical);
        else result |= solve_down_cross(first, last, vertical);
    }
    else
    {
        if (geometry_m.placement_m == adobe::eve_t::place_row) result |= solve_down_with(first, last, horizontal);
        else result |= solve_down_cross(first, last, horizontal);
    }
    
    return result;
}

/*************************************************************************************************/

void view_proxy_t::layout(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    if (geometry_m.placement_m == adobe::eve_t::place_leaf) return;

    if (select == vertical)
    {
        if (geometry_m.placement_m == adobe::eve_t::place_column) layout_with(first, last, vertical);
        else layout_cross(first, last, vertical);
    }
    else
    {
        if (geometry_m.placement_m == adobe::eve_t::place_row) layout_with(first, last, horizontal);
        else layout_cross(first, last, horizontal);
    }
}

/*************************************************************************************************/

void view_proxy_t::flatten(::child_iterator first, ::child_iterator last, slice_select_t select, adobe::eve_t::evaluate_options_t options)
{
// Push the guides into the element being placed.
    
    guide_set_t& guide_set(place_m.slice_m[select].guide_set_m);

    switch (geometry_m.slice_m[select].alignment_m)
    {
     case layout_attributes_t::align_forward:
     case layout_attributes_t::align_forward_fill:
        {
        guide_set = container_guide_set_m[select][layout_attributes_t::align_forward];
        }
        break;
     case layout_attributes_t::align_reverse:
     case layout_attributes_t::align_reverse_fill:
        {
        guide_set = container_guide_set_m[select][layout_attributes_t::align_reverse];
        adobe::transform(guide_set, boost::begin(guide_set), 
                boost::bind(std::minus<long>(), place_m.slice_m[select].length_m, _1));
        adobe::reverse(guide_set);
        }
        break;
    default:
        break;
    }

// Flatten the coordinate system if needed for the children.

    if (options == adobe::eve_t::evaluate_nested && geometry_m.create_m) return;
    
    long position (place_m.slice_m[select].position_m);

    for (; first != last; ++first)
    {
        first->place_m.slice_m[select].position_m += position;
    }
}

/*************************************************************************************************/

void view_proxy_t::adjust_outsets(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    if (select == vertical)
    {
        if (geometry_m.placement_m == adobe::eve_t::place_column) adjust_outsets_with(first, last, vertical);
        else adjust_outsets_cross(first, last, vertical);
    }
    else
    {
        if (geometry_m.placement_m == adobe::eve_t::place_row) adjust_outsets_with(first, last, horizontal);
        else adjust_outsets_cross(first, last, horizontal);
    }
}

/*************************************************************************************************/

void view_proxy_t::adjust_outsets_with( ::child_iterator    first,
                                        ::child_iterator    last,
                                        slice_select_t  select)
{
    const extents_t::slice_t& eslice(geometry_m.extents_m.slice_m[select]);
    place_data_t::slice_t&           pslice(place_m.slice_m[select]);

    if (first == last)
    {
        pslice.outset_m = eslice.outset_m;
        return;
    }

    --last;

    adobe::place_data_t::slice_t&            leading_pslice  (first->place_m.slice_m[select]);
    adobe::place_data_t::slice_t&            trailing_pslice (last->place_m.slice_m[select]);

    long leading_outset_position = leading_pslice.position_m - leading_pslice.outset_m.first;

    // trailing_outset_position is measured as distance from edge to inside (like frame)
    long trailing_outset_position = pslice.length_m - (trailing_pslice.position_m
            + trailing_pslice.length_m + trailing_pslice.outset_m.second);

    // REVISIT (sparent) : We need a warning mechanism to report this issue!
    #ifndef NDEBUG
    if (!(!eslice.frame_m.first || (leading_outset_position >= eslice.frame_m.first)))
    {
        std::cerr << "WARNING (sparent) : outset collision." << std::endl;
    }
    #endif

    pslice.outset_m.first = std::max(pslice.outset_m.first, -leading_outset_position);
    
    #ifndef NDEBUG
    if (!(!eslice.frame_m.second || (trailing_outset_position >= eslice.frame_m.second)))
    {
        std::cerr << "WARNING (sparent) : outset collision." << std::endl;
    }
    #endif
    
    pslice.outset_m.second = std::max(pslice.outset_m.second, -trailing_outset_position);
}

/*************************************************************************************************/

void view_proxy_t::adjust_outsets_cross(::child_iterator first, ::child_iterator last,
        slice_select_t select)
{
    const extents_t::slice_t& eslice(geometry_m.extents_m.slice_m[select]);
    place_data_t::slice_t&           pslice(place_m.slice_m[select]);
    
    if (first == last)
    {
        pslice.outset_m = eslice.outset_m;
        return;
    }

    // start by assuming enough space for the frame.
    long leading_outset_position = eslice.frame_m.first;
    long trailing_outset_position = eslice.frame_m.second;

    for (::child_iterator iter (first); iter != last; ++iter)
    {
        place_data_t::slice_t&           iter_pslice(iter->place_m.slice_m[select]);

        leading_outset_position = std::min(leading_outset_position,
                iter_pslice.position_m - iter_pslice.outset_m.first);

        // trailing_outset_position is measured as distance from edge to inside (like frame)
        trailing_outset_position = std::min(trailing_outset_position,
                pslice.length_m - (iter_pslice.position_m + iter_pslice.length_m
                + iter_pslice.outset_m.second));
    }

    // REVISIT (sparent) : We need a warning mechanism to report this issue!
    #ifndef NDEBUG
    if (!(!eslice.frame_m.first || (leading_outset_position >= eslice.frame_m.first)))
    {
        std::cerr << "WARNING (sparent) : outset collision." << std::endl;
    }
    #endif

    pslice.outset_m.first = std::max(pslice.outset_m.first, -leading_outset_position);

    #ifndef NDEBUG
    if (!(!eslice.frame_m.second || (trailing_outset_position >= eslice.frame_m.second)))
    {
        std::cerr << "WARNING (sparent) : outset collision." << std::endl;
    }
    #endif

    pslice.outset_m.second = std::max(pslice.outset_m.second, -trailing_outset_position);
}

/*************************************************************************************************/

void view_proxy_t::layout_with(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    if (first == last) return;

    const layout_attributes_t::slice_t& gslice(geometry_m.slice_m[select]);
    const extents_t::slice_t&           eslice(geometry_m.extents_m.slice_m[select]);
    place_data_t::slice_t&              pslice(place_m.slice_m[select]);

    /*
    REVISIT (sparent) : Something to think about. Counting the number of children could be expressed as count_if
    that takes a "within set" as a predicate.
    */
    
    /*
    REVISIT (sparent) : The extra space should be destributed to the items in blocks - up to the
    first guide, then to the next.
    */

    // count the number of children to distribute any additional space
    
    int padded_count (0);
    
    for (::child_iterator iter(first) ; iter != last; ++iter)
    {
        
        switch (iter->geometry_m.slice_m[select].alignment_m)
        {
         case adobe::layout_attributes_t::align_center:
         case adobe::layout_attributes_t::align_proportional:
         case adobe::layout_attributes_t::align_forward_fill:
         case adobe::layout_attributes_t::align_reverse_fill:
            ++padded_count;
            break;
         default:
            break;
        }
    }
    
    // calculate the additional per/item space
    
    long remaining_additional_length(pslice.length_m - measured_length_m[select]);
            
    long available_length (pslice.length_m - gslice.margin_m.first
        - gslice.margin_m.second - eslice.frame_m.first
        - eslice.frame_m.second);
        
    // place any reverse align items
    long rlength = pslice.length_m - (gslice.margin_m.second + eslice.frame_m.second);
    
    ::reverse_child_iterator riter (last);
    ::reverse_child_iterator rlast (first);
    
    guide_set_t::iterator reverse_guide_iter(container_guide_set_m[select][layout_attributes_t::align_reverse].begin());
    
    for (; riter != rlast; ++riter)
    {
        const layout_attributes_t::slice_t& iter_gslice(riter->geometry_m.slice_m[select]);
        
        if (iter_gslice.alignment_m != layout_attributes_t::align_reverse
            && iter_gslice.alignment_m != layout_attributes_t::align_reverse_fill) break;
            
        place_data_t::slice_t& iter_pslice(riter->place_m.slice_m[select]);
        guide_set_t& reverse_child_guide_set(riter->container_guide_set_m[select][layout_attributes_t::align_reverse]);
                    
        iter_pslice.length_m = riter->container_length_m[select];
        
        // These items are offset by their guides
        
        if (!iter_gslice.suppress_m && reverse_child_guide_set.size())
        {
            if (riter->geometry_m.placement_m == eve_t::place_leaf)
            {
                rlength -= *reverse_guide_iter - reverse_child_guide_set.front();
            }
            std::advance(reverse_guide_iter, guide_set_t::difference_type(reverse_child_guide_set.size()));
        }
        
        /*
            REVISIT (sparent) : Filled items fill forward. The space to behind is going to
            be dead space... It would be good to allow it to be filled.
        */
        
        if (iter_gslice.alignment_m == adobe::layout_attributes_t::align_reverse_fill)
        {
        
            long additional_length(remaining_additional_length / padded_count);
        
            --padded_count;
            remaining_additional_length -= additional_length;
        
            iter_pslice.length_m = riter->container_length_m[select] + additional_length;
        }
        
        iter_pslice.position_m = rlength - iter_pslice.length_m;
            
        rlength -= iter_pslice.length_m + riter->space_before_m;
    }
    
    // place any forward items
    
    long zero_length = gslice.margin_m.first + eslice.frame_m.first;
    long length = zero_length;

    guide_set_t::iterator guide_iter(container_guide_set_m[select][layout_attributes_t::align_forward].begin());
    
    for (::child_iterator iter (first); iter != riter.base(); ++iter)
    {
        const layout_attributes_t::slice_t& iter_gslice(iter->geometry_m.slice_m[select]);
        place_data_t::slice_t&           iter_pslice(iter->place_m.slice_m[select]);
        
        guide_set_t& forward_child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_forward]);

        length += iter->space_before_m;

        if (iter_gslice.alignment_m != adobe::layout_attributes_t::align_fill)
        {
            iter_pslice.length_m = iter->container_length_m[select];
        }
        
        long additional_length(0);
        
        switch (iter_gslice.alignment_m)
        {
        case adobe::layout_attributes_t::align_forward_fill:
        case adobe::layout_attributes_t::align_center:
        case adobe::layout_attributes_t::align_proportional:
            {
            additional_length = remaining_additional_length / padded_count;
            --padded_count;
            remaining_additional_length -= additional_length;
            }
        break;
        default: break;
        }
        
        switch (iter_gslice.alignment_m)
        {
         case layout_attributes_t::align_forward:
         case layout_attributes_t::align_forward_fill:
            {
            // These items are offset by their guides
            
            if (!iter_gslice.suppress_m && forward_child_guide_set.size())
            {
                if (iter->geometry_m.placement_m == eve_t::place_leaf)
                {
                    length = *guide_iter - forward_child_guide_set.front();
                }
                std::advance(guide_iter, guide_set_t::difference_type(forward_child_guide_set.size()));
            }
            
            /*
                REVISIT (sparent) : Filled items fill forward. The space to behind is going to
                be dead space... It would be good to allow it to be filled.
            */
            
            if (iter_gslice.alignment_m == adobe::layout_attributes_t::align_forward_fill)
            {
                iter_pslice.length_m = iter->container_length_m[select] + additional_length;
            }
            
            iter_pslice.position_m = length;
            }
            break;
         case adobe::layout_attributes_t::align_center:
            {
            long block_length (iter->container_length_m[select] + additional_length);
            iter_pslice.position_m = length + (block_length - iter->container_length_m[select]) / 2;
            length += additional_length;
            }
            break;
         case adobe::layout_attributes_t::align_proportional:
            {
            long block_length (iter->container_length_m[select] + additional_length);
            double proportion (double(length - zero_length)
                    / double(available_length - block_length));
            iter_pslice.position_m = length + adobe::lround_half_up(proportion * additional_length);
            length += additional_length;
            }
            break;
         case adobe::layout_attributes_t::align_reverse:
         case adobe::layout_attributes_t::align_reverse_fill:
            /*
                REVISIT (sparent) : This is a runtime error and needs to go through the stream
                error reporting mechanism.
            */
            throw std::logic_error("Right align item before other item.");
         default:
            break;
        }
        
        length += iter_pslice.length_m;
    }
}

/*************************************************************************************************/

void view_proxy_t::layout_cross(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    const layout_attributes_t::slice_t& gslice(geometry_m.slice_m[select]);
    const extents_t::slice_t&           eslice(geometry_m.extents_m.slice_m[select]);
    place_data_t::slice_t&              pslice(place_m.slice_m[select]);

    // calculate a base for the child position.
    
    long length = gslice.margin_m.first + eslice.frame_m.first;
    long rlength = pslice.length_m - (gslice.margin_m.second + eslice.frame_m.second);

    for (::child_iterator iter (first); iter != last; ++iter)
    {
        const layout_attributes_t::slice_t& iter_gslice(iter->geometry_m.slice_m[select]);
        place_data_t::slice_t&           iter_pslice(iter->place_m.slice_m[select]);
        
        guide_set_t& forward_child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_forward]);
        guide_set_t& reverse_child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_reverse]);
        
        if (iter_gslice.alignment_m != adobe::layout_attributes_t::align_fill)
        {
            iter_pslice.length_m = iter->container_length_m[select];
        }
        
        long iter_length(length);
        long iter_rlength(rlength);
        
        switch (iter_gslice.alignment_m)
        {
         case layout_attributes_t::align_forward:
         case layout_attributes_t::align_forward_fill:
            iter_length += iter->geometry_m.indent_m;
            break;
         case layout_attributes_t::align_reverse:
         case layout_attributes_t::align_reverse_fill:
            iter_rlength -= iter->geometry_m.indent_m;
            break;
         default:
            break;
        }

        switch (iter_gslice.alignment_m)
        {
         case layout_attributes_t::align_forward:
         case layout_attributes_t::align_forward_fill:
            {
            if ((iter->geometry_m.placement_m == eve_t::place_leaf) && !iter_gslice.suppress_m
                    && forward_child_guide_set.size())
            {
                iter_length = container_guide_set_m[select][layout_attributes_t::align_forward].front() - forward_child_guide_set.front();
            }
            
            if (iter_gslice.alignment_m == layout_attributes_t::align_forward_fill)
            {
                iter_pslice.length_m = iter_rlength - iter_length;
            }
            
            iter_pslice.position_m = iter_length;
            }
            break;
         case layout_attributes_t::align_reverse:
         case layout_attributes_t::align_reverse_fill:
            {
            if ((iter->geometry_m.placement_m == eve_t::place_leaf) && !iter_gslice.suppress_m
                    && reverse_child_guide_set.size())
            {
                iter_rlength = pslice.length_m - (container_guide_set_m[select][layout_attributes_t::align_reverse].front() - reverse_child_guide_set.front());
            }
            
            if (iter_gslice.alignment_m == layout_attributes_t::align_reverse_fill)
            {
                iter_pslice.length_m = iter_rlength - iter_length;
            }
            
            iter_pslice.position_m = iter_rlength - iter->container_length_m[select];
            }
            break;
         case layout_attributes_t::align_center:
         case layout_attributes_t::align_proportional:
            iter_pslice.position_m = (iter_rlength + iter_length - iter->container_length_m[select]) / 2;
            break;
         default:
            break;
        }
    }
}

/*************************************************************************************************/

void view_proxy_t::adjust_with(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    // set the spacing for each of our children.

    /*
    REVISIT (sparent) : I'm still struggling to write a concise version of this section.
    
    Things to note:
        1. we need a make_tranform_range().
        2. This function should be passed _range_ instead of first, last.
        3. complain to boost again about bind/mem_fn not dealing with non-const member references.
    
    ----
    
    typedef adobe::mem_data_t<view_proxy_t, long>           transform_function;
    typedef boost::transform_iterator<transform_function>   tranform_iterator;
    typedef boost::tranform_range<tranform_iterator>        tranform_range;
    
    tranform_range child_spacing_range(
            adobe::make_tranform_range(range, adobe::mem_data(&view_proxy_t::space_before_m));
            
    assert(!geometry_m.spacing_m.empty());
    
    std::fill(
        adobe::copy_bound(geometry.spacing_m, child_spacing_range).second,
        boost::end(child_spacing_range), geometry_m.spacing_m.back());
    */
    
    typedef layout_attributes_t::spacing_t::difference_type difference_type;

    const difference_type copy_count(std::min(difference_type(geometry_m.spacing_m.size()),
            difference_type(std::distance(first, last))));
            
    assert(!geometry_m.spacing_m.empty());
    
    std::fill(
        std::copy(geometry_m.spacing_m.begin(), geometry_m.spacing_m.begin() + copy_count,
                boost::make_transform_iterator(first, adobe::mem_data(&view_proxy_t::space_before_m))),
        boost::make_transform_iterator(last, adobe::mem_data(&view_proxy_t::space_before_m)),
        geometry_m.spacing_m.back());
    
    // size the container guide set based on the number of guides our children have.
    
    /*
    REVISIT (sparent) : I'm sure there is oportunity to do less work here but we clear the guides
    so we can re-adjust on a size adjust.
    */
    
    container_guide_set_m[select][layout_attributes_t::align_forward].clear();
    container_guide_set_m[select][layout_attributes_t::align_reverse].clear();
    
    /*
    REVISIT (sparent) : This is a good candidate for using boost::lambda and accumlate.
    There may also be the notion of an iterator adaptor that adapts iterator to pointer
    to behave as a simple iterator.
    */
    
    std::size_t forward_guide_count(0);
    std::size_t reverse_guide_count(0);
    
    for (::child_iterator iter(first); iter != last; ++iter)
    {
        layout_attributes_t::slice_t& gslice(iter->geometry_m.slice_m[select]);
        if (!gslice.suppress_m)
        {
            switch (gslice.alignment_m)
            {
             case layout_attributes_t::align_forward:
                {
                forward_guide_count += iter->container_guide_set_m[select][layout_attributes_t::align_forward].size();
                }
                break;
             case layout_attributes_t::align_reverse:
                {
                reverse_guide_count += iter->container_guide_set_m[select][layout_attributes_t::align_reverse].size();
                }
                break;
             case layout_attributes_t::align_forward_fill:
             case layout_attributes_t::align_reverse_fill:
                {
                forward_guide_count += iter->container_guide_set_m[select][layout_attributes_t::align_forward].size();
                reverse_guide_count += iter->container_guide_set_m[select][layout_attributes_t::align_reverse].size();
                }
                break;
             default:
                gslice.suppress_m = true;
                break;
            }
        }
    }

    container_guide_set_m[select][layout_attributes_t::align_forward].resize(forward_guide_count);
    container_guide_set_m[select][layout_attributes_t::align_reverse].resize(reverse_guide_count);
}

/*************************************************************************************************/

void view_proxy_t::adjust_cross(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    // size the container guide set based on the number of guides our children have.

    /*
    REVISIT (sparent) : I'm sure there is oportunity to do less work here but we clear the guides
    so we can re-adjust on a size adjust.
    */
    
    container_guide_set_m[select][layout_attributes_t::align_forward].clear();
    container_guide_set_m[select][layout_attributes_t::align_reverse].clear();
    
    /*
    REVISIT (sparent) : This is a good candidate for using boost::lambda and accumlate.
    There may also be the notion of an iterator adaptor that adapts iterator to pointer
    to behave as a simple iterator.
    */
    
    std::size_t forward_guide_count(0);
    std::size_t reverse_guide_count(0);
    
    for (::child_iterator iter(first); iter != last; ++iter)
    {
        layout_attributes_t::slice_t& gslice(iter->geometry_m.slice_m[select]);
        
        if (gslice.suppress_m) continue;

        switch (gslice.alignment_m)
        {
         case layout_attributes_t::align_forward:
            {
            forward_guide_count = std::max(forward_guide_count, iter->container_guide_set_m[select][layout_attributes_t::align_forward].size());
            }
            break;
         case layout_attributes_t::align_reverse:
            {
            reverse_guide_count = std::max(reverse_guide_count, iter->container_guide_set_m[select][layout_attributes_t::align_reverse].size());
            }
            break;
         case layout_attributes_t::align_forward_fill:
         case layout_attributes_t::align_reverse_fill:
            {
            forward_guide_count = std::max(forward_guide_count, iter->container_guide_set_m[select][layout_attributes_t::align_forward].size());
            reverse_guide_count = std::max(reverse_guide_count, iter->container_guide_set_m[select][layout_attributes_t::align_reverse].size());
            }
            break;
         default:
            gslice.suppress_m = true;
            break;
        }
    }

    container_guide_set_m[select][layout_attributes_t::align_forward].resize(forward_guide_count);
    container_guide_set_m[select][layout_attributes_t::align_reverse].resize(reverse_guide_count);
}

/*************************************************************************************************/

void view_proxy_t::solve_up_with(::child_iterator first, ::child_iterator last,
        slice_select_t select)
{
    const layout_attributes_t::slice_t&     gslice(geometry_m.slice_m[select]);
    const extents_t::slice_t&               eslice(geometry_m.extents_m.slice_m[select]);
    long                                    length(eslice.frame_m.first + gslice.margin_m.first);
    long                                    rlength(eslice.frame_m.second + gslice.margin_m.second);
    
    guide_set_t&                            forward_guide_set(container_guide_set_m[select][layout_attributes_t::align_forward]);
    guide_set_t&                            reverse_guide_set(container_guide_set_m[select][layout_attributes_t::align_reverse]);
    guide_set_t::iterator                   forward_guide_iter(forward_guide_set.begin());
    guide_set_t::iterator                   reverse_guide_iter(reverse_guide_set.begin());
    
    long                                    additional_rlength(rlength);

    // accumulate the length and update the position.
    
    for (::reverse_child_iterator rfirst(last), rlast(first); rfirst != rlast; ++rfirst)
    {
        switch (rfirst->geometry_m.slice_m[select].alignment_m)
        {
         case layout_attributes_t::align_reverse:
         case layout_attributes_t::align_reverse_fill:
         case layout_attributes_t::align_forward_fill:
            {
            if (rfirst->geometry_m.slice_m[select].suppress_m) break;
            
            guide_set_t::iterator reverse_first_guide(reverse_guide_iter);
            guide_set_t& reverse_child_guide_set(rfirst->container_guide_set_m[select][layout_attributes_t::align_reverse]);
            
            for (guide_set_t::iterator guide(reverse_child_guide_set.begin());
                    guide != reverse_child_guide_set.end(); ++guide, ++reverse_guide_iter)
            {
                assert(reverse_guide_iter != reverse_guide_set.end());
                *reverse_guide_iter = std::max(*reverse_guide_iter, *guide + rlength);
            }
            
            if ((rfirst->geometry_m.placement_m == adobe::eve_t::place_leaf) && reverse_child_guide_set.size())
            {
                assert(reverse_first_guide != reverse_guide_set.end());
                long current_rlength(rlength);
                rlength = *reverse_first_guide - reverse_child_guide_set.front();
                additional_rlength += rlength - current_rlength;
            }
            }
            break;
         default:
            break;
        }
    
        rlength += rfirst->container_length_m[select] + rfirst->space_before_m;
    }
        
    for (::child_iterator iter(first); iter != last; ++iter)
    {
        length += iter->space_before_m;
        
        switch (iter->geometry_m.slice_m[select].alignment_m)
        {
         case layout_attributes_t::align_forward:
         case layout_attributes_t::align_forward_fill:
         case layout_attributes_t::align_reverse_fill:
            {
            if (iter->geometry_m.slice_m[select].suppress_m) break;
            
            guide_set_t::iterator forward_first_guide(forward_guide_iter);
            guide_set_t& forward_child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_forward]);
            
            for (guide_set_t::iterator guide(forward_child_guide_set.begin());
                    guide != forward_child_guide_set.end(); ++guide, ++forward_guide_iter)
            {
                assert(forward_guide_iter != forward_guide_set.end());
                *forward_guide_iter = std::max(*forward_guide_iter, *guide + length);
            }
            
            if ((iter->geometry_m.placement_m == adobe::eve_t::place_leaf) && forward_child_guide_set.size())
            {
                assert(forward_first_guide != forward_guide_set.end());
                length = *forward_first_guide - forward_child_guide_set.front();
            }
            }
            break;
         default:
            break;
        }
                
        length += iter->container_length_m[select];
    }
    
    // balance the guides
    
    if (gslice.balance_m && forward_guide_set.size() > 2)
    {
        guide_set_t::iterator       iter(adobe::max_adjacent_difference(forward_guide_set));
        guide_set_t::value_type     difference(*boost::next(iter) - *iter);
        
        guide_set_t::value_type accumulate(forward_guide_set.front());
        
        /*
        REVISIT (sparent) : This should be a call to generate - but I need a simple to create
        accumulate function object.
        */
                
        for (guide_set_t::iterator first(boost::next(forward_guide_set.begin())), last(forward_guide_set.end());
            first != last; ++first)
        {
            accumulate += difference;
            *first = accumulate;
        }
    }

    length += additional_rlength;

    // update properties on this container.
    
    measured_length_m[select] = length;
    container_length_m[select] = std::max(length, container_length_m[select]);
}

/*************************************************************************************************/

bool view_proxy_t::solve_down_with(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    bool                                                result(false);
    const layout_attributes_t::slice_t&                 gslice(geometry_m.slice_m[select]);
    const extents_t::slice_t&                           eslice(geometry_m.extents_m.slice_m[select]);
    long                                                length(eslice.frame_m.first + gslice.margin_m.first);
    long                                                rlength(eslice.frame_m.second + gslice.margin_m.second);
    guide_set_t::iterator                               guide_iter(container_guide_set_m[select][layout_attributes_t::align_forward].begin());
    guide_set_t::iterator                               reverse_guide_iter(container_guide_set_m[select][layout_attributes_t::align_reverse].begin());

    // accumulate the length and update the position.
    
    /*
        REVISIT (sparent) : After we move the guides on a container we don't know if we need to
        move the guides on subsequent children because the container may (or may not) grow in
        response. If it grows, it will shift the sibling over, which may resolve the guide for us.
        
        Becuase of this, we break out of this loop once we've adjusted one guide on one container.
        ... I suspect there is a pathelogical case here that's tending towards N^2. I need to 
        revist how we are solving but the actual case where this happens is complex enough
        (requiring 4 levels of nesting with two guides) that I don't think it is a huge problem.
    */
    
    for (::reverse_child_iterator rfirst(last), rlast(first); rfirst != rlast && !result; ++rfirst)
    {
        switch (rfirst->geometry_m.slice_m[select].alignment_m)
        {
         case layout_attributes_t::align_reverse:
         case layout_attributes_t::align_reverse_fill:
         case layout_attributes_t::align_forward_fill:
            {
            if (rfirst->geometry_m.slice_m[select].suppress_m) break;
            
            guide_set_t& child_guide_set(rfirst->container_guide_set_m[select][layout_attributes_t::align_reverse]);
        
            guide_set_t::iterator guide(child_guide_set.begin());
            guide_set_t::iterator guide_last(child_guide_set.end());
                    
            if ((rfirst->geometry_m.placement_m == eve_t::place_leaf) && guide != guide_last)
            {
                // Snap the current postion to the guide.
                assert(reverse_guide_iter != container_guide_set_m[select][layout_attributes_t::align_reverse].end());
                rlength = *reverse_guide_iter++ - *guide++;
            }
            
            for (; guide != guide_last; ++guide, ++reverse_guide_iter)
            {
                assert(reverse_guide_iter != container_guide_set_m[select][layout_attributes_t::align_reverse].end());
                long new_guide (*reverse_guide_iter - rlength);
                
                if (new_guide != *guide)
                    {
                    result = true;
                    *guide = new_guide;
                    }
            }
            }
            break;
         default:
            break;
        }
    
        rlength += rfirst->container_length_m[select] + rfirst->space_before_m;
    }
    
    if (result) return result;
        
    for (::child_iterator iter(first); (iter != last) && !result; ++iter)
    {
        length += iter->space_before_m;
        
        switch (iter->geometry_m.slice_m[select].alignment_m)
        {
         case layout_attributes_t::align_forward:
         case layout_attributes_t::align_forward_fill:
         case layout_attributes_t::align_reverse_fill:
            {
            if (iter->geometry_m.slice_m[select].suppress_m) break;
            
            guide_set_t& child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_forward]);
        
            guide_set_t::iterator guide(child_guide_set.begin());
            guide_set_t::iterator guide_last(child_guide_set.end());
                    
            if ((iter->geometry_m.placement_m == adobe::eve_t::place_leaf) && guide != guide_last)
            {
                // Snap the current postion to the guide.
                assert(guide_iter != container_guide_set_m[select][layout_attributes_t::align_forward].end());
                length = *guide_iter++ - *guide++;
            }
            
            for (; guide != guide_last; ++guide, ++guide_iter)
            {
                assert(guide_iter != container_guide_set_m[select][layout_attributes_t::align_forward].end());
                long new_guide (*guide_iter - length);
                
                if (new_guide != *guide)
                    {
                    result = true;
                    *guide = new_guide;
                    }
            }
            }
            break;
         default:
            break;
        }
                
        length += iter->container_length_m[select];
    }

    return result;
}

/*************************************************************************************************/

void view_proxy_t::solve_up_cross(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    const layout_attributes_t::slice_t&     gslice(geometry_m.slice_m[select]);
    const extents_t::slice_t&               eslice(geometry_m.extents_m.slice_m[select]);
    long                                    near_additional(eslice.frame_m.first + gslice.margin_m.first);
    long                                    far_additional(eslice.frame_m.second + gslice.margin_m.second);
    guide_set_t&                            forward_guide_set(container_guide_set_m[select][layout_attributes_t::align_forward]);
    guide_set_t&                            reverse_guide_set(container_guide_set_m[select][layout_attributes_t::align_reverse]);
    
    // Solve for guides
    
    for (::child_iterator iter(first); iter != last; ++iter)
    {
        const layout_attributes_t::slice_t& iter_gslice(iter->geometry_m.slice_m[select]);
        
        if (iter_gslice.suppress_m) continue;

        long forward_iter_length(near_additional);
        long reverse_iter_length(far_additional);
        
        switch (iter_gslice.alignment_m)
        {
         case layout_attributes_t::align_forward:
         case layout_attributes_t::align_forward_fill:
            forward_iter_length += iter->geometry_m.indent_m;
            break;
         case layout_attributes_t::align_reverse:
         case layout_attributes_t::align_reverse_fill:
            reverse_iter_length += iter->geometry_m.indent_m;
            break;
         default:
            break;
        }
        
        switch (iter_gslice.alignment_m)
        {
         case layout_attributes_t::align_forward:
         case layout_attributes_t::align_forward_fill:
         case layout_attributes_t::align_reverse_fill:
            {
            guide_set_t& forward_child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_forward]);
            
            for (guide_set_t::iterator guide_first(forward_child_guide_set.begin()),
                                       guide_last(forward_child_guide_set.end()),
                                       base_guide(forward_guide_set.begin()); guide_first != guide_last; ++guide_first, ++base_guide)
            {
                assert(base_guide != forward_guide_set.end());
                *base_guide = std::max(*base_guide, *guide_first + forward_iter_length);
            }
            }
            break;
         default:
            break;
        }
        
        switch (iter_gslice.alignment_m)
        {
         case layout_attributes_t::align_reverse:
         case layout_attributes_t::align_reverse_fill:
         case layout_attributes_t::align_forward_fill:
            {
            guide_set_t& reverse_child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_reverse]);
            
            for (guide_set_t::iterator guide_first(reverse_child_guide_set.begin()),
                                       guide_last(reverse_child_guide_set.end()),
                                       base_guide(reverse_guide_set.begin()); guide_first != guide_last; ++guide_first, ++base_guide)
            {
                assert(base_guide != reverse_guide_set.end());
                *base_guide = std::max(*base_guide, *guide_first + reverse_iter_length);
            }
            }
            break;
         default:
            break;
        }
    }
    
    long            length (near_additional + far_additional);
    
    /*
    Solve for length. For containers the effects of guides needs to be solved down then back up.
    For leaf-nodes we offset them directly.
    */
    
    for (::child_iterator iter(first); iter != last; ++iter)
    {
        const layout_attributes_t::slice_t& iter_gslice(iter->geometry_m.slice_m[select]);

        long forward_iter_length (near_additional);
        long reverse_iter_length (far_additional);

        switch (iter_gslice.alignment_m)
        {
         case layout_attributes_t::align_fill:
         case layout_attributes_t::align_forward:
            forward_iter_length += iter->geometry_m.indent_m;
            break;
         case layout_attributes_t::align_reverse:
         case layout_attributes_t::align_reverse_fill:
            reverse_iter_length += iter->geometry_m.indent_m;
            break;
         default:
            break;
        }
        
        if (iter->geometry_m.placement_m == eve_t::place_leaf && !iter_gslice.suppress_m)
        {
            switch (iter_gslice.alignment_m)
            {
             case layout_attributes_t::align_forward:
             case layout_attributes_t::align_forward_fill:
             case layout_attributes_t::align_reverse_fill:
                {
                guide_set_t& forward_child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_forward]);
                
                if (forward_child_guide_set.size())
                {
                    forward_iter_length += forward_guide_set.front() - (forward_child_guide_set.front() + forward_iter_length);
                }
                }
                break;
             default:
                break;
            }
            switch (iter_gslice.alignment_m)
            {
             case layout_attributes_t::align_reverse:
             case layout_attributes_t::align_reverse_fill:
             case layout_attributes_t::align_forward_fill:
                {
                guide_set_t& reverse_child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_reverse]);
                if (reverse_child_guide_set.size())
                {
                    reverse_iter_length += reverse_guide_set.front() - (reverse_child_guide_set.front() + reverse_iter_length);
                }
                }
                break;
             default:
                break;
            }
        }
        
        length = std::max(length, iter->container_length_m[select] + forward_iter_length + reverse_iter_length);
    }
    
    // update properties on this container.
    
    measured_length_m[select] = length;
    container_length_m[select] = std::max(length, container_length_m[select]);
}

/*************************************************************************************************/

bool view_proxy_t::solve_down_cross(::child_iterator first, ::child_iterator last, slice_select_t select)
{
    bool                                    result(false);
    const layout_attributes_t::slice_t&     gslice(geometry_m.slice_m[select]);
    const extents_t::slice_t&               eslice(geometry_m.extents_m.slice_m[select]);
    long                                    near_additional(eslice.frame_m.first + gslice.margin_m.first);
    long                                    far_additional(eslice.frame_m.second + gslice.margin_m.second);

    // Impose guide positions on containers.

    for (::child_iterator iter(first); iter != last; ++iter)
    {
        const layout_attributes_t::slice_t& iter_gslice(iter->geometry_m.slice_m[select]);

        if (iter_gslice.suppress_m) continue;

        guide_set_t::iterator forward_guide_iter(container_guide_set_m[select][layout_attributes_t::align_forward].begin());
        guide_set_t::iterator reverse_guide_iter(container_guide_set_m[select][layout_attributes_t::align_reverse].begin());

        // Leaf nodes are offset to the first guide.

        long length(near_additional);
        long rlength(far_additional);
        
        switch (iter_gslice.alignment_m)
        {
         case layout_attributes_t::align_fill:
         case layout_attributes_t::align_forward:
            length += iter->geometry_m.indent_m;
            break;
         case layout_attributes_t::align_reverse:
         case layout_attributes_t::align_reverse_fill:
            rlength += iter->geometry_m.indent_m;
            break;
         default:
            break;
        }
        
        switch (iter_gslice.alignment_m)
        {
         case layout_attributes_t::align_forward:
         case layout_attributes_t::align_forward_fill:
         case layout_attributes_t::align_reverse_fill:
            {
            guide_set_t& forward_child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_forward]);
            
            guide_set_t::iterator forward_guide_first(forward_child_guide_set.begin());
            guide_set_t::iterator forward_guide_last(forward_child_guide_set.end());

            if (iter->geometry_m.placement_m == adobe::eve_t::place_leaf)
            {
                if (forward_guide_first != forward_guide_last) length = *forward_guide_iter++ - *forward_guide_first++;
            }

            for (; forward_guide_first != forward_guide_last; ++forward_guide_first, ++forward_guide_iter)
            {
                long new_guide(*forward_guide_iter - length);
                if (new_guide != *forward_guide_first)
                {
                    result = true;
                    *forward_guide_first = new_guide;
                }
            }
            }
            break;
         default:
            break;
        }
        switch (iter_gslice.alignment_m)
        {
         case layout_attributes_t::align_reverse:
         case layout_attributes_t::align_reverse_fill:
         case layout_attributes_t::align_forward_fill:
            {
            guide_set_t& reverse_child_guide_set(iter->container_guide_set_m[select][layout_attributes_t::align_reverse]);
            
            guide_set_t::iterator reverse_guide_first(reverse_child_guide_set.begin());
            guide_set_t::iterator reverse_guide_last(reverse_child_guide_set.end());

            if (iter->geometry_m.placement_m == adobe::eve_t::place_leaf)
            {
                if (reverse_guide_first != reverse_guide_last) rlength = *reverse_guide_iter++ - *reverse_guide_first++;
            }

            for (; reverse_guide_first != reverse_guide_last; ++reverse_guide_first, ++reverse_guide_iter)
            {
                long new_guide(*reverse_guide_iter - rlength);
                if (new_guide != *reverse_guide_first)
                {
                    result = true;
                    *reverse_guide_first = new_guide;
                }
            }
            }
            break;
         default:
            break;
        }
    }
    return result;
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)

place_data_t::slice_t::slice_t() :
    length_m(0),
    position_m(0)
{ }

#endif

/*************************************************************************************************/

}// namespace adobe

/*************************************************************************************************/
