/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_FOREST_HPP
#define ADOBE_FOREST_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/algorithm/reverse.hpp>
#include <GG/adobe/iterator/set_next.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/next_prior.hpp>
#include <boost/range.hpp>

#include <cstddef>
#include <iterator>
#include <functional>

/*************************************************************************************************/

namespace adobe {
    
/*************************************************************************************************/

/*
    NOTE (sparent) : These are the edge index value - trailing as 0 and leading as 1 is done to
    reflect that it used to be a leading bool value. Changing the order of this enum requires 
    code review as some code relies on the test for leading. 
*/

enum
{
    forest_trailing_edge,
    forest_leading_edge
};

/*************************************************************************************************/

template <typename I> // I models FullorderIterator
inline void pivot(I& i) { i.edge() ^= 1; }

template <typename I> // I models FullorderIterator
inline I pivot_of(I i) { pivot(i); return i; }

/*************************************************************************************************/

template <typename I> // I models a FullorderIterator
inline I leading_of(I i) { i.edge() = forest_leading_edge; return i; }

template <typename I> // I models a FullorderIterator
inline I trailing_of(I i) { i.edge() = forest_trailing_edge; return i; }

/*************************************************************************************************/

template <typename I> // I models FullorderIterator
I find_parent(I i)
{
    do { i = trailing_of(i); ++i; } while (i.edge() != forest_trailing_edge);
    return i;
}

/*************************************************************************************************/

template <typename I> // I models FullorderIterator
bool has_children(const I& i)
{ return !i.equal_node(boost::next(leading_of(i))); }
    
/*************************************************************************************************/

template <typename I> // I models FullorderIterator
class child_iterator : public boost::iterator_adaptor<child_iterator<I>, I>
{
 public:
    child_iterator() { }
    explicit child_iterator(I x) : child_iterator::iterator_adaptor_(x) { }
    template <typename U> child_iterator(const child_iterator<U>& u) :
            child_iterator::iterator_adaptor_(u.base()) { }

 private:
    friend class boost::iterator_core_access;
    
    void increment() { pivot(this->base_reference()); ++this->base_reference(); }
    void decrement() { --this->base_reference(); pivot(this->base_reference()); }
};
    
/*************************************************************************************************/

template <typename I> // I models FullorderIterator
I find_edge(I x, std::size_t edge) { while (x.edge() != edge) ++x; return x; }

template <typename I> // I models FullorderIterator
I find_edge_reverse(I x, std::size_t edge) { while (x.edge() != edge) --x; return x; }
    
/*************************************************************************************************/

template <typename I, std::size_t Edge> // I models FullorderIterator
class edge_iterator : public boost::iterator_adaptor<edge_iterator<I, Edge>, I>
{
 public:
    edge_iterator() { }
    explicit edge_iterator(I x) : edge_iterator::iterator_adaptor_(find_edge(x, Edge))
    { }
    template <typename U> edge_iterator(const edge_iterator<U, Edge>& u) :
            edge_iterator::iterator_adaptor_(u.base()) { }
    
 private:
    friend class boost::iterator_core_access;
    
    void increment()
    { this->base_reference() = find_edge(boost::next(this->base()), Edge); }
    
    void decrement()
    { this->base_reference() = find_edge_reverse(boost::prior(this->base()), Edge); }
};
    
/*************************************************************************************************/

/*
    I need to establish dereferencable range vs. traversable range.
    
    Here [f, l) must be a dereferencable range and p() will not be applied outside that range
    although the iterators may travel beyond that range.
*/

template <  typename I, // I models a Forest
            typename P> // P models UnaryPredicate of value_type(I)
class filter_fullorder_iterator
    : public boost::iterator_adaptor<filter_fullorder_iterator<I, P>, I>
{
 public:
    filter_fullorder_iterator() { }
    
    filter_fullorder_iterator(I f, I l, P p) :
        filter_fullorder_iterator::iterator_adaptor_(skip_forward(f, l, p)),
        inside_m(true),
        first_m(f),
        last_m(l),
        predicate_m(p)
    { }
    
    filter_fullorder_iterator(I f, I l) :
        filter_fullorder_iterator::iterator_adaptor_(skip_forward(f, l, P())),
        inside_m(true),
        first_m(f),
        last_m(l)
    { }
    
    template <typename U> filter_fullorder_iterator(const filter_fullorder_iterator<U, P>& x) :
        filter_fullorder_iterator::iterator_adaptor_(x.base()),
        inside_m(x.inside_m),
        first_m(x.first_m),
        last_m(x.last_m),
        predicate_m(x.predicate_m)
    { }
    
    P predicate() const { return predicate_m; }
    
    std::size_t edge() const { return this->base().edge(); }
    std::size_t& edge() { return this->base_reference().edge(); }

    bool equal_node(const filter_fullorder_iterator& y) const { return this->base().equal_node(y.base()); }
        
 private:
    friend class boost::iterator_core_access;
    
    void increment()
    {
        I i = this->base();
        
        if (i == last_m) inside_m = false;
        ++i;
        if (i == first_m) inside_m = true;
        if (inside_m) i = skip_forward(i, last_m, predicate_m);
        this->base_reference() = i;
    }
        
    static I skip_forward(I f, I l, P p)
    // Precondition: l is on a leading edge
    {
        while((f.edge() == forest_leading_edge) && (f != l) && !p(*f)) {
            f.edge() = forest_trailing_edge;
            ++f;
        }
        return f;
    }
    
    static I skip_backward(I f, I l, P p)
    // Precondition: f is on a trailing edge
    {
        while((l.edge() == forest_trailing_edge) && (f != l) && !p(*l)) {
            l.edge() = forest_leading_edge;
            --l;
        }
        return l;
    }
    
    void decrement()
    {
        I i = this->base();
        
        if (i == first_m) inside_m = false;
        --i;
        if (i == last_m) inside_m = true;
        if (inside_m) i = skip_backward(first_m, i, predicate_m);
        this->base_reference() = i;
    }
    
    bool    inside_m;
    I       first_m;
    I       last_m;
    P       predicate_m;
};
    
/*************************************************************************************************/

/*
    REVISIT (sparent) : This is an interesting case - an edge is a property of an iterator but it
    is determined by examining a vertex in the graph. Here we need to examine the prior vertex
    to determine the edge. If the range is empty (or we are at the "end" of the range) then this
    examination is questionable.
    
    We let it go, because we know the forest structure is an infinite loop through the root. One
    answer to this would be to construct a reverse iterator which was not "off by one" for forest -
    but that might break people who assume base() is off by one for a reverse iterator, and it still
    assumes a root().
*/

template <typename I> // I models a FullorderIterator
class reverse_fullorder_iterator : public boost::iterator_facade<reverse_fullorder_iterator<I>,
           typename boost::iterator_value<I>::type, std::bidirectional_iterator_tag,
       typename boost::iterator_reference<I>::type>
{
    typedef typename boost::iterator_facade<reverse_fullorder_iterator<I>,
                typename boost::iterator_value<I>::type, std::bidirectional_iterator_tag,
                typename boost::iterator_reference<I>::type> 
        inherited_t;
 public:
    typedef I iterator_type;
    typedef typename inherited_t::reference reference;
 
    reverse_fullorder_iterator() : edge_m(forest_trailing_edge) { }
    reverse_fullorder_iterator(I x) : base_m(--x), edge_m(forest_leading_edge - base_m.edge())
    { }
    template <typename U> reverse_fullorder_iterator(const reverse_fullorder_iterator<U>& x) :
        base_m(x.base()), edge_m(x.edge_m)
    { }
    
    iterator_type base() const { return boost::next(base_m); }

    std::size_t edge() const { return edge_m; }
    std::size_t& edge() { return edge_m; }
        
    bool equal_node(const reverse_fullorder_iterator& y) const
    { return base_m.equal_node(y.base_m); }
    
 private:
    friend class boost::iterator_core_access;
    
    void increment()
    {
        base_m.edge() = forest_leading_edge - edge_m;
        --base_m;
        edge_m = forest_leading_edge - base_m.edge();
    }
    void decrement()
    {
        base_m.edge() = forest_leading_edge - edge_m;
        ++base_m;
        edge_m = forest_leading_edge - base_m.edge();
    }
    reference dereference() const { return *base_m; }
    
    bool equal(const reverse_fullorder_iterator& y) const
    { return (base_m == y.base_m) && (edge_m == y.edge_m); }
    
    I base_m;
    std::size_t edge_m;
};
    
/*************************************************************************************************/

template <typename I> // I models FullorderIterator
class depth_fullorder_iterator : public boost::iterator_adaptor<depth_fullorder_iterator<I>, I>
{
 public:
    typedef typename boost::iterator_adaptor<depth_fullorder_iterator<I>, I>::difference_type difference_type;
 
    depth_fullorder_iterator(difference_type d = 0) : depth_m(d) { }
    explicit depth_fullorder_iterator(I x, difference_type d = 0) :
        depth_fullorder_iterator::iterator_adaptor_(x),
        depth_m(d)
    { }
    template <typename U> depth_fullorder_iterator(const depth_fullorder_iterator<U>& x) :
        depth_fullorder_iterator::iterator_adaptor_(x.base()),
        depth_m(x.depth_m)
    { }

    difference_type depth() const { return depth_m; }
    std::size_t edge() const { return this->base().edge(); }
    std::size_t& edge() { return this->base_reference().edge(); }
    bool equal_node(depth_fullorder_iterator const& y) const { return this->base().equal_node(y.base()); }

 private:
    friend class boost::iterator_core_access;
    
    void increment()
    {
        std::size_t old_edge(edge());
        ++this->base_reference();
        if (old_edge == edge()) depth_m += difference_type(old_edge << 1) - 1;
    }
    void decrement()
    {
        bool old_edge(edge());
        --this->base_reference();
        if (old_edge == edge()) depth_m -= difference_type(old_edge << 1) - 1;
    }
    
    difference_type depth_m;
};
    
/*************************************************************************************************/

template <typename Forest> class child_adaptor;
template <typename T> class forest;
template <typename T> void swap(forest<T>&, forest<T>&);

/*************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)
namespace implementation {

template <typename D> // derived class
struct node_base
{
    enum next_prior_t
    {
        prior_s,
        next_s
    };

    typedef D*               node_ptr;
    typedef node_ptr&        reference;

    node_base()
    {
        // leading is 1, trailing is 0
        nodes_m[forest_leading_edge][std::size_t(next_s)] = static_cast<node_ptr>(this);
        nodes_m[forest_trailing_edge][std::size_t(prior_s)] = static_cast<node_ptr>(this);
    }
    
    node_ptr& link(std::size_t edge, next_prior_t link)
    { return nodes_m[edge][std::size_t(link)]; }
    
    node_ptr link(std::size_t edge, next_prior_t link) const
    { return nodes_m[edge][std::size_t(link)]; }

    node_ptr nodes_m[2][2];
};

template <typename T> // T models Regular
struct node : public node_base<node<T> >
{
    typedef T value_type;

    explicit node(const value_type& data) : data_m(data) { }
    
    value_type data_m;
};

/*************************************************************************************************/

template <typename T> class forest_const_iterator;

template <typename T> // T is value_type
class forest_iterator : public boost::iterator_facade<forest_iterator<T>,
                                                      T,
                                                      std::bidirectional_iterator_tag>
{
    typedef boost::iterator_facade<forest_iterator<T>,
                                   T,
                                   std::bidirectional_iterator_tag> inherited_t;
public:
    typedef typename inherited_t::reference         reference;
    typedef typename inherited_t::difference_type   difference_type;
    typedef typename inherited_t::value_type        value_type;

    forest_iterator() :
        node_m(0), edge_m(forest_leading_edge) { }

    forest_iterator(const forest_iterator& x) :
        node_m(x.node_m), edge_m(x.edge_m) { }

    std::size_t edge() const { return edge_m; }
    std::size_t& edge() { return edge_m; }
    bool equal_node(forest_iterator const& y) const { return node_m == y.node_m; }

private:
    friend class adobe::forest<value_type>;
    friend class boost::iterator_core_access;
    template <typename> friend class forest_iterator;
    template <typename> friend class forest_const_iterator;
    friend struct unsafe::set_next_fn<forest_iterator>;

    typedef node<T> node_t;

    reference dereference() const { return node_m->data_m; }
    
    void increment()
    {
        node_t* next(node_m->link(edge_m, node_t::next_s));
        
        if (edge_m) edge_m = std::size_t(next != node_m);
        else edge_m = std::size_t(next->link(forest_leading_edge, node_t::prior_s) == node_m);
        
        node_m = next;
    }
        
    void decrement()
    {
        node_t* next(node_m->link(edge_m, node_t::prior_s));
        
        if (edge_m) edge_m = std::size_t(next->link(forest_trailing_edge, node_t::next_s) != node_m);
        else edge_m = std::size_t(next == node_m);
        
        node_m = next;
    }
    
    bool equal(const forest_iterator& y) const
    { return (node_m == y.node_m) && (edge_m == y.edge_m); }

    node_t*     node_m;
    std::size_t edge_m;

    forest_iterator(node_t* node, std::size_t edge) :
        node_m(node), edge_m(edge) { }
};


/*************************************************************************************************/

template <typename T> // T is value_type
class forest_const_iterator : public boost::iterator_facade<forest_const_iterator<T>,
                                                      const T,
                                                      std::bidirectional_iterator_tag>
{
    typedef boost::iterator_facade<forest_const_iterator<T>,
                                   const T,
                                   std::bidirectional_iterator_tag> inherited_t;
public:
    typedef typename inherited_t::reference         reference;
    typedef typename inherited_t::difference_type   difference_type;
    typedef typename inherited_t::value_type        value_type;

    forest_const_iterator() :
        node_m(0), edge_m(forest_leading_edge) { }

    forest_const_iterator(const forest_const_iterator& x) :
        node_m(x.node_m), edge_m(x.edge_m) { }

    forest_const_iterator(const forest_iterator<T>& x) :
        node_m(x.node_m), edge_m(x.edge_m) { }

    std::size_t edge() const { return edge_m; }
    std::size_t& edge() { return edge_m; }
    bool equal_node(forest_const_iterator const& y) const { return node_m == y.node_m; }

private:
    friend class adobe::forest<value_type>;
    friend class boost::iterator_core_access;
    template <typename> friend class forest_const_iterator;
    friend struct unsafe::set_next_fn<forest_const_iterator>;

    typedef const node<T> node_t;

    reference dereference() const { return node_m->data_m; }
    
    void increment()
    {
        node_t* next(node_m->link(edge_m, node_t::next_s));
        
        if (edge_m) edge_m = std::size_t(next != node_m);
        else edge_m = std::size_t(next->link(forest_leading_edge, node_t::prior_s) == node_m);
        
        node_m = next;
    }
        
    void decrement()
    {
        node_t* next(node_m->link(edge_m, node_t::prior_s));
        
        if (edge_m) edge_m = std::size_t(next->link(forest_trailing_edge, node_t::next_s) != node_m);
        else edge_m = std::size_t(next == node_m);
        
        node_m = next;
    }
    
    bool equal(const forest_const_iterator& y) const
    { return (node_m == y.node_m) && (edge_m == y.edge_m); }

    node_t*     node_m;
    std::size_t edge_m;

    forest_const_iterator(node_t* node, std::size_t edge) :
        node_m(node), edge_m(edge) { }
};


/*************************************************************************************************/

} // namespace implementation
#endif

/*************************************************************************************************/

namespace unsafe {

template <typename T> // T is node<T>
struct set_next_fn<implementation::forest_iterator<T> >
{
    void operator()(implementation::forest_iterator<T> x, implementation::forest_iterator<T> y) const
    {
        typedef typename implementation::node<T> node_t;
    
        x.node_m->link(x.edge(), node_t::next_s) = y.node_m;
        y.node_m->link(y.edge(), node_t::prior_s) = x.node_m;
    }
};

} // namespace unsafe

/*************************************************************************************************/

template <typename T>
class forest
{
private:
    typedef implementation::node<T> node_t;
    friend class child_adaptor<forest<T> >;
public:
    // types
    typedef T&                                          reference;
    typedef const T&                                    const_reference;
    typedef implementation::forest_iterator<T>          iterator;
    typedef implementation::forest_const_iterator<T>    const_iterator;
    typedef std::size_t                                 size_type;
    typedef std::ptrdiff_t                              difference_type;
    typedef T                                           value_type;
    typedef T*                                          pointer;
    typedef const T*                                    const_pointer;
    typedef reverse_fullorder_iterator<iterator>          reverse_iterator;
    typedef reverse_fullorder_iterator<const_iterator>    const_reverse_iterator;

    typedef adobe::child_iterator<iterator>                    child_iterator;
/* qualification needed since: A name N used in a class S shall refer to the same declaration
   in its context and when re-evaluated in the completed scope of
   S. */

    typedef adobe::child_iterator<const_iterator>       const_child_iterator;
    typedef std::reverse_iterator<child_iterator>       reverse_child_iterator;
    
    typedef edge_iterator<iterator, forest_leading_edge>        preorder_iterator;
    typedef edge_iterator<const_iterator, forest_leading_edge>  const_preorder_iterator;
    typedef edge_iterator<iterator, forest_trailing_edge>       postorder_iterator;
    typedef edge_iterator<const_iterator, forest_trailing_edge> const_postorder_iterator;
    
#if !defined(ADOBE_NO_DOCUMENTATION)
    forest();
    ~forest() { clear(); }

    forest(const forest&);
    forest& operator=(forest x) { this->swap(x); return *this; }
    
    void swap(forest&);
#endif

    size_type size() const;
    size_type size();
    size_type max_size() const { return size_type(-1); }
    bool size_valid() const { return size_m != 0 || empty(); }
    bool empty() const { return begin() == end(); } // Don't test size which may be expensive
        
    // iterators
    iterator        root() { return iterator(tail(), forest_leading_edge); }
    
    iterator        begin() { return ++root(); }
    iterator        end() { return iterator(tail(), forest_trailing_edge); }
    const_iterator  begin() const { return ++const_iterator(tail(), forest_leading_edge); }
    const_iterator  end() const { return const_iterator(tail(), forest_trailing_edge); }
        
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    reference       front() { assert(!empty()); return *begin(); }
    const_reference front() const { assert(!empty()); return *begin(); }
    reference       back() { assert(!empty()); return *(--end()); }
    const_reference back() const { assert(!empty()); return *(--end()); }

    // modifiers
    void clear()
    {
        erase(begin(), end()); 
        assert(empty()); // Make sure our erase is correct
    }
    
    iterator erase(const iterator& position);
    iterator erase(const iterator& first, const iterator& last);
    
    iterator insert(const iterator& position, const T& x)
    {
        iterator result(new node_t(x), true);

        if (size_valid()) ++size_m;
        
        unsafe::set_next(boost::prior(position), result);
        unsafe::set_next(boost::next(result), position);
                
        return result;
    }
    
    void push_front(const T& x) { insert(begin(), x); }
    void push_back(const T& x) { insert(end(), x); }
    void pop_front() { assert(!empty()); erase(begin()); }
    void pop_back() { assert(!empty()); erase(--end()); }
    
    iterator insert(iterator position, const_child_iterator first, const_child_iterator last);
    
    iterator splice(iterator position, forest<T>& x);
    iterator splice(iterator position, forest<T>& x, iterator i);
    iterator splice(iterator position, forest<T>& x, child_iterator first, child_iterator last);
    iterator splice(iterator position, forest<T>& x, child_iterator first, child_iterator last, size_type count);

    iterator insert_parent(child_iterator front, child_iterator back, const T& x);
    void reverse(child_iterator first, child_iterator last);
    
private:
#if !defined(ADOBE_NO_DOCUMENTATION)

    friend class implementation::forest_iterator<value_type>;
    friend class implementation::forest_const_iterator<value_type>;
    friend struct unsafe::set_next_fn<iterator>;


#if 0
    struct node_base
    {
        enum next_prior_t
        {
            prior_s,
            next_s
        };
    
        typedef node*               node_ptr;
        typedef node_ptr&           reference;

        node_base()
        {
            // leading is 1, trailing is 0
            nodes_m[forest_leading_edge][std::size_t(next_s)] = static_cast<node*>(this);
            nodes_m[forest_trailing_edge][std::size_t(prior_s)] = static_cast<node*>(this);
        }
        
        node_ptr& link(std::size_t edge, next_prior_t link)
        { return nodes_m[edge][std::size_t(link)]; }
        
        node_ptr link(std::size_t edge, next_prior_t link) const
        { return nodes_m[edge][std::size_t(link)]; }

        node_ptr nodes_m[2][2];
    };

    struct node : public node_base
    {
        explicit node(const value_type& data) : data_m(data) { }
        
        value_type data_m;
    };

#endif

    size_type                                   size_m;
    implementation::node_base<node_t>           tail_m;
    
    node_t* tail() { return static_cast<node_t*>(&tail_m); }
    const node_t* tail() const { return static_cast<const node_t*>(&tail_m); }
#endif
};

/*************************************************************************************************/

template <typename T>
bool operator==(const forest<T>& x, const forest<T>& y)
{
    if (x.size() != y.size()) return false;

    for (typename forest<T>::const_iterator first(x.begin()), last(x.end()), pos(y.begin());
            first != last; ++first, ++pos)
    {
        if (first.edge() != pos.edge()) return false;
        if (first.edge() && (*first != *pos)) return false;
    }
    
    return true;
}

/*************************************************************************************************/

template <typename T>
bool operator!=(const forest<T>& x, const forest<T>& y)
{ return !(x == y); }

/*************************************************************************************************/

namespace unsafe {

template <typename I> // I models a FullorderIterator
struct set_next_fn<child_iterator<I> >
{
    void operator()(child_iterator<I> x, child_iterator<I> y)
    {
        unsafe::set_next(pivot_of(x.base()), y.base());
    }
};

} // namespace unsafe
        
/*************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)

template <typename T>
forest<T>::forest() :
        size_m(0)
{
    unsafe::set_next(end(), root());
}

/*************************************************************************************************/

template <typename T>
forest<T>::forest(const forest& x) :
        size_m(0)
{
    unsafe::set_next(end(), root());
    
    insert(begin(), const_child_iterator(x.begin()), const_child_iterator(x.end()));
}

/*************************************************************************************************/

template <typename T>
void forest<T>::swap(forest& tree)
{
    size_type old_size(size_valid() ? 0 : size());
    iterator last(splice(end(), tree));
    tree.splice(tree.end(), *this, child_iterator(begin()), child_iterator(last), old_size);
}
    
#endif

/*************************************************************************************************/

template <typename T>
typename forest<T>::size_type forest<T>::size()
{
    if (!size_valid())
    {
        const_preorder_iterator first(begin());
        const_preorder_iterator last(end());

        size_m = size_type(std::distance(first, last));
    }

    return size_m;
}

/*************************************************************************************************/

template <typename T>
typename forest<T>::size_type forest<T>::size() const
{
    if (size_valid()) return size_m;

    const_preorder_iterator first(begin());
    const_preorder_iterator last(end());

    return size_type(std::distance(first, last));
}

/*************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::erase(const iterator& first, const iterator& last)
{
    difference_type stack_depth(0);
    iterator        position(first);
    
    while (position != last)
    {
        if (position.edge() == forest_leading_edge)
        {
            ++stack_depth;
            ++position;
        }
        else
        {
            if (stack_depth > 0) position = erase(position);
            else ++position;
            stack_depth = std::max<difference_type>(0, stack_depth - 1);
        }
    }
    return last;
}
        
/*************************************************************************************************/

template <typename T>
void swap(forest<T>& x, forest<T>& y)
{ x.swap(y); }
        
/*************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::erase(const iterator& position)
{
    /*
        NOTE (sparent) : After the first call to set_next() the invariants of the forest are
        violated and we can't determing leading/trailing if we navigate from the effected node.
        So we gather all the iterators up front then do the set_next calls.
    */

    if (size_valid()) --size_m;

    iterator leading_prior(boost::prior(leading_of(position)));
    iterator leading_next(boost::next(leading_of(position)));
    iterator trailing_prior(boost::prior(trailing_of(position)));
    iterator trailing_next(boost::next(trailing_of(position)));
    
    if (has_children(position))
    {
        unsafe::set_next(leading_prior, leading_next);
        unsafe::set_next(trailing_prior, trailing_next);
    }
    else
    {
        unsafe::set_next(leading_prior, trailing_next);
    }
    
    delete position.node_m;
    
    return  position.edge() ? boost::next(leading_prior) : trailing_next;
}
        
/*************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::splice(iterator position, forest<T>& x)
{
    return splice(position, x, child_iterator(x.begin()), child_iterator(x.end()),
            x.size_valid() ? x.size() : 0);
}

/*************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::splice(iterator position, forest<T>& x, iterator i)
{
    i.edge() = forest_leading_edge;
    return splice(position, x, child_iterator(i), ++child_iterator(i), has_children(i) ? 0 : 1);
}

/*************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::insert(iterator pos, const_child_iterator f,
        const_child_iterator l)
{
    for (const_iterator first(f.base()), last(l.base()); first != last; ++first, ++pos)
    {
        if (first.edge()) pos = insert(pos, *first);
    }
    
    return pos;
}

/*************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::splice(iterator pos, forest<T>& x, child_iterator first,
        child_iterator last, size_type count)
{
    if (first == last || first.base() == pos) return pos;
    
    if (&x != this)
    {
        if (count)
        {
            if (size_valid()) size_m += count;
            if (x.size_valid()) x.size_m -= count;
        }
        else
        {
            size_m = 0;
            x.size_m = 0;
        }
    }
    
    iterator back(boost::prior(last.base()));
    
    unsafe::set_next(boost::prior(first), last);
    
    unsafe::set_next(boost::prior(pos), first.base());
    unsafe::set_next(back, pos);
    
    return first.base();
}

/*************************************************************************************************/

template <typename T>
inline typename forest<T>::iterator forest<T>::splice(iterator pos, forest<T>& x,
        child_iterator first, child_iterator last)
{ return splice(pos, x, first, last, 0); }

/*************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::insert_parent(child_iterator first, child_iterator last,
        const T& x)
{
    iterator result(insert(last.base(), x));
    if (first == last) return result;
    splice(trailing_of(result), *this, first, child_iterator(result));
    return result;
}

/*************************************************************************************************/

template <typename T>
void forest<T>::reverse(child_iterator first, child_iterator last)
{
    iterator prior(first.base());
    --prior;
    first = unsafe::reverse_nodes(first, last);
    unsafe::set_next(prior, first.base());
}

/*************************************************************************************************/

template <typename Forest>
class child_adaptor
{
public:
    typedef Forest                                  forest_type;
    typedef typename Forest::value_type             value_type;
    typedef typename Forest::iterator               iterator_type;
    typedef typename Forest::reference              reference;
    typedef typename Forest::const_reference        const_reference;
    typedef typename Forest::difference_type        difference_type;
    typedef typename Forest::child_iterator         iterator;

    child_adaptor(forest_type& f, iterator_type& i) :
        forest_m(f), iterator_m(i)
    { }

    iterator& back() { return *(--child_end(iterator_m)); }
    iterator& front() { return *(child_begin(iterator_m)); }

    void push_back(const value_type& x) { forest_m.insert(child_end(iterator_m).base(), x); }
    void push_front(const value_type& x) { forest_m.insert(child_begin(iterator_m).base(), x); }

    void pop_back() { forest_m.erase(--child_end(iterator_m).base()); }
    void pop_front() { forest_m.erase(child_begin(iterator_m).base()); }

private:
    child_adaptor(); // not defined

    forest_type&    forest_m;
    iterator_type&  iterator_m;
};

/*************************************************************************************************/

template <typename I> // I models FullorderIterator
child_iterator<I> child_begin(const I& x)
{ return child_iterator<I>(boost::next(leading_of(x))); }
    
/*************************************************************************************************/

template <typename I> // I models FullorderIterator
child_iterator<I> child_end(const I& x)
{ return child_iterator<I>(trailing_of(x)); }

/*************************************************************************************************/

/*
    NOTE (fbrereto) : The Doxygen documentation is inline for the functions below because their
                      signatures are particularly prone to translation error.
*/

/*!
\relates adobe::forest

\param x the FullorderRange to which the filter will be applied
\param p the predicate to be applied to the FullorderIterator

\return
    A filtered FullorderRange
*/

template <typename R, typename P> // R models FullorderRange
inline std::pair<filter_fullorder_iterator<typename boost::range_iterator<R>::type, P>,
                filter_fullorder_iterator<typename boost::range_iterator<R>::type, P> >
filter_fullorder_range(R& x, P p)
{
    typedef filter_fullorder_iterator<typename boost::range_iterator<R>::type, P> iterator;

    return std::make_pair(iterator(boost::begin(x), boost::end(x), p), iterator(boost::end(x), boost::end(x), p));
}

/*!
\relates adobe::forest

\param x the const FullorderRange to which the filter will be applied
\param p the predicate to be applied to value_type(R)

\return
    A filtered FullorderRange
*/

template <typename R, typename P> // R models FullorderRange
inline std::pair<filter_fullorder_iterator<typename boost::range_const_iterator<R>::type, P>,
        filter_fullorder_iterator<typename boost::range_const_iterator<R>::type, P> >
filter_fullorder_range(const R& x, P p)
{
    typedef filter_fullorder_iterator<typename boost::range_const_iterator<R>::type, P> iterator;

    return std::make_pair(iterator(p, boost::begin(x), boost::end(x)), iterator(p, boost::end(x), boost::end(x)));
}

/*************************************************************************************************/

/*
    REVISIT (sparent) : There should be some way to generalize this into a make_range - which is
    specialized.
    
    One option -
    
    reverse_range(R& x)
    
    Hmmm - maybe reverse_fullorder_iterator should be a specialization of std::reverse_iterator?
*/

/*!
\relates adobe::forest

\param x the FullorderRange which will be reversed

\return
    A reverse FullorderRange
*/

template <typename R> // R models FullorderRange
inline std::pair<reverse_fullorder_iterator<typename boost::range_iterator<R>::type>,
                 reverse_fullorder_iterator<typename boost::range_iterator<R>::type> >
    reverse_fullorder_range(R& x)
{
    typedef reverse_fullorder_iterator<typename boost::range_iterator<R>::type> iterator;

    return std::make_pair(iterator(boost::end(x)), iterator(boost::begin(x)));
}

/*!
\relates adobe::forest

\param x the const FullorderRange which will be reversed

\return
    A const reverse FullorderRange
*/

template <typename R> // R models FullorderRange
inline std::pair<reverse_fullorder_iterator<typename boost::range_const_iterator<R>::type>,
                reverse_fullorder_iterator<typename boost::range_const_iterator<R>::type> >
    reverse_fullorder_range(const R& x)
{
    typedef reverse_fullorder_iterator<typename boost::range_const_iterator<R>::type> iterator;

    return std::make_pair(iterator(boost::end(x)), iterator(boost::begin(x)));
}


/*************************************************************************************************/

/*!
\relates adobe::forest

\param x the FullorderRange which will be made into a depth FullorderRange

\return
    A depth FullorderRange
*/

template <typename R> // R models FullorderRange
inline std::pair<depth_fullorder_iterator<typename boost::range_iterator<R>::type>,
                depth_fullorder_iterator<typename boost::range_iterator<R>::type> >
    depth_range(R& x)
{
    typedef depth_fullorder_iterator<typename boost::range_iterator<R>::type> iterator;

    return std::make_pair(iterator(boost::begin(x)), iterator(boost::end(x)));
}

/*!
\relates adobe::forest

\param x the const FullorderRange which will be made into a const depth FullorderRange

\return
    A const depth FullorderRange
*/

template <typename R> // R models FullorderRange
inline std::pair<depth_fullorder_iterator<typename boost::range_const_iterator<R>::type>,
                depth_fullorder_iterator<typename boost::range_const_iterator<R>::type> >
    depth_range(const R& x)
{
    typedef depth_fullorder_iterator<typename boost::range_const_iterator<R>::type> iterator;

    return std::make_pair(iterator(boost::begin(x)), iterator(boost::end(x)));
}

/*************************************************************************************************/

/*!
\relates adobe::forest

\param x the FullorderRange which will be made into a postorder range

\return
    A postorder range
*/

template <typename R> // R models FullorderRange
inline std::pair<edge_iterator<typename boost::range_iterator<R>::type, forest_trailing_edge>,
                edge_iterator<typename boost::range_iterator<R>::type, forest_trailing_edge> >
    postorder_range(R& x)
{
    typedef edge_iterator<typename boost::range_iterator<R>::type, forest_trailing_edge> iterator;

    return std::make_pair(iterator(boost::begin(x)), iterator(boost::end(x)));
}

/*!
\relates adobe::forest

\param x the const FullorderRange which will be made into a const postorder range

\return
    A const postorder range
*/

template <typename R> // R models FullorderRange
inline std::pair<edge_iterator<typename boost::range_const_iterator<R>::type, forest_trailing_edge>,
                edge_iterator<typename boost::range_const_iterator<R>::type, forest_trailing_edge> >
    postorder_range(const R& x)
{
    typedef edge_iterator<typename boost::range_const_iterator<R>::type, forest_trailing_edge> iterator;

    return std::make_pair(iterator(boost::begin(x)), iterator(boost::end(x)));
}

/*************************************************************************************************/

/*!
\relates adobe::forest

\param x the FullorderRange which will be made into a preorder range

\return
    A preorder range
*/

template <typename R> // R models FullorderRange
inline std::pair<edge_iterator<typename boost::range_iterator<R>::type, forest_leading_edge>,
                edge_iterator<typename boost::range_iterator<R>::type, forest_leading_edge> >
    preorder_range(R& x)
{
    typedef edge_iterator<typename boost::range_iterator<R>::type, forest_leading_edge> iterator;

    return std::make_pair(iterator(boost::begin(x)), iterator(boost::end(x)));
}

/*!
\relates adobe::forest

\param x the const FullorderRange which will be made into a const preorder range

\return
    A const preorder range
*/

template <typename R> // R models FullorderRange
inline std::pair<edge_iterator<typename boost::range_const_iterator<R>::type, forest_leading_edge>,
                edge_iterator<typename boost::range_const_iterator<R>::type, forest_leading_edge> >
    preorder_range(const R& x)
{
    typedef edge_iterator<typename boost::range_const_iterator<R>::type, forest_leading_edge> iterator;

    return std::make_pair(iterator(boost::begin(x)), iterator(boost::end(x)));
}

/*************************************************************************************************/
    
} // namespace adobe

/*************************************************************************************************/

#endif
        
/*************************************************************************************************/
