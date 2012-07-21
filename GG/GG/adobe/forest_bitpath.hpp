/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_FOREST_BITPATH_HPP
#define ADOBE_FOREST_BITPATH_HPP

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/dynamic_bitset.hpp>
#include <boost/operators.hpp>

#include <GG/adobe/forest.hpp>
#include <GG/adobe/vector.hpp>

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

enum
{
    bitpath_first_child = 0,
    bitpath_next_sibling = 1
};

/**************************************************************************************************/

struct bitpath_t : boost::totally_ordered<bitpath_t>
{
    typedef unsigned char                     value_type;
    typedef boost::dynamic_bitset<value_type> path_type;
    typedef path_type::size_type              size_type;

    bitpath_t() :
        path_m(size_type(1), path_type::block_type(1))
    { }

    template <typename ForestFullorderIterator>
    bitpath_t(ForestFullorderIterator node,
              ForestFullorderIterator root)
    {
        while (true)
        {
            node = --adobe::leading_of(node);
    
            if (node == root)
                break;
    
            path_m.push_back(node.edge() == adobe::forest_trailing_edge);
        }

        path_m.push_back(true);
    }

    template <typename Container>
    explicit bitpath_t(const Container& x) :
        path_m(boost::begin(x), boost::end(x))
    {
        clip_zero_fill();
    }

    explicit bitpath_t(const path_type& path) :
        path_m(path)
    {
        clip_zero_fill();
    }

    bitpath_t(const bitpath_t& rhs) :
        path_m(rhs.path_m)
    { }

    bool empty() const
    { return path_m.empty(); }
    size_type size() const
    { return path_m.size(); }

    bool valid() const
    { return !empty() && path_m[path_m.size() - 1] == 1; }

    bool operator[](size_type n) const
    { return path_m[n]; }

    void push(bool value = bitpath_first_child)
    {
        path_m.resize(path_m.size() + 1);
    
        path_m <<= 1;
    
        path_m[0] = value;
    }

    bool pop()
    {
        bool result(path_m[0]);

        path_m >>= 1;

        path_m.resize(path_m.size() - 1);

        return result;
    }

    adobe::vector<value_type> portable() const
    {
        adobe::vector<value_type> result;

        result.reserve(path_m.size() / path_m.bits_per_block + 1);

        to_block_range(path_m, std::back_inserter(result));

        return result;
    }

    inline friend bool operator==(const bitpath_t& x, const bitpath_t& y)
    { return x.size() == y.size() && x.path_m == y.path_m; }

    inline friend bool operator<(const bitpath_t& x, const bitpath_t& y)
    {
        bitpath_t::size_type xs(x.size());
        bitpath_t::size_type ys(y.size());

        // dynamic_bitset asserts if xs != ys

        return xs < ys || (xs == ys && x.path_m < y.path_m);
    }

    inline friend std::ostream& operator<<(std::ostream& s, const bitpath_t& x)
    { return s << x.path_m; }

private:
    void clip_zero_fill()
    {
        // bitpaths must always start with a 1, so we
        // need to clip the zero-fill from the import.

        size_type orig(path_m.size());
        size_type size(orig);
    
        while (--size)
            if (path_m[size])
                break;

        if (orig != ++size)
            path_m.resize(size);
    }

    path_type path_m;
};

/**************************************************************************************************/

inline static const adobe::bitpath_t& nbitpath()
{
    static adobe::bitpath_t bitpath_s;

    while (bitpath_s.valid())
        bitpath_s.pop();

    return bitpath_s;
}

/**************************************************************************************************/
/*!
    Returns true if you would traverse through the node represented by candidate
    in order to get to the node represented by path.

    precondition: path.valid() && candidate.valid()

    passes_through(a, a) == true
*/
inline bool passes_through(const bitpath_t& path, const bitpath_t& candidate)
{
    if (!path.valid() || !candidate.valid())
        return false;

    bitpath_t::size_type path_size(path.size());
    bitpath_t::size_type candidate_size(candidate.size());

    if (path_size < candidate_size)
        return false;
    else if (path_size == candidate_size)
        return path == candidate;

    bitpath_t::size_type offset(path_size - candidate_size);

    while (candidate_size != 0)
        if (path[offset + --candidate_size] != candidate[candidate_size])
            return false;

    return true;
}

/**************************************************************************************************/

template <typename Forest>
typename Forest::iterator traverse(const bitpath_t& path, Forest& f)
{
    if (path.empty() || f.empty())
        return f.end();

    int                       highest_bit(path.size() - 1);
    typename Forest::iterator result(f.begin());

    while (highest_bit-- && result.edge() == forest_leading_edge)
    {
        // Traversing the forest this way will always leave the
        // forest iterator on the leading edge. Should a pivot
        // take place it means the node specified by the bitpath
        // does not exist in the forest, so we return the closest
        // we can get to a successful traversal.

        if (path[highest_bit]) // follow next sibling
            result = ++trailing_of(result);
        else // follow first child
            result = ++leading_of(result);
    }

    return result;
}

/**************************************************************************************************/

template <typename Forest>
typename Forest::const_iterator traverse(const bitpath_t& path, const Forest& f)
{
    return traverse(path, const_cast<Forest&>(f));
}

/**************************************************************************************************/

inline bitpath_t parent_of(const bitpath_t& src)
{
    bitpath_t result(src);

    while (true)
        if (!result.pop() || result.empty())
            break;

    return result;
}

/**************************************************************************************************/

inline bitpath_t prior_sibling_of(const bitpath_t& src)
{
    bitpath_t result(src);

    while (true)
        if (result.pop() || result.empty())
            break;

    return result;
}

/**************************************************************************************************/

inline bitpath_t first_child_of(const bitpath_t& src)
{
    bitpath_t result(src);

    result.push();

    return result;
}

/**************************************************************************************************/

inline bitpath_t next_sibling_of(const bitpath_t& src)
{
    bitpath_t result(src);

    result.push(true);

    return result;
}

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

#endif

/**************************************************************************************************/
