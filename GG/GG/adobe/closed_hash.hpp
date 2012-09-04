/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_CLOSED_HASH_HPP
#define ADOBE_CLOSED_HASH_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/closed_hash_fwd.hpp>

#include <climits>
#include <cstddef>
#include <limits>

#include <boost/compressed_pair.hpp>
#include <boost/functional/hash.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/has_nothrow_constructor.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/operators.hpp>
#include <boost/next_prior.hpp>

#include <GG/adobe/algorithm/lower_bound.hpp>
#include <GG/adobe/conversion.hpp>
#include <GG/adobe/cstdint.hpp>
#include <GG/adobe/empty.hpp>
#include <GG/adobe/functional.hpp>
#include <GG/adobe/iterator/set_next.hpp>
#include <GG/adobe/memory.hpp>
#include <GG/adobe/move.hpp>
#include <GG/adobe/utility.hpp>

#include <GG/adobe/implementation/swap.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

template <typename T, typename V> // V is value_type(T) const qualified
class closed_hash_iterator : public boost::iterator_facade<closed_hash_iterator<T, V>, V,
                                                           std::bidirectional_iterator_tag>
{
    typedef boost::iterator_facade<closed_hash_iterator<T, V>, V,
                                   std::bidirectional_iterator_tag> inherited_t;

    typedef typename T::node_t node_t;
 public:
    typedef typename inherited_t::reference         reference;
    typedef typename inherited_t::difference_type   difference_type;
    typedef typename inherited_t::value_type        value_type;

    closed_hash_iterator() : node_m(0) { }

    template <typename O>
    closed_hash_iterator(const closed_hash_iterator<T, O>& x) : node_m(x.node_m) { }

 public:
    /*
        REVISIT (sparent@adobe.com) : node_m should be private but
        "gcc version 4.0.1 (Apple Inc. build 5465)" doesn't like it.
    */

    node_t* node_m;

 private:
    
    reference dereference() const { return node_m->value_m; }
    void increment() { node_m = node_m->next(); }           
    void decrement() { node_m = node_m->prior(); }

    template< typename O>
    bool equal(const closed_hash_iterator<T, O>& y) const { return node_m == y.node_m; }
    
    std::size_t state() const { return node_m->state(); }
    void set_state(std::size_t x) { return node_m->set_state(x); }

    explicit closed_hash_iterator(node_t* node) : node_m(node) { }
    
        friend class version_1::closed_hash_set<value_type, typename T::key_transform, typename T::hasher,
            typename T::key_equal, typename T::allocator_type>;
    friend class boost::iterator_core_access;
    friend struct unsafe::set_next_fn<closed_hash_iterator>;
};

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

namespace unsafe {

template <typename T, typename V>
struct set_next_fn<implementation::closed_hash_iterator<T, V> >
{
    typedef typename implementation::closed_hash_iterator<T, V> iterator;

    void operator()(iterator x, iterator y) const
    { set_next(*x.node_m, *y.node_m); }
};

} // namespace unsafe

/*************************************************************************************************/

#ifndef ADOBE_NO_DOCUMENTATION

namespace version_1 {

#endif

/*************************************************************************************************/

/*!
\defgroup abi_container ABI-Safe Containers: hash containers, vector, ...
\ingroup abi_safe container
 */

/*!
\ingroup abi_container

\brief A hash based associative container.

\par
A \c closed_hash_set is a hash based associative container, similar to a hash_set.


\model_of
    - \ref concept_regular_type
    - \ref stldoc_UniqueHashedAssociativeContainer

\todo
    - re-order parameters so key_function is after comparison - to be consistent with lower_bound.

*/

template<   typename T,
            typename KeyTransform,
            typename Hash,
            typename Pred,
            typename A>
class closed_hash_set : boost::equality_comparable<closed_hash_set<T, KeyTransform, Hash, Pred, A>,
                                        closed_hash_set<T, KeyTransform, Hash, Pred, A>,
                                        empty_base<closed_hash_set<T, KeyTransform, Hash, Pred, A> > >
{
 public:
    typedef KeyTransform                        key_transform;

    typedef typename boost::remove_reference<typename key_transform::result_type>::type
                                                key_type;

    typedef T                                   value_type;
    typedef Hash                                hasher;
    typedef Pred                                key_equal;
    typedef A                                   allocator_type;
    typedef value_type*                         pointer;
    typedef const value_type*                   const_pointer;
    typedef value_type&                         reference;
    typedef const value_type&                   const_reference;
    typedef std::size_t                         size_type;
    typedef std::ptrdiff_t                      difference_type;

    friend class implementation::closed_hash_iterator<closed_hash_set, value_type>;
    friend class implementation::closed_hash_iterator<closed_hash_set, const value_type>;
    
    typedef implementation::closed_hash_iterator<closed_hash_set, value_type>       iterator;
    typedef implementation::closed_hash_iterator<closed_hash_set, const value_type> const_iterator;

    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

 private:
    enum
    {
        state_free          = 0,
        state_home          = 1,
        state_misplaced     = 2
    };
    
    template <typename U> // U is derived node
    struct list_node_base
    {
        list_node_base() { next_m = static_cast<U*>(this); prior_m = static_cast<U*>(this); }
        
        U* address() { return static_cast<U*>(this); }
        const U* address() const { return static_cast<const U*>(this); }
        
        operator U& () { return *static_cast<U*>(this); }
        operator const U& () const { return *static_cast<const U*>(this); }
        
        friend inline void set_next(U& x, U& y)
        { x.next_m = reinterpret_cast<U*>(uintptr_t(&y) | uintptr_t(x.state())); y.prior_m = &x; }
            
        friend inline void set_next_raw(U& x, U& y)
        { x.next_m = &y; y.prior_m = &x; }
        
        std::size_t state() const { return std::size_t(uintptr_t(next_m) & uintptr_t(0x03UL)); }
        void set_state(std::size_t x)
        {
            assert(x < 0x04UL);
            next_m = reinterpret_cast<U*>(uintptr_t(next()) | uintptr_t(x));
        }
        
        U* next() const { return reinterpret_cast<U*>(reinterpret_cast<uintptr_t>(next_m) & ~uintptr_t(0x03UL)); }
        U* prior() const { return prior_m; }

     private:
        U* next_m;
        U* prior_m;
    };
    
    struct node_t : list_node_base<node_t>
    {
        T           value_m;
    };
    
    typedef list_node_base<node_t> node_base_t;
    
    struct header_t
    {
        struct compact_header_t
        {
            boost::compressed_pair<allocator_type, node_base_t> alloc_free_tail_m;
            node_base_t used_tail_m;
            std::size_t capacity_m;
            std::size_t size_m;
        };
        
        /*
        NOTE (sparent) - the assumption is that the initial items are pointers and that size_t is
        either equal to the sizeof a pointer or a lower power of two so this packs tightly.
        */
        
        BOOST_STATIC_ASSERT(!(sizeof(A) == sizeof(void*) || sizeof(A) == 0)
            || (sizeof(compact_header_t) == (sizeof(allocator_type) + 2 * sizeof(node_base_t) + 2 *
                sizeof(std::size_t))));
        
        aligned_storage<compact_header_t> header_m;
        node_t      storage_m[1];
        
        allocator_type&  allocator() { return header_m.get().alloc_free_tail_m.first(); }
        const allocator_type& allocator() const { return header_m.get().alloc_free_tail_m.first(); }
        node_base_t&  free_tail() { return header_m.get().alloc_free_tail_m.second(); }
        const node_base_t& free_tail() const { return header_m.get().alloc_free_tail_m.second(); }
        node_base_t&  used_tail() { return header_m.get().used_tail_m; }
        const node_base_t& used_tail() const { return header_m.get().used_tail_m; }
        std::size_t&  capacity() { return header_m.get().capacity_m; }
        const std::size_t& capacity() const { return header_m.get().capacity_m; }
        std::size_t&  size() { return header_m.get().size_m; }
        const std::size_t& size() const { return header_m.get().size_m; }
    };
    
    typedef node_t* node_ptr;

    typedef boost::compressed_pair< hasher,
                                    boost::compressed_pair< key_equal,
                                                            boost::compressed_pair< key_transform,
                                                                                                                                                                        header_t*
                                                                                  >
                                                          >
                                  > data_t;

    data_t  data_m;

    typedef header_t* header_pointer;
   
    const header_pointer& header() const { return data_m.second().second().second(); }
    header_pointer& header() { return data_m.second().second().second(); }
    
 public:
    // construct/destroy/copy

    closed_hash_set() { header() = 0; }
    
    explicit closed_hash_set(size_type n)
    {
        header() = 0;
        allocate(allocator_type(), n);
    }
    
    closed_hash_set(size_type n, const hasher& hf, const key_equal& eq = key_equal(),
                                                   const key_transform& kf = key_transform(),
                                                   const allocator_type& a = allocator_type())
    {
        header() = 0;
        data_m.first() = hf;
        data_m.second().first() = eq;
        data_m.second().second().first() = kf;
        allocate(a, n);
    }

    template <typename I> // I models InputIterator
    closed_hash_set(I f, I l) { header() = 0; insert(f, l); }

    template <typename I> // I models InputIterator
    closed_hash_set(I f, I l, size_type n, const hasher& hf = hasher(),
                                           const key_equal& eq = key_equal(),
                                           const key_transform& kf = key_transform(),
                                           const allocator_type& a = allocator_type())
    {
        header() = 0;
        data_m.first() = hf;
        data_m.second().first() = eq;
        data_m.second().second().first() = kf;
        allocate(a, n);
        insert(f, l);
    }
    
    closed_hash_set(const closed_hash_set& x) : data_m(x.data_m)
    {
        header() = 0;
        allocate(x.get_allocator(), x.size());
        insert(x.begin(), x.end());
    }
    closed_hash_set& operator=(closed_hash_set x) { swap(x, *this); return *this; }

    allocator_type get_allocator() const
    { return header() ? header()->allocator() : allocator_type(); }

    closed_hash_set(move_from<closed_hash_set> x) : data_m(x.source.data_m) { x.source.header() = 0; }

#if 0
    template <typename I> // I models ForwardIterator
    closed_hash_set(I f, I l, move_ctor) { header() = 0; move_insert(f, l); }
#endif

    // size and capacity

    size_type size() const { return header() ? header()->size() : 0; }
    size_type max_size() const { return size_type(-1) / sizeof(node_t); }
    bool empty() const { return size() == 0; }
    size_type capacity() const { return header() ? header()->capacity() : 0; }

    void reserve(size_type n)
    {
        if (n <= capacity()) return;
        
        if (!header()) allocate(allocator_type(), n);
        else
        {
            closed_hash_set tmp(n, hash_function(), key_eq(), key_function(), get_allocator());
            tmp.move_insert(begin(), end());
            swap(*this, tmp);
        }
    }
    
    key_transform key_function() const { return data_m.second().second().first(); }
    hasher hash_function() const { return data_m.first(); }
    key_equal key_eq() const { return data_m.second().first(); }
    
    iterator begin() { return iterator(header() ? header()->used_tail().next() : 0); }
    iterator end() { return iterator(header() ? header()->used_tail().address() : 0); }
    
    const_iterator begin() const { return iterator(header() ? header()->used_tail().next() : 0); }
    const_iterator end() const { return iterator(header() ? const_cast<node_t*>(header()->used_tail().address()) : 0); }
    
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    
    iterator erase(iterator location)
    {
        iterator next(boost::next(location));
        iterator result = next;
    
        if ((location.state() == std::size_t(state_home)) && (next != end())
                && (next.state() == std::size_t(state_misplaced)))
        {
            swap(*next, *location);
            result = location;
            location = next;
        }
    
        unsafe::skip_node(location);
        erase_raw(location);
        
        --header()->size();
        
        return result;
    }
    
    std::size_t erase(const key_type& key)
    {
        iterator node(find(key));
        if (node == end()) return 0;
        erase(node);
        return 1;
    }
    
    void clear()
    {
        for(iterator first(begin()), last(end()); first != last; first = erase(first)) ;
    }
    
    const_iterator find(const key_type& key) const
    {
        return adobe::remove_const(*this).find(key);
    }
    
    iterator find(const key_type& key)
    {
        if (empty()) return end();
        
        iterator node(bucket(key));
        
        if (node.state() != std::size_t(state_home)) return end();
        
        return find(node, key);
    }
    
    std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        const_iterator result = find(key);
        if (result == end()) return std::make_pair(result, result);
        return std::make_pair(result, boost::next(result));
    }
    
    std::pair<iterator, iterator> equal_range(const key_type& key)
    {
        iterator result = find(key);
        if (result == end()) return std::make_pair(result, result);
        return std::make_pair(result, boost::next(result));
    }
    
    std::size_t count(const key_type& key) const
    { return std::size_t(find(key) != end()); }
    
    template <typename I> // I models InputIterator
    void insert(I first, I last)
    { while (first != last) { insert(*first); ++first; } }
    
    template <typename I> // I models ForwardIterator
    void move_insert(I first, I last)
    { while (first != last) { insert(::adobe::move(*first)); ++first; } }
    
    /*
        NOTE (sparent): If there is not enough space for one element we will reserve the space
        prior to attempting the insert even if the item is already in the hash table. Without
        recalculating the bucket (a potentially expensive operation) there is no other solution.
    */
    
    template <typename U>
    std::pair<iterator, bool> insert(const U& x, typename copy_sink<U, value_type>::type = 0)
    {
        if (capacity() == size()) {
            value_type tmp(x); // Make a copy incase resize moves the element.
            reserve(size() ? 2 * size() : 3);
            return unsafe_copy_insert(tmp);
        }
        return unsafe_copy_insert(x);
    }
    
    template <typename U>
    std::pair<iterator, bool> insert(U x, typename move_sink<U, value_type>::type = 0)
    {
        if (capacity() == size()) reserve(size() ? 2 * size() : 3);
        
        iterator node = bucket(key_function()(x));
        
        switch (node.state())
        {
        case state_home:
            {
            iterator found = find(node, key_function()(x));
            if (found != end()) {
                *found = ::adobe::move(x);
                return std::make_pair(found, false);
            }
            
            iterator free(begin_free());
            insert_raw(free, ::adobe::move(x), state_misplaced);
            unsafe::splice_node_range(node, free, free);
            node = free;
            }
            break;
        case state_misplaced:
            {
            iterator free(begin_free());
            insert_raw(free, ::adobe::move(*node), state_misplaced);
            
            unsafe::set_next(boost::prior(node), free);
            unsafe::set_next(free, boost::next(node));
            
            erase_raw(node);
            }
            // fall through
        default: // state_free
            {
            insert_raw(node, ::adobe::move(x), state_home);
            unsafe::splice_node_range(end(), node, node);
            }
        }
        header()->size() += 1;
        return std::make_pair(node, true);
    }
    
    template <typename U>
    iterator insert(iterator, const U& x, typename copy_sink<U, value_type>::type = 0)
    {
        return insert(x).first;
    }
    
    template <typename U>
    iterator insert(iterator, U x, typename move_sink<U, value_type>::type = 0)
    {
        return insert(::adobe::move(x)).first;
    }
    
    ~closed_hash_set()
    {
        if (header())
        {
            for(iterator first(begin()), last(end()); first != last; ++first) destroy(&*first);
            raw_allocator alloc(get_allocator());
            alloc.deallocate(reinterpret_cast<char*>(header()), 0);
        }
    }
        
    friend void swap(closed_hash_set& x, closed_hash_set& y)
    {
        std::swap(x.data_m, y.data_m);
    }
    
    friend bool operator==(const closed_hash_set& x, const closed_hash_set& y)
    {
        if (x.size() != y.size()) return false;
        for (const_iterator first(x.begin()), last(x.end()); first != last; ++first)
        {
            const_iterator iter(y.find(y.key_function()(*first)));
            if (iter == y.end() || !(*first == *iter)) return false;
        }
        return true;
    }
 private:
 
    typedef typename allocator_type::template rebind<char>::other raw_allocator;


    void allocate(const allocator_type& a, size_type n)
    {
        // table of primes such that p[n + 1] = next_prime(2 * p[n])
    
        static const std::size_t prime_table[] = { 3UL, 7UL, 17UL, 37UL, 79UL, 163UL, 331UL, 673UL,
            1361UL, 2729UL, 5471UL, 10949UL, 21911UL, 43853UL, 87719UL, 175447UL, 350899UL,
            701819UL, 1403641UL, 2807303UL, 5614657UL, 11229331UL, 22458671UL, 44917381UL,
            89834777UL, 179669557UL, 359339171UL, 718678369UL, 1437356741UL, 2874713497UL,
            ULONG_MAX
        };

        assert(!header() && "WARNING (sparent@adobe.com) : About to write over allocated header.");

        if (n == 0 && a == allocator_type()) return;

        n = *adobe::lower_bound(prime_table, n);
            
        raw_allocator alloc(a);
    
        header() = reinterpret_cast<header_t*>(alloc.allocate(sizeof(header_t) - sizeof(node_t)
            + sizeof(node_t) * n));
        header()->capacity() = n;
        header()->size() = 0;
        construct(&header()->free_tail());
        construct(&header()->used_tail());
        construct(&header()->allocator(), a);
        
        node_t* prior = header()->free_tail().address();
        for (node_ptr first(&header()->storage_m[0]), last(&header()->storage_m[0]+ n);
                first != last; ++first)
        {
            set_next_raw(*prior, *first);
            prior = first;
            // first->set_state(state_free);
        }
        set_next_raw(*prior, header()->free_tail());

    }
 
    iterator bucket(const key_type& key)
    {
        std::size_t slot(hash_function()(key) % capacity());
        return iterator(&header()->storage_m[0] + slot);
    }
 
    iterator find(iterator node, const key_type& key)
    {
        do
        {
            if (key_eq()(key, key_function()(*node))) return node;
            ++node;
        } while ((node != end()) && (node.state() != std::size_t(state_home)));
        
        return end();
    }
 
    // location points to a free node
    template <typename U>
    static void insert_raw(iterator location, const U& x, std::size_t state,
        typename copy_sink<U, value_type>::type = 0)
    {
        ::new (&(*location)) value_type(x);
        location.set_state(state);
        unsafe::skip_node(location);
    }
 
    // location points to a free node
    template <typename U>
    static void insert_raw(iterator location, U x, std::size_t state,
        typename move_sink<U, value_type>::type = 0)
    {
        move_construct<value_type>(&*location, x);
        location.set_state(state);
        unsafe::skip_node(location);
    }
    
    // location points to a used but detatched node
    void erase_raw(iterator location)
    {
        destroy(&*location);
        location.set_state(state_free);
        unsafe::splice_node_range(end_free(), location, location);
    }
 
    iterator begin_free() { return iterator(header() ? header()->free_tail().next() : 0); }
    iterator end_free() { return iterator(header() ? header()->free_tail().address() : 0); }


    std::pair<iterator, bool> unsafe_copy_insert(const value_type& x)
    {
        // pre-condition is that there is capacity for the element.
        iterator node = bucket(key_function()(x));
        
        switch (node.state())
        {
        case state_home:
            {
            iterator found = find(node, key_function()(x));
            if (found != end()) {
                *found = x;
                return std::make_pair(found, false);
            }
            
            iterator free(begin_free());
            insert_raw(free, x, state_misplaced);
            unsafe::splice_node_range(node, free, free);
            node = free;
            }
            break;
        case state_misplaced:
            {
            iterator free(begin_free());
            insert_raw(free, *node, state_misplaced);
            
            unsafe::set_next(boost::prior(node), free);
            unsafe::set_next(free, boost::next(node));
            
            erase_raw(node);
            }
            // fall through
        default: // state_free
            {
            insert_raw(node, x, state_home);
            unsafe::splice_node_range(end(), node, node);
            }
        }
        header()->size() += 1;
        return std::make_pair(node, true);
    }

};

/*************************************************************************************************/

/*!

\ingroup abi_container

\brief A hash based associative container.

\par
A \c closed_hash_map is a hash based associative container, it is an adapted \c closed_hash_set
where value_type is \c adobe::pair<Key, T> and the KeyTransform returns the first element of the
pair.

\model_of
    - \ref concept_regular_type
    - \ref stldoc_UniqueHashedAssociativeContainer


*/

template<typename Key,
         typename T,
         typename Hash,
         typename Pred,
         typename A>
class closed_hash_map : public closed_hash_set<pair<Key, T>,
                                               get_element<0, pair<Key, T> >,
                                               Hash,
                                               Pred,
                                               A>
{
    typedef closed_hash_set<pair<Key, T>,
                            get_element<0, pair<Key, T> >,
                            Hash,
                            Pred,
                            A> set_type;
 public:
    typedef T mapped_type;
 
    closed_hash_map() { }
    
    template <typename I> // I models InputIterator
    closed_hash_map(I f, I l) : set_type(f, l) { }
    
#if 0
    template <typename I> // I models ForwardIterator
    closed_hash_map(I f, I l, move_ctor) : set_type(f, l, move_ctor()) { }
#endif
        
    closed_hash_map(const closed_hash_map& x) : set_type(x) { }
    closed_hash_map(move_from<closed_hash_map> x) : set_type(move_from<set_type>(x.source)) { }
    closed_hash_map& operator=(closed_hash_map x)
    { swap(x, *this); return *this; }
            
    friend void swap(closed_hash_map& x, closed_hash_map& y)
    { swap(static_cast<set_type&>(x), static_cast<set_type&>(y)); }
    
    
    friend bool operator==(const closed_hash_map& x, const closed_hash_map& y)
    { return static_cast<const set_type&>(x) == static_cast<const set_type&>(y); }
    
    /*
        NOTE (sparent) : Can't use boost::equality_comparable without introducing extra base class
        overhead.
    */
    
    friend bool operator!=(const closed_hash_map& x, const closed_hash_map& y)
    { return !(x == y); }

#ifndef ADOBE_CLOSED_HASH_MAP_INDEX
#define ADOBE_CLOSED_HASH_MAP_INDEX 1
#endif

#if ADOBE_CLOSED_HASH_MAP_INDEX
        
    mapped_type& operator[](const Key& x)
    {
        typename set_type::iterator i = this->find(x);
        if (i == this->end()) return this->insert(adobe::make_pair(x, mapped_type())).first->second;
        return i->second;
    }
    
#endif
};

/*************************************************************************************************/

BOOST_STATIC_ASSERT(sizeof(closed_hash_set<int>) == sizeof(void*));


#ifndef ADOBE_NO_DOCUMENTATION

} // namespace version_1

#endif

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

ADOBE_NAME_TYPE_1("closed_hash_set:version_1:adobe",
    adobe::version_1::closed_hash_set<T0, adobe::identity<const T0>, boost::hash<T0>, std::equal_to<T0>,
        adobe::capture_allocator<T0> >);
ADOBE_NAME_TYPE_2("closed_hash_map:version_1:adobe",
    adobe::version_1::closed_hash_map<T0, T1, boost::hash<T0>, std::equal_to<T0>,
            adobe::capture_allocator<adobe::pair<T0, T1> > >);

ADOBE_NAME_TYPE_5("closed_hash_set:version_1:adobe",
    adobe::version_1::closed_hash_set<T0, T1, T2, T3, T4 >);
ADOBE_NAME_TYPE_5("closed_hash_map:version_1:adobe",
    adobe::version_1::closed_hash_map<T0, T1, T2, T3, T4 >);

/*************************************************************************************************/

namespace boost {

template<   typename T,
            typename KeyTransform,
            typename Hash,
            typename Pred,
            typename A>
struct has_nothrow_constructor<adobe::version_1::closed_hash_set<T, KeyTransform, Hash, Pred, A> >
        : boost::mpl::true_ { };

template<typename Key,
         typename T,
         typename Hash,
         typename Pred,
         typename A>
struct has_nothrow_constructor<adobe::version_1::closed_hash_map<Key, T, Hash, Pred, A> >
    : boost::mpl::true_ { };

} // namespace boost

/*************************************************************************************************/

#endif

/*************************************************************************************************/
