/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_VECTOR_HPP
#define ADOBE_VECTOR_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/vector_fwd.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>

#include <boost/operators.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/has_nothrow_constructor.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/utility/enable_if.hpp>

#include <GG/adobe/empty.hpp>
#include <GG/adobe/typeinfo.hpp>
#include <GG/adobe/memory.hpp>
#include <GG/adobe/algorithm/minmax.hpp>

#include <GG/adobe/move.hpp>
#include <GG/adobe/implementation/swap.hpp>

#ifdef ADOBE_STD_SERIALIZATION
#include <GG/adobe/iomanip.hpp>
#endif

/*************************************************************************************************/

namespace adobe {
namespace version_1 {

/*!
\defgroup container Containers
\ingroup asl_libraries
 */


/*************************************************************************************************/

//!\ingroup abi_container
template <typename T,   // T models Regular
          typename A>   // A models Allocator(T)
class vector : boost::totally_ordered<vector<T, A>, vector<T, A> >
{
 public:
    typedef T&                              reference;
    typedef const T&                        const_reference;
    typedef T*                              iterator;
    typedef const T*                        const_iterator;
    typedef std::size_t                     size_type;
    typedef std::ptrdiff_t                  difference_type;
    typedef T                               value_type;
    typedef A                               allocator_type;
    typedef T*                              pointer;
    typedef const T*                        const_pointer;
    typedef std::reverse_iterator<T*>       reverse_iterator;
    typedef std::reverse_iterator<const T*> const_reverse_iterator;
    
 private:
    struct header_t
    {
        struct compact_header_t
        {
            boost::compressed_pair<A, T*> allocate_finish_m;
            T* end_of_storage_m;
        };
        aligned_storage<compact_header_t> header_m;
        T  storage_m[1];

        allocator_type& allocator() { return header_m.get().allocate_finish_m.first(); }
        const allocator_type& allocator() const { return header_m.get().allocate_finish_m.first(); }

        pointer& finish() { return header_m.get().allocate_finish_m.second(); }
        const pointer& finish() const { return header_m.get().allocate_finish_m.second(); }

        pointer& end_of_storage() { return header_m.get().end_of_storage_m; }
        const pointer& end_of_storage() const { return header_m.get().end_of_storage_m; }
    };

    header_t* header_m;
    
    void set_finish(T* x)
    {
        assert(header_m != 0 || x == 0);
        if (header_m) header_m->finish() = x;
    }
    
    const T* end_of_storage() const { return header_m ? header_m->end_of_storage() : 0; }
    
    static header_t* allocate(allocator_type, std::size_t);
    
    size_type remaining() const { return end_of_storage() - end(); }
    
    template <typename I> // I models InputIterator
    void append(I f, I l) { append(f, l, typename std::iterator_traits<I>::iterator_category()); }
    
    template <typename I> // I models InputIterator
    void append(I f, I l, std::input_iterator_tag);
    
    template <typename I> // I models ForwardIterator
    void append(I f, I l, std::forward_iterator_tag);
    
    template <typename I> // I models InputIterator
    void move_append(I f, I l)
    { move_append(f, l, typename std::iterator_traits<I>::iterator_category()); }
    
    template <typename I> // I models InputIterator
    void move_append(I f, I l, std::input_iterator_tag);
    
    template <typename I> // I models ForwardIterator
    void move_append(I f, I l, std::forward_iterator_tag);
    
    template <typename I> // I models InputIterator
    iterator insert(iterator p, I f, I l, std::input_iterator_tag);
    
    template <typename I> // I models ForwardIterator
    iterator insert(iterator p, I f, I l, std::forward_iterator_tag);
   
 public:
    // 23.2.4.1 construct/copy/destroy

    explicit vector(const allocator_type& a) : header_m(allocate(a, 0)) { }
    vector() : header_m(0) { }
    
    explicit vector(size_type n) : header_m(allocate(allocator_type(), n))
    {
        std::uninitialized_fill_n(end(), n, value_type());
        set_finish(end() + n);
    }
    
    vector(size_type n, const value_type& x) : header_m(allocate(allocator_type(), n))
    {
        std::uninitialized_fill_n(end(), n, x);
        set_finish(end() + n);
    }

    vector(size_type n, const value_type& x, const allocator_type& a) : header_m(allocate(a, n))
    {
        std::uninitialized_fill_n(end(), n, x);
        set_finish(end() + n);
    }

    vector(const vector& x) : header_m(allocate(x.get_allocator(), x.size()))
    {
#ifndef NDEBUG
    /* REVISIT (sparent) : MS stupid "safety check" doesn't known about empty ranges. */
        set_finish(x.begin() == x.end() ? end() : std::uninitialized_copy(x.begin(), x.end(), end()));
#else
        set_finish(std::uninitialized_copy(x.begin(), x.end(), end()));
#endif
    }

    template <typename I> // I models InputIterator
    vector(I f, I l, typename boost::disable_if<boost::is_integral<I> >::type* = 0) : header_m(0)
    { append(f, l); }

    template <typename I> // I models InputIterator
    vector(I f, I l, const allocator_type& a,
        typename boost::disable_if<boost::is_integral<I> >::type* = 0) : header_m(allocate(a), 0)
    { append(f, l); }

    ~vector() {
        if (header_m) {
            clear();

            typename allocator_type::template rebind<char>::other alloc(get_allocator());
            alloc.deallocate(reinterpret_cast<char*>(header_m),
                (end_of_storage() - begin()) * sizeof(T) + (sizeof(header_t) - sizeof(T)));
        }
    }

    // adobe addition
    
    vector(move_from<vector> x) : header_m(x.source.header_m) { x.source.header_m = 0; }

    allocator_type get_allocator() const
    { return header_m ? header_m->allocator() : allocator_type(); }
    
    iterator begin() { return header_m ? &header_m->storage_m[0] : 0; }
    iterator end() { return header_m ? header_m->finish() : 0; }
    
    const_iterator begin() const { return header_m ? &header_m->storage_m[0] : 0; }
    const_iterator end() const { return header_m ? header_m->finish() : 0; }
    
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    
    size_type size() const { return size_type(end() - begin()); }
    size_type max_size() const { return size_type(-1) / sizeof(value_type); }
    
    size_type capacity() const { return size_type(end_of_storage() - begin()); }
    bool empty() const { return begin() == end(); }
    
    reference operator[](size_type n) { assert(n < size()); return *(begin() + n); }
    const_reference operator[](size_type n) const { assert(n < size()); return *(begin() + n); }
    
    /*
        REVISIT (sparent@adobe.com): at() explicitly omitted because it pulls in out_of_range
        which inherits from logic_error and uses std::string.
    */
    
    vector& operator=(vector x) { swap(x); return *this; }
    
#if 0
    template <typename I> // I models ForwardIterator
    vector(I f, I l, move_ctor) : header_m(0)
    { move_append(f, l); }

#endif
    
    void reserve(size_type n);
    
    reference front() { assert(!empty()); return *begin(); }
    const_reference front() const { assert(!empty()); return *begin(); }
    
    reference back() { assert(!empty()); return *(end() - 1); }
    const_reference back() const { assert(!empty()); return *(end() - 1); }
    
    template <typename U>
    void push_back(const U& x,  typename copy_sink<U, T>::type = 0)
    {
        if (remaining()) {
            construct<value_type>(end(), x);
        } else {
            value_type tmp(x); // copy in case the reallocation moves the object
            reserve(size() ? 2 * size() : 1);
            construct(end(), tmp);
        }
        set_finish(end() + 1);
    }
    
    template <typename U>
    void push_back(U x, typename move_sink<U, T>::type = 0)
    { move_append(&x, &x + 1); }
    
    void pop_back() { assert(!empty()); resize(size() - 1); }
    
    void swap(vector& x) { std::swap(header_m, x.header_m); }
    
    template <typename U>
    iterator insert(iterator p, const U& x, typename copy_sink<U, T>::type = 0)
    {
        if (remaining()) return insert(p, &x, &x + 1);
        else { value_type tmp(x); return insert(p, &tmp, &tmp + 1); }
    }
    
    template <typename U>
    iterator insert(iterator p, U x, typename move_sink<U, T>::type = 0)
    { return move_insert(p, &x, &x + 1); }
    
    template <typename I> // I models InputIterator
    iterator insert(iterator p, I f, I l, typename boost::disable_if<boost::is_integral<I> >::type* = 0)
    { return insert(p, f, l, typename std::iterator_traits<I>::iterator_category()); }
    
    template <typename I> // I models ForwardIterator
    iterator move_insert(iterator p, I f, I l);
    
    iterator insert(iterator p, size_type n, const T& x);
    
    iterator erase(iterator pos) { assert(pos != end()); return erase(pos, pos + 1); }
    
    iterator erase(iterator f, iterator l);
    
    void clear() { erase(begin(), end()); }
    
    void resize(size_type n);
    
    void resize(size_type n, const value_type& x);
    
    friend inline bool operator==(const vector& x, const vector& y)
    {
        return (x.size() == y.size()) && std::equal(x.begin(), x.end(), y.begin());
    }
    
    friend inline bool operator<(const vector& x, const vector& y)
    {
        return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
    }
    
    friend inline void swap(vector& x, vector& y) { x.swap(y); }
};

template <typename T, typename A>
typename vector<T, A>::header_t* vector<T, A>::allocate(allocator_type a, std::size_t n)
{
    if (n == 0) {
        if (a == allocator_type()) return 0;
         n = 1;
    }

    typename allocator_type::template rebind<char>::other alloc(a);

    header_t* result = reinterpret_cast<header_t*>(alloc.allocate(sizeof(header_t) - sizeof(T)
            + n * sizeof(T)));
    construct(&result->allocator(), a);
    result->finish() = &result->storage_m[0];
    result->end_of_storage() = result->finish() + n;
    
    return result;
}

template <typename T, typename A>
template <typename I> // I models InputIterator
void vector<T, A>::append(I f, I l, std::input_iterator_tag) { while (f != l) { push_back(*f); ++f; } }

template <typename T, typename A>
template <typename I> // I models InputIterator
void vector<T, A>::move_append(I f, I l, std::input_iterator_tag)
{ while (f != l) { push_back(::adobe::move(*f)); ++f; } }
    
template <typename T, typename A>
template <typename I> // I models ForwardIterator
void vector<T, A>::append(I f, I l, std::forward_iterator_tag)
{
    size_type n(std::distance(f, l));
    
        if (remaining() < n) reserve((adobe::max)(size() + n, 2 * size()));
    set_finish(std::uninitialized_copy(f, l, end()));
}
    
template <typename T, typename A>
template <typename I> // I models ForwardIterator
void vector<T, A>::move_append(I f, I l, std::forward_iterator_tag)
{
    size_type n(std::distance(f, l));
    
        if (remaining() < n) reserve((adobe::max)(size() + n, 2 * size()));
    set_finish(::adobe::uninitialized_move(f, l, end()));
}
    
template <typename T, typename A>
template <typename I> // I models InputIterator
typename vector<T, A>::iterator vector<T, A>::insert(iterator p, I f, I l, std::input_iterator_tag)
{
    size_type o(p - begin());
    size_type s = size();
    append(f, l);
    // REVISIT (sparent) : This could be a move based rotate
    std::rotate(begin() + o, begin() + s, end());
    return end() - s + o;
}
    
template <typename T, typename A>
template <typename I> // I models ForwardIterator
typename vector<T, A>::iterator vector<T, A>::insert(iterator p, I f, I l, std::forward_iterator_tag)
{
    size_type n(std::distance(f, l));
    iterator  last = end();
    size_type before = p - begin();

    if (remaining() < n) {
        vector tmp;
        tmp.reserve((adobe::max)(size() + n, 2 * size()));
        tmp.move_append(begin(), p);
        tmp.append(f, l);
        tmp.move_append(p, last);
        swap(tmp);
    } else {
        size_type after(last - p);
        
        if (n < after) {
            move_append(last - n, last);
            ::adobe::move_backward(p, last - n, last);
            std::copy(f, l, p);
        } else {
            I m = f;
            std::advance(m, after);
            append(m, l);
            move_append(p, last);
            std::copy(f, m, p);
        }
    }
    return begin() + before + n;
}
    
template <typename T, typename A>
template <typename I> // I models ForwardIterator
typename vector<T, A>::iterator vector<T, A>::move_insert(iterator p, I f, I l)
{
    size_type n(std::distance(f, l));
    iterator  last = end();
    size_type before = p - begin();

    if (remaining() < n) {
        vector tmp;
        tmp.reserve((adobe::max)(size() + n, 2 * size()));
        tmp.move_append(begin(), p);
        tmp.move_append(f, l);
        tmp.move_append(p, last);
        swap(tmp);
    } else {
        size_type after(last - p);
        
        if (n < after) {
            move_append(last - n, last);
            ::adobe::move_backward(p, last - n, last);
            ::adobe::move(f, l, p);
        } else {
            I m = f;
            std::advance(m, after);
            move_append(m, l);
            move_append(p, last);
            ::adobe::move(f, m, p);
        }
    }
    return begin() + before + n;
}

template <typename T, typename A>
void vector<T, A>::reserve(size_type n)
{
    if (capacity() < n) {
        vector tmp;
        tmp.header_m = allocate(get_allocator(), n);
        tmp.header_m->finish() = ::adobe::uninitialized_move(begin(), end(), tmp.end());
        swap(tmp);
    }
}

template <typename T, typename A>
typename vector<T, A>::iterator vector<T, A>::insert(iterator p, size_type n, const T& x)
{
    iterator  last = end();
    size_type before = p - begin();

    if (remaining() < n) {
        vector tmp;
        tmp.reserve((adobe::max)(size() + n, 2 * size()));
        tmp.move_append(begin(), p);
        std::uninitialized_fill_n(tmp.end(), n, x);
        tmp.set_finish(tmp.end() + n);
        tmp.move_append(p, last);
        swap(tmp);
    } else {
        size_type after(last - p);
        
        if (n < after) {
            move_append(last - n, last);
            ::adobe::move_backward(p, last - n, last);
            std::fill_n(p, n, x);
        } else {
            std::uninitialized_fill_n(last, n - after, x);
            set_finish(last + (n - after));
            move_append(p, last);
            std::fill_n(p, after, x);
        }
    }
    return begin() + before + n;
}
    
template <typename T, typename A>
typename vector<T, A>::iterator vector<T, A>::erase(iterator f, iterator l)
{
    iterator i = ::adobe::move(l, end(), f);
    for (iterator b(i), e(end()); b != e; ++b) {
        b->~value_type();
    }
    set_finish(i);
    return f;
}
    
template <typename T, typename A>
void vector<T, A>::resize(size_type n)
{
    if (n < size()) erase(begin() + n, end());
    else insert(end(), n - size(), value_type());
}
    
template <typename T, typename A>
void vector<T, A>::resize(size_type n, const value_type& x)
{
    if (n < size()) erase(begin() + n, end());
    else insert(end(), n - size(), x);
}

/*************************************************************************************************/

#ifdef ADOBE_STD_SERIALIZATION

template <typename T, typename A>
std::ostream& operator<<(std::ostream& out, const vector<T, A>& x)
{
    out << begin_sequence;
    
    for (typename vector<T, A>::const_iterator first(x.begin()), last(x.end()); first != last; ++first)
    {
        out << format(*first);
    }
    
    out << end_sequence;
    
    return out;
}

#endif

/*************************************************************************************************/

BOOST_STATIC_ASSERT(sizeof(vector<int>) == sizeof(void*));

/*************************************************************************************************/

} // namespace version_1
} // namespace adobe

/*************************************************************************************************/

ADOBE_NAME_TYPE_1("vector:version_1:adobe", adobe::version_1::vector<T0,
        adobe::capture_allocator<T0> >)
ADOBE_NAME_TYPE_2("vector:version_1:adobe", adobe::version_1::vector<T0, T1>)

/*************************************************************************************************/

namespace boost {

template <typename T, typename A>
struct has_nothrow_constructor<adobe::version_1::vector<T, A> > : boost::mpl::true_ { };

} // namespace boost

/*!
@}
*/
/*************************************************************************************************/

#endif
