/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_CIRCULAR_QUEUE_HPP
#define ADOBE_CIRCULAR_QUEUE_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/algorithm/equal.hpp>
#include <GG/adobe/iterator.hpp>
#include <GG/adobe/move.hpp>

#include <boost/operators.hpp>

#include <cassert>
#include <vector>

/*************************************************************************************************/

namespace adobe {
/*!
\defgroup other_container Other containers: circular_queue, table_index, static_table, etc.
\ingroup container
*/


/*!
\typedef adobe::circular_queue::value_type

The type of object, \c T, stored in the queue.
*/

/*!
\typedef adobe::circular_queue::reference

Reference to \c T.
*/

/*!
\typedef adobe::circular_queue::pointer

Pointer to \c T.
*/

/*!
\typedef adobe::circular_queue::const_reference

Const reference to \c T.
*/

/*!
\typedef adobe::circular_queue::size_type

Equivalent to std::size_t.
*/

/*!
\fn adobe::circular_queue::circular_queue(std::size_t capacity = 0)

Creates a circular_queue.

\param capacity
        Capacity for this queue.
*/
/*!
\fn adobe::circular_queue::size_type adobe::circular_queue::size() const

\return
    The number of elements retained in the queue.
*/

/*!
\fn adobe::circular_queue::size_type adobe::circular_queue::capacity() const

\return
    \c Capacity.
*/

/*!
\fn adobe::circular_queue::size_type adobe::circular_queue::max_size() const

Equivalent to \c capacity(), provided for completeness.

\return
    \c Capacity
*/

/*!
\fn bool adobe::circular_queue::empty() const

\return
    \true if the queue's size is 0.
*/

/*!
\fn bool adobe::circular_queue::full() const

\return
    \true if <code>size() == capacity()</code>.
*/

/*!
\fn void adobe::circular_queue::clear()

All elements are removed from the queue. Equivalent to <code>while (size()) pop_front();</code> except with constant complexity.
*/

/*!
\fn adobe::circular_queue::reference adobe::circular_queue::front()

\return
    A mutable reference to the element at the front of the queue, that is, the element least recently inserted.

\pre
    \c empty() is \false.
*/

/*!
\fn adobe::circular_queue::const_reference adobe::circular_queue::front() const

\return
    A const reference to the element at the front of the queue, that is, the element least recently inserted.

\pre
    \c empty() is \false.
*/

/*!
\fn void adobe::circular_queue::push_back(const value_type& x)

\param x
    Inserts \c x at the back of the queue.

\post
    If \c full(), the front item of the queue will be lost and the queue will remain full. Otherwise, \c size() will be incremented by \c 1.
*/

/*!
\fn void adobe::circular_queue::pop_front()

The element at the front of the queue is removed. The element is not destructed and may be returned with \c putback().

\pre
    \c empty() is \false.

\post
    \c size() will be decremented by \c 1.
*/

/*!
\fn void adobe::circular_queue::putback()

The last element popped from the front of the queue is returned to the front of the queue.
\pre
    Result undefined if putback() is called more times than pop_front().
\pre
    Result is undefined if \c full().
\post
    \c size() will be incremented by \c 1 and \c front() will return previous front.

\code
circular_queue<int> queue;

queue.push_back(10);
queue.push_back(20);
assert(queue.front() == 10);
queue.pop_front();
assert(queue.front() == 20);
queue.putback();
assert(queue.front() == 10);
\endcode
*/

/*!
\fn bool adobe::operator==(const circular_queue& x, const circular_queue& y)
\relates adobe::circular_queue

\param x first queue to compare
\param y second queue to compare

\complexity
    Linear. \c size() elements are compared (popped elements are not compared).
*/

/*!
\fn bool swap(circular_queue& x, circular_queue& y)
\relates adobe::circular_queue

\param x first queue to swap
\param y second queue to swap

\exception
    Unknown If the elements are swappable without throwing then the circular_queue will be swappable without throwing. See the requirements for \ref stldoc_Assignable.

\complexity
    Linear. \c size() of larger queue elements are swapped.
*/


/*************************************************************************************************/

#if 1 // REVISIT (fbrereto) : Possible compiler optimization?
    #define ADOBE_NOTHROW throw()
#else
    #define ADOBE_NOTHROW
#endif

/*************************************************************************************************/

template <typename T> class circular_queue;

template <typename T>
bool operator == (const circular_queue<T>& x, const circular_queue<T>& y);

template <typename T>
void swap(circular_queue<T>&, circular_queue<T>&);

/*************************************************************************************************/

/*!
\ingroup other_container

\brief A queue with a fixed capacity which supports putting back elements. Pushing more elements than there is capacity will pop the least recently pushed elements.

\template_parameters
    - \c T The queue's value type: the type of object that is stored in the queue.

\model_of
    - \ref stldoc_EqualityComparable
    - \ref stldoc_Assignable
    - \ref stldoc_DefaultConstructible

\type_requirements
    \c T is a model of \ref stldoc_Assignable.
*/
template <typename T>
class circular_queue : boost::equality_comparable<circular_queue<T> >
{
public:
    typedef T           value_type;
    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef T&          reference;
    typedef const T&    const_reference;
    typedef std::size_t size_type;
    
    circular_queue(std::size_t capacity = 0);

#if !defined(ADOBE_NO_DOCUMENTATION)
    circular_queue(const circular_queue& rhs);

    circular_queue& operator = (circular_queue rhs);
#endif // !defined(ADOBE_NO_DOCUMENTATION)

    size_type size() const ADOBE_NOTHROW;
    size_type max_size() const ADOBE_NOTHROW { return container_m.size(); }
    size_type capacity() const ADOBE_NOTHROW { return container_m.size(); }

    bool empty() const ADOBE_NOTHROW { return is_empty_m; }
    bool full() const ADOBE_NOTHROW { return !is_empty_m && begin_m == end_m; }
    
    void clear() ADOBE_NOTHROW { begin_m = end_m; is_empty_m = true; }

    reference       front() ADOBE_NOTHROW;
    const_reference front() const ADOBE_NOTHROW;

    template <typename U>
    void push_back(const U& x,  typename copy_sink<U, T>::type = 0);
    template <typename U>
    void push_back(U x, typename move_sink<U, T>::type = 0);
    
    void pop_front() ADOBE_NOTHROW;
    
    void putback() ADOBE_NOTHROW;

#if !defined(ADOBE_NO_DOCUMENTATION)
private:
    friend inline void swap(circular_queue& x, circular_queue& y)
    {
        swap(x.container_m, y.container_m);
        std::swap(x.begin_m, y.begin_m);
        std::swap(x.end_m, y.end_m);
        swap(x.is_empty_m, y.is_empty_m);
    }

    friend bool operator == <> (const circular_queue& x,
                                const circular_queue& y);
    
    typedef typename std::vector<value_type>        container_t;
    typedef typename container_t::iterator          iterator;
    typedef typename container_t::const_iterator    const_iterator;

    /* WARNING (sparent) : Order of members is important to initialization */
    container_t container_m;
    iterator    begin_m;
    iterator    end_m;
    bool        is_empty_m;
    /* WARNING (sparent) : Order of members is important to initialization */
    
    // Note that these ranges assume non-empty.
    
    typedef std::pair<const_iterator, const_iterator> const_range;
    
    const_range first_range() const
    { return const_range(begin_m, begin_m < end_m ? const_iterator(end_m) : boost::end(container_m)); }
    
    const_range second_range() const
    { return const_range(begin_m < end_m ? const_iterator(end_m) : boost::begin(container_m), end_m); }
    
#endif // !defined(ADOBE_NO_DOCUMENTATION)
};

/*************************************************************************************************/

template <typename T>
circular_queue<T>::circular_queue(std::size_t capacity) :
    container_m(capacity),
    begin_m(boost::begin(container_m)),
    end_m(boost::begin(container_m)),
    is_empty_m(true)
{ }

#if !defined(ADOBE_NO_DOCUMENTATION)

/*************************************************************************************************/

template <typename T>
circular_queue<T>::circular_queue(const circular_queue& rhs) :
    container_m (rhs.capacity()),
    begin_m     (boost::begin(container_m)),
    end_m       (boost::begin(container_m)),
    is_empty_m  (rhs.is_empty_m)
{
    if (is_empty_m) return;
    
    end_m = copy(rhs.first_range(), end_m);
    end_m = copy(rhs.second_range(), end_m);
}

/*************************************************************************************************/

template <typename T>
inline circular_queue<T>& circular_queue<T>::operator = (circular_queue rhs)
{  swap(*this, rhs); return *this; }

#endif // !defined(ADOBE_NO_DOCUMENTATION)
/*************************************************************************************************/

template <typename T>
typename circular_queue<T>::reference circular_queue<T>::front() ADOBE_NOTHROW
{
    assert(!empty());
    return *begin_m;
}

/*************************************************************************************************/

template <typename T>
typename circular_queue<T>::const_reference circular_queue<T>::front() const
        ADOBE_NOTHROW
{
    assert(!empty());
    return *begin_m;
}

/*************************************************************************************************/

template <typename T>
template <typename U>
void circular_queue<T>::push_back(const U& x, typename copy_sink<U, T>::type)
{
    *end_m = x;
    
    bool was_full (full());
    
    if (++end_m == boost::end(container_m)) end_m = boost::begin(container_m);
    if (was_full) begin_m = end_m;
    
    is_empty_m = false;
}

/*************************************************************************************************/

template <typename T>
template <typename U>
void circular_queue<T>::push_back(U x, typename move_sink<U, T>::type)
{
    *end_m = ::adobe::move(x);
    
    bool was_full (full());
    
    if (++end_m == boost::end(container_m)) end_m = boost::begin(container_m);
    if (was_full) begin_m = end_m;
    
    is_empty_m = false;
}

/*************************************************************************************************/

template <typename T>
void circular_queue<T>::pop_front() ADOBE_NOTHROW
{
    assert(!empty());
    if (++begin_m == boost::end(container_m))
        begin_m = boost::begin(container_m);
    is_empty_m = begin_m == end_m;
}

/*************************************************************************************************/

template <typename T>
void circular_queue<T>::putback() ADOBE_NOTHROW
{
    assert(!full());
    if (begin_m == boost::begin(container_m))
        begin_m = boost::end(container_m);
    --begin_m;
    is_empty_m = false;
}

/*************************************************************************************************/

template <typename T>
typename circular_queue<T>::size_type circular_queue<T>::size() const ADOBE_NOTHROW
{
    if (begin_m < end_m) return std::distance(begin_m, end_m);

    return is_empty_m ? 0 : capacity() - std::distance(end_m, begin_m);
}

/*************************************************************************************************/

template <typename T>
bool operator == (  const circular_queue<T>& x,
                    const circular_queue<T>& y)
{
/*
    REVISIT (sparent) : I'm trying to move the code towards equality of containers to mean "the
    size is the same and all the parts are the same." By making the segmented iterators part of
    a public begin, end interface this would simply be a generic. "equal_containers()" function.
*/
    typedef typename circular_queue<T>::const_range const_range;

    if (x.size() != y.size()) return false;
    
    const_range sequence1[] = { x.first_range(), x.second_range() };
    const_range sequence2[] = { y.first_range(), y.second_range() };
    
    return equal(make_segmented_range(sequence1),
                 make_segmented_iterator(sequence2));
}

/*************************************************************************************************/

#undef ADOBE_NOTHROW

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif // ADOBE_CIRCULAR_QUEUE_HPP

/*************************************************************************************************/
