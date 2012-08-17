/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_TABLE_INDEX_HPP
#define ADOBE_TABLE_INDEX_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <functional>
#include <stdexcept>
#include <vector>

#include <boost/operators.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/next_prior.hpp>
#include <boost/type_traits/remove_reference.hpp>

#include <GG/adobe/algorithm/count.hpp>
#include <GG/adobe/algorithm/equal_range.hpp>
#include <GG/adobe/algorithm/lower_bound.hpp>
#include <GG/adobe/algorithm/sort.hpp>
#include <GG/adobe/algorithm/unique.hpp>
#include <GG/adobe/algorithm/upper_bound.hpp>

#include <GG/adobe/closed_hash.hpp>
#include <GG/adobe/functional.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*!
\class adobe::table_index
\ingroup other_container

\brief Utility class for indexing objects based on specific member variable values.

\par Terminology:
    - Row : any class or struct type; an element
    - Column : a data or logical member of an element
    - Table : any random access container of elements
    - Transformation Function : given an element returns 1 or more columns. The result is by const reference so columns must be adjacent
    - Key : a value which can be compared against a single member of an element

The table_index does <i>not</i> own the elements it indexes. Therefore the client is required to maintain the elements in the table_index as long as the table_index refers to them.

If the index is not sorted then the result of a lookup by key is undefined.

\todo (sparent) note here on why not auto sort.
*/

/*!
\typedef adobe::table_index::index_type

Container to store the elements of the table.
*/

/*!
\typedef adobe::table_index::transform_type

\ref concept_convertible_to_function used to transform an element into a specific key. 
*/

/*!
\typedef adobe::table_index::key_type

The table_index's key type, \c Key.
*/

/*!
\typedef adobe::table_index::value_type

The type of object referenced by the table_index.
*/

/*!
\typedef adobe::table_index::key_compare

Functor that compares two keys for ordering. 
*/

/*!
\typedef adobe::table_index::reference

Reference to \c T.
*/

/*!
\typedef adobe::table_index::const_reference

Const reference to \c T.
*/

/*!
\typedef adobe::table_index::size_type

An unsigned integral type.
*/

/*!
\typedef adobe::table_index::difference_type

A signed integral type.
*/

/*!
\typedef adobe::table_index::pointer

Pointer to \c T.
*/

/*!
\typedef adobe::table_index::const_pointer

Const pointer to \c T.
*/

/*!
\typedef adobe::table_index::iterator

Iterator used to iterate through a table_index.
*/

/*!
\typedef adobe::table_index::const_iterator

Const iterator used to iterate through a table_index.
*/

/*!
\typedef adobe::table_index::reverse_iterator

Iterator used to iterate backwards through a table_index.
*/

/*!
\typedef adobe::table_index::const_reverse_iterator

Const iterator used to iterate backwards through a table_index.
*/

/*!
\fn adobe::table_index::table_index(TransformPrimitive transform, const adobe::table_index::key_compare& compare)

\param transform \ref concept_convertible_to_function to be converted to the transformation function for this index
\param compare key comparison function for this index
*/

/*!
\fn adobe::table_index::table_index(const adobe::table_index::transform_type& transform, const adobe::table_index::key_compare& compare)

\param transform transformation function for this index
\param compare key comparison function for this index
*/

/*!
\fn adobe::table_index::table_index(InputIterator first, InputIterator last, const transform_type& transform, const key_compare& compare)

\param first iterator to first element to populate the table
\param last iterator to one-past-the-last element to populate the table
\param transform transformation function for this index
\param compare key comparison function for this index
*/

/*!
\fn adobe::table_index::size_type adobe::table_index::max_size() const

\return
    The largest possible size of the table_index.
*/

/*!
\fn adobe::table_index::size_type adobe::table_index::size() const

\return
    Returns the size of the table_index.
*/

/*!
\fn bool adobe::table_index::empty() const

\return
    \c true if the table_index's size is 0.
*/

/*!
\fn void adobe::table_index::unique()

Reduces the index to a single value per key.
*/

/*!
\fn adobe::table_index::iterator adobe::table_index::begin()

\return
    An iterator pointing to the beginning of the table_index.
*/

/*!
\fn adobe::table_index::const_iterator adobe::table_index::begin() const

\return
    A const_iterator pointing to the beginning of the table_index.
*/

/*!
\fn adobe::table_index::iterator adobe::table_index::end()

\return
    An iterator pointing to the end of the table_index.
*/

/*!
\fn adobe::table_index::const_iterator adobe::table_index::end() const

\return
    A const_iterator pointing to the end of the table_index.
*/

/*!
\fn adobe::table_index::reverse_iterator adobe::table_index::rbegin()

\return
    A reverse_iterator pointing to the beginning of the reversed table_index.
*/

/*!
\fn adobe::table_index::const_reverse_iterator adobe::table_index::rbegin() const

\return
    A const_reverse_iterator pointing to the beginning of the reversed table_index.
*/

/*!
\fn adobe::table_index::reverse_iterator adobe::table_index::rend()

\return
    A reverse_iterator pointing to the end of the reversed table_index.
*/

/*!
\fn adobe::table_index::const_reverse_iterator adobe::table_index::rend() const

\return
    A const_reverse_iterator pointing to the end of the reversed table_index.
*/

/*!
\fn adobe::table_index::reference adobe::table_index::at(adobe::table_index::size_type n)

\param n index into the table.

\return
    Element \c n in the table.
*/

/*!
\fn adobe::table_index::const_reference adobe::table_index::at(adobe::table_index::size_type n) const

\param n index into the table.

\return
    Element \c n in the table.
*/

/*!
\fn adobe::table_index::reference adobe::table_index::front()

\return
    The first element in the table.
*/

/*!
\fn adobe::table_index::const_reference adobe::table_index::front() const

\return
    The first element in the table.
*/

/*!
\fn adobe::table_index::reference adobe::table_index::back()

\return
    The last element in the table.
*/

/*!
\fn adobe::table_index::const_reference adobe::table_index::back() const

\return
    The last element in the table.
*/

/*!
\fn void adobe::table_index::push_back(adobe::table_index::value_type& x)

\param x The element to insert at the end of the table

\post
    \c x must exist as long as this table's reference to it exists.
*/

/*!
\fn void adobe::table_index::pop_back()

Eliminates the last element from the table.
*/

/*!
\fn adobe::table_index::iterator adobe::table_index::insert(adobe::table_index::iterator position, adobe::table_index::value_type& x)

\param position position at which the new element is to be inserted
\param x The element to insert as the position specified

\return
    An iterator pointing to the new element.

\post
    \c x must exist as long as this table's reference to it exists.
*/

/*!
\fn void adobe::table_index::insert(adobe::table_index::iterator position, InputIterator first, InputIterator last)

\param position position at which the new elements are to be inserted
\param first Iterator to the first new element to be inserted
\param last Iterator to one past the last new element to be inserted

\post
    The elements found in <code>(first, last]</code> must exist as long as this table's references to them exist.
*/

/*!
\fn adobe::table_index::iterator adobe::table_index::erase(adobe::table_index::iterator position)

\param position iterator to the element to be removed from the index

\return
    An iterator pointing to the next element after the one removed from the index
*/

/*!
\fn adobe::table_index::iterator adobe::table_index::erase(adobe::table_index::iterator first, adobe::table_index::iterator last)

\param first Iterator to the first element to be erased from the index
\param last Iterator to one past the last element to be erased from the index

\return
    An iterator pointing to to the next element after the ones removed from the index
*/

/*!
\fn void adobe::table_index::clear()

Erases the entire table_index.
*/

/*!
\fn adobe::table_index::index_type& adobe::table_index::index()

\return
    All the elements in the index in a \c std::vector
*/

/*!
\fn const adobe::table_index::index_type& adobe::table_index::index() const

\return
    All the elements in the index in a \c std::vector
*/

/*!
\fn void adobe::table_index::sort()

Sorts all the elements in the table based on their transformed values as compared with the key_compare function object.
*/

/*!
\fn adobe::table_index::reference adobe::table_index::operator[](const key_type& key)

\pre
    table must be sorted.

\param key key value to find in the table index.

\return
    One or more adjacent values whose keys match \c key.
*/

/*!
\fn adobe::table_index::const_reference adobe::table_index::operator[](const key_type& key) const

\pre
    table must be sorted.

\param key key value to find in the table index.

\return
    One or more adjacent values whose keys match \c key.
*/

/*!
\fn adobe::table_index::iterator adobe::table_index::find(const adobe::table_index::key_type& x)

\pre
    table must be sorted.

\param x key value to find within the index

\return
    Iterator to the element with key value \c x, or \c end() if there is none.
*/

/*!
\fn adobe::table_index::const_iterator adobe::table_index::find(const adobe::table_index::key_type& x) const

\pre
    table must be sorted.

\param x key value to find within the index

\return
    Iterator to the element with key value \c x, or \c end() if there is none.
*/

/*!
\fn adobe::table_index::size_type adobe::table_index::count(const adobe::table_index::key_type& x) const

\pre
    table must be sorted.

\param x key value to count within the index

\return
    Count of the elements with key value \c x.
*/

/*!
\fn adobe::table_index::iterator adobe::table_index::lower_bound(const adobe::table_index::key_type& x)

\pre
    table must be sorted.

\param x key value to find within the index

\return
    The first position where the element transformed to \c x could be inserted without violating the ordering of the table.
*/

/*!
\fn adobe::table_index::const_iterator adobe::table_index::lower_bound(const adobe::table_index::key_type& x) const

\pre
    table must be sorted.

\param x key value to find within the index

\return
    The first position where the element transformed to \c x could be inserted without violating the ordering of the table.
*/

/*!
\fn adobe::table_index::iterator adobe::table_index::upper_bound(const adobe::table_index::key_type& x)

\pre
    table must be sorted.

\param x key value to find within the index

\return
    The last position where the element transformed to \c x could be inserted without violating the ordering of the table.
*/

/*!
\fn adobe::table_index::const_iterator adobe::table_index::upper_bound(const adobe::table_index::key_type& x) const

\pre
    table must be sorted.

\param x key value to find within the index

\return
    The last position where the element transformed to \c x could be inserted without violating the ordering of the table.
*/

/*!
\fn std::pair<adobe::table_index::iterator, adobe::table_index::iterator> adobe::table_index::equal_range(const adobe::table_index::key_type& x)

\pre
    table must be sorted.

\param x key value to find within the index

\return
    Essentially a combination of the values returned by lower_bound and upper_bound
*/

/*!
\fn std::pair<adobe::table_index::const_iterator, adobe::table_index::const_iterator> adobe::table_index::equal_range(const adobe::table_index::key_type& x) const

\pre
    table must be sorted.

\param x key value to find within the index

\return
    Essentially a combination of the values returned by lower_bound and upper_bound
*/

/*!
\fn void swap(adobe::table_index& x, adobe::table_index& y)
\relates adobe::table_index

Swaps the contents of two table_indexes.

\param x the first table_index to swap.
\param y the second table_index to swap.
*/


template <  typename T,                     // models Regular
            typename H = boost::hash<T>,    // models UnaryFunction key_type -> size_t
            typename C = std::equal_to<T>,  // models EqualityComparison(key_type, key_type)
            typename P = identity<T> >      // models UnaryFunction T -> key_type
class hash_index
{
 public:
    typedef T                                                               value_type;
    typedef P                                                               key_function_type;

    typedef typename boost::remove_reference<typename key_function_type::result_type>::type
                                                                            key_type;

    typedef H                                                               hasher;
    typedef C                                                               key_equal;
    typedef value_type*                                                     pointer;
    typedef const value_type*                                               const_pointer;
    typedef value_type&                                                     reference;
    typedef const value_type&                                               const_reference;
    typedef std::size_t                                                     size_type;
    typedef std::ptrdiff_t                                                  difference_type;

    typedef unary_compose< key_function_type, indirect<value_type> >    indirect_key_function_type;

    typedef closed_hash_set<pointer, indirect_key_function_type, hasher, key_equal>
                                                                            index_type;

    typedef boost::indirect_iterator<typename index_type::iterator>         iterator;
    typedef boost::indirect_iterator<typename index_type::const_iterator>   const_iterator;
    typedef std::reverse_iterator<iterator>                                 reverse_iterator;
    typedef std::reverse_iterator<const_iterator>                           const_reverse_iterator;

    /*
        REVISIT (sparent@adobe.com) : This is a very limited set of constructors - need to have a 
        general strategy for which constructors to provide.
    */


    hash_index() { }

    /*
        NOTE: The constructor is templatized so things like mem_fun are not necessary. 
    */

    template <typename F> // F is convertible to key_function_type
    hash_index(hasher hf, key_equal eq, F kf) :
        index_m(0, hf, eq, compose(key_function_type(kf), indirect<value_type>()))
    { }

    //allocator_type get_allocator() const;
    size_type max_size() const { return index_m.max_size(); }
    
    size_type size() const { return index_m.size(); }
    bool      empty() const { return index_m.empty(); }
    size_type capacity() const { return index_m.capacity(); }

    void reserve(size_type n) { index_m.reserve(); }
    
    iterator begin() { return iterator(index_m.begin()); }
    iterator end() { return iterator(index_m.end()); }

    const_iterator begin() const { return const_iterator(index_m.begin()); }
    const_iterator end() const { return const_iterator(index_m.end()); }
    
    reverse_iterator rbegin() { return reverse_iterator(index_m.rbegin()); }
    reverse_iterator rend() { return reverse_iterator(index_m.rend()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(index_m.rbegin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(index_m.rend()); }
        
    std::pair<iterator, bool> insert(value_type& x)
    {
        std::pair<typename index_type::iterator, bool> result = index_m.insert(&x);
        return std::make_pair(iterator(result.first), result.second);
    }

    template <typename I>
    void insert(I f, I l)
    {
        index_m.insert( boost::make_transform_iterator(f, pointer_to<value_type>()),
                        boost::make_transform_iterator(l, pointer_to<value_type>()));
    }
    
    iterator insert(iterator i, value_type& x)
    {
        return iterator(index_m.insert(i, &x));
    }
    
    iterator erase(iterator i) { return index_m.erase(i.base()); }
    size_type erase(const key_type& x) { return index_m.erase(x); }

    void clear() { index_m.clear(); }
    
    index_type& index() { return index_m; }
    const index_type& index() const { return index_m; }

    iterator        find(const key_type& x) { return iterator(index_m.find(x)); }
    const_iterator  find(const key_type& x) const { return const_iterator(index_m.find(x)); }
    size_type       count(const key_type& x) const { return index_m.count(x); }
    
    iterator        lower_bound(const key_type& x) { return iterator(index_m.lower_bound()); }
    const_iterator  lower_bound(const key_type& x) const
    { return const_iterator(index_m.lower_bound()); }
    iterator        upper_bound(const key_type& x) { return iterator(index_m.upper_bound()); }
    const_iterator  upper_bound(const key_type& x) const
    { return const_iterator(index_m.upper_bound()); }
    
    std::pair<iterator, iterator> equal_range(const key_type& x)
    {
        std::pair<typename index_type::iterator, typename index_type::iterator> result = index_m.equal_range(x);
        return std::make_pair(iterator(result.first), iterator(result.second));
    }
    std::pair<const_iterator, const_iterator> equal_range(const key_type& x) const
    {
        std::pair<typename index_type::const_iterator, typename index_type::const_iterator> result
                = index_m.equal_range(x);

        return std::make_pair(const_iterator(result.first), const_iterator(result.second));
    }

    key_function_type key_function() const { return index_m.key_function(); }
    hasher hash_function() const { return index_m.hash_function(); }
    key_equal key_eq() const { return index_m.key_eq(); }

    friend void swap(hash_index& x, hash_index& y)
    {
        swap(x.index_m, y.index_m);
    }
 private:

    index_type              index_m;
};

/*************************************************************************************************/

template    <   typename Key,       // models Regular
                typename T,         // models Regular
                typename Transform  = mem_data_t<T, const Key>, // models UnaryFunction Key(T)
                typename Compare    = std::less<Key>    // models BinaryFunction bool(Key, Key)
            >
class table_index
{
    
 public:
    typedef typename std::vector<T*>                index_type;
    typedef Transform                               transform_type;
    
    typedef Key                                     key_type;
    typedef T                                       value_type;
    typedef Compare                                 key_compare; // Revisit?
//  typedef Allocator                               allocator_type;
    typedef T&                                      reference;
    typedef const T&                                const_reference;
    typedef typename index_type::size_type          size_type;
    typedef typename index_type::difference_type    difference_type;
    typedef T*                                      pointer;
    typedef const T*                                const_pointer;
    
    typedef boost::indirect_iterator<typename index_type::iterator>         iterator;
    typedef boost::indirect_iterator<typename index_type::const_iterator>   const_iterator;
    typedef std::reverse_iterator<iterator>                                 reverse_iterator;
    typedef std::reverse_iterator<const_iterator>                           const_reverse_iterator;
    
    template <class TransformPrimitive>
    explicit table_index(TransformPrimitive transform, const key_compare& compare = key_compare()) :
        transform_m(transform),
        compare_m(compare)
    { }
    explicit table_index(const transform_type&, const key_compare& = key_compare());
    
    template <typename InputIterator, typename TransformPrimitive>
    table_index(    InputIterator first,
                    InputIterator last,
                    TransformPrimitive transform,
                    const key_compare& compare = key_compare()) :
            transform_m(transform),
            compare_m(compare)
    {
        insert(begin(), first, last);
    }

    //allocator_type get_allocator() const;
    size_type max_size() const;
    
    size_type size() const;
    bool      empty() const;
    
    iterator        begin();
    const_iterator  begin() const;
    iterator        end();
    const_iterator  end() const;
    
    reverse_iterator        rbegin();
    const_reverse_iterator  rbegin() const;
    reverse_iterator        rend();
    const_reverse_iterator  rend() const;
    
    const_reference at(size_type n) const;
    reference       at(size_type n);
    
    reference       front();
    const_reference front() const;
    reference       back();
    const_reference back() const;
    
    void push_back(value_type&);
    void pop_back();
    
    iterator insert(iterator, value_type&);
    template <class InputIterator>
        void insert(iterator position, InputIterator first, InputIterator last);
        
    iterator erase(iterator location);
    iterator erase(iterator first, iterator last);
    void clear();
    
    index_type& index();
    const index_type& index() const;
    
    /*
        REVISIT (sparent) : If we wanted to allow sort() and unique() - and other algoriths to
        operate directly on the index rather than being built in, what would be required of the
        index interface?
    */
    
    void sort();
    
    // Operations on a sorted index.
    
    void unique();

    reference       operator[](const key_type&);
    const_reference operator[](const key_type&) const;
    
    iterator        find(const key_type& x);
    const_iterator  find(const key_type& x) const;
    size_type       count(const key_type& x) const;
    
    iterator        lower_bound(const key_type& x);
    const_iterator  lower_bound(const key_type& x) const;
    iterator        upper_bound(const key_type& x);
    const_iterator  upper_bound(const key_type& x) const;
    
    std::pair<iterator, iterator>               equal_range(const key_type& x);
    std::pair<const_iterator, const_iterator>   equal_range(const key_type& x) const;

    transform_type transform() const { return transform_m; }

    friend void swap(table_index& x, table_index& y)
    {
        swap(x.transform_m, y.transform_m);
        swap(x.compare_m, y.compare_m);
        swap(x.index_m, y.index_m);
    }
 private:

#ifndef ADOBE_NO_DOCUMENTATION
    struct indirect_compare_t : std::binary_function<pointer, pointer, bool>
    {
        typedef bool result_type;
    
        indirect_compare_t(transform_type& transform, const key_compare& compare) :
            transform_m(transform),
            compare_m(compare)
        { }

        bool operator () (pointer x, pointer y) const
        {
            return compare_m(transform_m(*x), transform_m(*y));
        }

     private:
        transform_type                  transform_m; /* REVISIT (sparent) : want reference here? */
        key_compare                     compare_m;
    };
    
#endif
 
    transform_type          transform_m;
    key_compare             compare_m;
    index_type              index_m;
};

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
table_index<Key, T, Compare, Transform>::table_index(   const transform_type&   transform, 
                                                        const key_compare&      compare) :
    transform_m(transform),
    compare_m(compare)
{ }

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::iterator
        table_index<Key, T, Compare, Transform>::begin()
{
    return iterator(index_m.begin());
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::const_iterator
        table_index<Key, T, Compare, Transform>::begin() const
{
    return const_iterator(index_m.begin());
}
    
/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::iterator 
        table_index<Key, T, Compare, Transform>::end()
{
    return iterator(index_m.end());
}
    
/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::const_iterator 
        table_index<Key, T, Compare, Transform>::end() const
{
    return const_iterator(index_m.end());
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::reverse_iterator
        table_index<Key, T, Compare, Transform>::rbegin()
{
    return reverse_iterator(index_m.rbegin());
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::const_reverse_iterator
        table_index<Key, T, Compare, Transform>::rbegin() const
{
    return const_reverse_iterator(index_m.rbegin());
}
    
/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::reverse_iterator 
        table_index<Key, T, Compare, Transform>::rend()
{
    return reverse_iterator(index_m.rend());
}
    
/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::const_reverse_iterator 
        table_index<Key, T, Compare, Transform>::rend() const
{
    return reverse_iterator(index_m.rend());
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::size_type
        table_index<Key, T, Compare, Transform>::max_size() const
{
    return index_m.max_size();
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::size_type
        table_index<Key, T, Compare, Transform>::size() const
{
    return index_m.size();
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline bool table_index<Key, T, Compare, Transform>::empty() const
{
    return index_m.empty();
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline void table_index<Key, T, Compare, Transform>::push_back(value_type& x)
{
    index_m.push_back(&x);
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline void table_index<Key, T, Compare, Transform>::pop_back()
{
    index_m.pop_back();
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::iterator
        table_index<Key, T, Compare, Transform>::insert(iterator position, value_type& x)
{
    return index_m.insert(position, &x);
}

/*************************************************************************************************/

/*
    REVISIT (sparent) : We should be able to greatly improve the table_index class - this function
    in particular could be much more efficient.
*/

template <class Key, class T, class Compare, class Transform>
template <class InputIterator>
inline void table_index<Key, T, Compare, Transform>::insert(iterator position, InputIterator first, InputIterator last)
{
    typename index_type::iterator iter = position.base();
    
    while (first != last) {
        iter = index_m.insert(iter, &*first);
        ++iter;
        ++first;
    }
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::iterator
        table_index<Key, T, Compare, Transform>::erase(iterator position)
{
    return index_m.erase(position.base());
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::iterator
        table_index<Key, T, Compare, Transform>::erase(iterator first, iterator last)
{
    return index_m.erase(first.base(), last.base());
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline void table_index<Key, T, Compare, Transform>::clear()
{
    index_m.clear();
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
void table_index<Key, T, Compare, Transform>::sort()
{
    adobe::sort(index_m, indirect_compare_t(transform_m, compare_m));
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
void table_index<Key, T, Compare, Transform>::unique()
{
    typename index_type::iterator i (adobe::unique(index_m, indirect_compare_t(transform_m, compare_m)));
    index_m.erase(i, index_m.end());
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::reference
        table_index<Key, T, Compare, Transform>::at(const size_type n)
{
    return *index_m.at(n);
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::const_reference
        table_index<Key, T, Compare, Transform>::at(const size_type n) const
{
    return *index_m.at(n);
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::reference
        table_index<Key, T, Compare, Transform>::operator [] (const key_type& key)
{
    iterator iter (find(key));
    
    if (iter == end())
        throw std::range_error("Key not present in table.");
    
    return *iter;
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::const_reference
        table_index<Key, T, Compare, Transform>::operator [] (const key_type& key) const
{
    const_iterator iter(find(key));
    
    if (iter == end())
        throw std::range_error("Key not present in table.");
    
    return *iter;
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::iterator
        table_index<Key, T, Compare, Transform>::find(const key_type& key)
{
    typename index_type::iterator iter = lower_bound(key).base();
    
    if (iter != index_m.end() && transform_m(**iter) != key)
        iter = index_m.end();

    return iter;
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::const_iterator
        table_index<Key, T, Compare, Transform>::find(const key_type& key) const
{
    typename index_type::const_iterator iter = lower_bound(key).base();
    
    if (iter != index_m.end() && transform_m(**iter) != key)
        iter = index_m.end();

    return iter;
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::size_type
        table_index<Key, T, Compare, Transform>::count(const key_type& key) const
{
    return adobe::count_if(index_m, bound_predicate_t(key, transform_m, compare_m));
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::iterator
        table_index<Key, T, Compare, Transform>::lower_bound(const key_type& key)
{
    return adobe::lower_bound(index_m, key, compare_m,
        boost::bind(transform_m, bind(indirect<value_type>(), _1)));
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::const_iterator
        table_index<Key, T, Compare, Transform>::lower_bound(const key_type& key) const
{
    return adobe::lower_bound(index_m, key, compare_m,
        boost::bind(transform_m, bind(indirect<value_type>(), _1)));
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::iterator
        table_index<Key, T, Compare, Transform>::upper_bound(const key_type& key)
{
    return adobe::upper_bound(index_m, key, compare_m,
        boost::bind(transform_m, bind(indirect<value_type>(), _1)));
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::const_iterator
        table_index<Key, T, Compare, Transform>::upper_bound(const key_type& key) const
{
    return adobe::upper_bound(index_m, key, compare_m,
        boost::bind(transform_m, bind(indirect<value_type>(), _1)));
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline std::pair<   typename table_index<Key, T, Compare, Transform>::iterator,
                    typename table_index<Key, T, Compare, Transform>::iterator>
        table_index<Key, T, Compare, Transform>::equal_range(const key_type& key)
{
    return adobe::equal_range(index_m, key, compare_m,
        boost::bind(transform_m, bind(indirect<value_type>(), _1)));
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline std::pair<   typename table_index<Key, T, Compare, Transform>::const_iterator,
                    typename table_index<Key, T, Compare, Transform>::const_iterator>
        table_index<Key, T, Compare, Transform>::equal_range(const key_type& key) const
{
    return adobe::equal_range(index_m, key, compare_m,
        boost::bind(transform_m, bind(indirect<value_type>(), _1)));
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline typename table_index<Key, T, Compare, Transform>::index_type&
        table_index<Key, T, Compare, Transform>::index()
{
    return index_m;
}

/*************************************************************************************************/

template <class Key, class T, class Compare, class Transform>
inline const typename table_index<Key, T, Compare, Transform>::index_type&
        table_index<Key, T, Compare, Transform>::index() const
{
    return index_m;
}

/*************************************************************************************************/

} // namespace adobe
    
/*************************************************************************************************/

#endif

/*************************************************************************************************/
