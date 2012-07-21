/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_STRING_HPP
#define ADOBE_STRING_HPP

#include <GG/adobe/config.hpp>

#include <cstring>
#include <functional>
#include <iterator>
#include <string>

#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/static_assert.hpp>
#include <boost/utility.hpp>

#include <GG/adobe/cstring.hpp>
#include <GG/adobe/string_fwd.hpp>
#include <GG/adobe/typeinfo.hpp>
#include <GG/adobe/vector.hpp>

/*************************************************************************************************/

namespace std {

/*************************************************************************************************/

/*! \defgroup string_algorithm String Algorithms
\ingroup container_algorithm
These operators were moved to namespace \c std because string is in this namespace.
Apparently sometimes CW 9.1 cannot find the correct operator without using ADL.
@{
*/

//!\ingroup string_algorithm
template<typename CharT, class Traits, class Allocator>
typename std::basic_string<CharT, Traits, Allocator>&
        operator << (   std::basic_string<CharT, Traits, Allocator>& out,
                        const std::basic_string<CharT, Traits, Allocator>& in)
{
    typename std::basic_string<CharT, Traits, Allocator>::size_type required(in.size() + out.size());

    if (required > out.capacity()) out.reserve((std::max)(out.capacity() * 2, required));

    out += in;
    return out;
}

/*************************************************************************************************/

//!\ingroup string_algorithm
template<typename CharT, class Traits, class Allocator>
typename std::basic_string<CharT, Traits, Allocator>& 
        operator << (std::basic_string<CharT, Traits, Allocator>& out_str, const CharT* in_str)
{
    typename std::basic_string<CharT, Traits, Allocator>::size_type required(std::strlen(in_str) + out_str.size());

    if (required > out_str.capacity()) out_str.reserve((std::max)(out_str.capacity() * 2, required));

    out_str += in_str;
    return out_str;
}

//@}

/*************************************************************************************************/

} // namespace std

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*!
\fn typename std::basic_string<CharT, Traits, Allocator>& std::operator<<(std::basic_string<CharT, Traits, Allocator>& out, const std::basic_string<CharT, Traits, Allocator>& in)
\ingroup string_algorithm


Enables \c operator<< for appending one string to another.

\param out string to be appended
\param in string to append to \c out

\return
    \c out with \c in appended to it.

\note
    To guarantee amortized constant time growth on the string we double the size of the capacity if growing is necessary.
*/

/*!
\fn typename std::basic_string<CharT, Traits, Allocator>& std::operator<<(std::basic_string<CharT, Traits, Allocator>& out_str, const CharT* in_str)
\ingroup string_algorithm

Enables \c operator<< for appending an NTBS to a string.

\param out_str string to be appended
\param in_str NTBS to append to \c out

\return
    \c out_str with \c in_str appended to it.

\note
    To guarantee amortized constant time growth on the string we double the size of the capacity if growing is necessary.
*/


/*! \addtogroup string_algorithm
@{
*/

inline std::string make_string(const char* a, const char * b)
{
    std::string result;
    result.reserve(std::strlen(a) + std::strlen(b));
    result += a;
    result += b;
    return result;
}

/*************************************************************************************************/

inline std::string make_string(const char* a, const char * b, const char* c)
{
    std::string result;
    result.reserve(std::strlen(a) + std::strlen(b) + std::strlen(b));
    result += a;
    result += b;
    result += c;
    return result;
}

//!@}

/*************************************************************************************************/

//!\ingroup misc_functional
struct str_less_t : std::binary_function<const char*, const char*, bool>
{
    bool operator()(const char* x, const char* y) const
    { return adobe::strcmp(x, y) < 0; }
};



/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

namespace adobe {
namespace version_1 {

/*! \defgroup abi_string ABI-safe Strings
\ingroup abi_safe
*/

/*************************************************************************************************/

// Move this to some appropriate file.
//!\ingroup abi_misc
template <typename Derived>
class empty_base_t { }; // Empty base to reduce size of adobe::string
        
/*************************************************************************************************/

/*!
\class adobe::string_t

\ingroup abi_string

\brief Lightweight string class designed to hold UTF8 strings in fixed binary structure.
 
The structure of a string_t is a simple adobe::vector_t<char> (which is, itself a fixed binary
structure). If the string is empty, the vector is empty. If the vector is not empty, it contains
a null-terminated sequence, the UTF8 string.
*/
class string_t : boost::totally_ordered<string_t, string_t, empty_base_t<string_t> >
{
 public:
    typedef char                               value_type;
    typedef char*                              pointer;
    typedef const char*                        const_pointer;
    typedef char&                              reference;
    typedef const char&                        const_reference;
    typedef std::size_t                        size_type;
    typedef std::ptrdiff_t                     difference_type;
    typedef char*                              iterator;
    typedef const char*                        const_iterator;
    typedef std::reverse_iterator<char*>       reverse_iterator;
    typedef std::reverse_iterator<const char*> const_reverse_iterator;

 private:
        typedef vector<value_type>                                 storage_type;
        
        storage_type storage_m;

        /*
                NOTE (eberdahl@adobe.com): Because c_str is required to return a null-terminated sequence,
                we ensure that the storage vector is always null-terminated. This means that the storage
                is always either empty or contains one extra character to hold the null-character.
        */
        template <typename ForwardIterator>
        void assign(ForwardIterator first, ForwardIterator last, std::forward_iterator_tag)
        {
                storage_type tmp;
                if (first != last)
                {
                        const size_type len(std::distance(first, last));
                        tmp.reserve(len + 1);
                        tmp.insert(tmp.end(), first, last);
                        tmp.push_back(char(0));
                }
                storage_m.swap(tmp);
        }

        template <typename InputIterator>
        void assign(InputIterator first, InputIterator last, std::input_iterator_tag)
        {
                storage_type tmp;
                if (first != last)
                {
                        tmp.insert(tmp.end(), first, last);
                        tmp.push_back(char(0));
                }
                storage_m.swap(tmp);
        }

        template <typename Iterator>
        void assign(Iterator first, Iterator last)
        {
                assign(first, last,
                           typename std::iterator_traits<Iterator>::iterator_category());
        }
        
 public:
        /*!
                Constructs an empty string.
        */
        string_t() { }
        
        /*!
                Copy constructor.
         */
        string_t(const string_t& s) : storage_m(s.storage_m) { }
    
    /*!
        Move constructor.
    */
    string_t(move_from<string_t> x) : storage_m(::adobe::move(x.source.storage_m)) { }

        /*!
                Constructs a string_t from a regular C string (including string literals).

                \param[in] s null-terminated string from which to construct the string_t
         */
        string_t(const char* s);

        /*!
                Constructs a string from a char sequence.

                \param[in] s character sequence from which to construct the string_t
                \param[in] length number of elements from s from which to construct the string_t
         */
        string_t(const char* s, std::size_t length);
        
        /*!
                Constructs a string_t from a generic sequence.
         
                \param[in] first start of sequence from which to construct the string_t
                \param[in] last end of sequence from which to construct the string_t
         */
        template <typename Iterator>
        string_t(Iterator first, Iterator last)
                { assign(first, last); }

        /*!
                Constructs a string_t from a std::string.
         
                \param[in] s std::string from which to construct the string_t
         */
        string_t(const std::string& s);
        
        /*!
                Destructor.
         */
        ~string_t() { }

        /*!
                Assignment operator.
         */

        string_t& operator=(string_t s) { storage_m = ::adobe::move(s.storage_m); return *this; }

        /*!
                Conversion operator to std::string.
         */
        /*
                NOTE (eberdahl@adobe.com): This function was created to support extracting std::string
                from any_regular_t (N.B. any_regular_t stores strings as string_t). Using conversion
                member functions may not be the best way to support the required conversion. Until
                any_regular_t changes the way it extracts data, we'll need this function.
         */
        operator std::string() const
                { return std::string(begin(), end()); }
        
        /*!
                Returns const pointer to a regular C string, identical to the string_t. The returned
                string is null-terminated.
         */
        const char* c_str() const
                { return empty() ? "" : &storage_m[0]; }

        /*!
                Appends one element to the end of the string.
         */
        void push_back(value_type c);

        /*!
                Appends a character sequence to the end of the string
         */
        template <typename Iterator>
        void append(Iterator first, Iterator last)
        {
                if (first != last)
                {
                        if (!storage_m.empty())
                                storage_m.pop_back();
                        
                        storage_m.insert(storage_m.end(), first, last);
                        storage_m.push_back(0);
                }
        }

        /*!
                Appends another string this one
         */
        void append(const string_t& s)
                { append(s.begin(), s.end()); }

        /*!
                Appends null-terminated char sequence to the string
         */
        void append(const char* s)
                { append(s, s + std::strlen(s)); }

        /*!
                Appends a number of characters from a sequence to the string
         */
        void append(const char* s, std::size_t length)
                { append(s, s + length); }

        /*!
                Appends the contents of a std::string to the string
         */
        void append(const std::string& s)
                { append(s.begin(), s.end()); }

        /*!
                Appends another string this one
         */
        string_t& operator+=(const string_t& s)
                { append(s); return *this; }
        
        /*!
                Appends null-terminated char sequence to the string
         */
        string_t& operator+=(const char* s)
                { append(s); return *this; }
        
        /*!
                Appends the contents of a std::string to the string
         */
        string_t& operator+=(const std::string& s)
                { append(s); return *this; }
        
        /*!
                Returns an iterator to the first element of the string.
         */
        const_iterator begin() const
                { return storage_m.begin(); }

        /*!
                Returns an iterator just past the end of the string.
         */
        const_iterator end() const
                { return storage_m.empty() ? storage_m.end() : boost::prior(storage_m.end()); }

        /*!
                Returns a reverse_iterator to the end of the current string.
         */
        const_reverse_iterator rbegin() const
                { return const_reverse_iterator(end()); }

        /*!
                Returns a reverse_iterator to the beginning of the current string.
         */
        const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

        /*!
                Returns the size of the allocated storage space for the
                elements of the string.
         */
        size_type capacity() const;
        
        /*!
                Requests that the capacity of the allocated storage space for
                the elements of the string be at least enough to hold n
                elements.
         */
        void reserve(size_type n)
                { storage_m.reserve(0 == n ? 0 : n + 1); }

        /*!
                Changes the string to be zero-length.
         */
        void clear()
                { storage_m.clear(); }
        
        /*!
                Returns the number of elements in the current string.
         */
    size_type size() const
                { return storage_m.empty() ? 0 : storage_m.size() - 1; }

        /*!
                Returns true if the string has no elements, false otherwise.
         */
    bool empty() const
                { return storage_m.empty(); }

        /*!
                Exchanges the elements of the current string with those of from s.
         */
        void swap(string_t& s)
                { storage_m.swap(s.storage_m); }
        
        friend bool operator==(const string_t& x, const string_t& y)
        {
                return x.storage_m == y.storage_m;
        }

        friend bool operator<(const string_t& x, const string_t& y)
        {
                return x.storage_m < y.storage_m;
        }
    
    friend inline void swap(string_t& x, string_t& y)
                { x.storage_m.swap(y.storage_m); }
};

/*!
        Concatenate an entity to a string.
        Anything that can be concatenated using string_t::operator+=
        may also be concatenated to a string_t using operator+.
 */
inline string_t operator+(string_t s1, const string_t& s2)     { return ::adobe::move(s1 += s2); }
inline string_t operator+(string_t s1, const std::string& s2)  { return ::adobe::move(s1 += s2); }
inline string_t operator+(string_t s1, const char* s2)         { return ::adobe::move(s1 += s2); }

/*************************************************************************************************/

#if defined(ADOBE_STD_SERIALIZATION)
inline std::ostream& operator << (std::ostream& os, const string_t& t)
{ return os << t.c_str(); }
#endif

/*************************************************************************************************/

/*!
\class adobe::string16_t 
\ingroup abi_string
 
\brief Lightweight string class designed to hold UTF16 strings in fixed binary structure.
 
The structure of a string16_t is a simple adobe::vector_t<boost::uint16_t> (which is, itself a
fixed binary structure). If the string is empty, the vector is empty. If the vector is not empty,
it contains a null-terminated sequence, the UTF16 string.
 */
class string16_t : boost::totally_ordered<string16_t, string16_t, empty_base_t<string16_t> >
{
public:
    typedef boost::uint16_t                               value_type;
    typedef boost::uint16_t*                              pointer;
    typedef const boost::uint16_t*                        const_pointer;
    typedef boost::uint16_t&                              reference;
    typedef const boost::uint16_t&                        const_reference;
    typedef std::size_t                                   size_type;
    typedef std::ptrdiff_t                                difference_type;
    typedef boost::uint16_t*                              iterator;
    typedef const boost::uint16_t*                        const_iterator;
    typedef std::reverse_iterator<boost::uint16_t*>       reverse_iterator;
    typedef std::reverse_iterator<const boost::uint16_t*> const_reverse_iterator;
        
private:
        typedef vector<value_type>                                                        storage_type;

        storage_type storage_m;
        
        /*
                NOTE (eberdahl@adobe.com): Because c_str is required to return a null-terminated sequence,
                we ensure that the storage vector is always null-terminated. This means that the storage
                is always either empty or contains one extra character to hold the null-character.
         */
        template <typename ForwardIterator>
        void assign(ForwardIterator first, ForwardIterator last, std::forward_iterator_tag)
        {
                storage_type tmp;
                if (first != last)
                {
                        const size_type len(std::distance(first, last));
                        tmp.reserve(len + 1);
                        tmp.insert(tmp.end(), first, last);
                        tmp.push_back(boost::uint16_t(0));
                }
                storage_m.swap(tmp);
        }
        
        template <typename InputIterator>
        void assign(InputIterator first, InputIterator last, std::input_iterator_tag)
        {
                storage_type tmp;
                if (first != last)
                {
                        tmp.insert(tmp.end(), first, last);
                        tmp.push_back(boost::uint16_t(0));
                }
                storage_m.swap(tmp);
        }
        
        template <typename Iterator>
        void assign(Iterator first, Iterator last)
        {
                assign(first, last,
                           typename std::iterator_traits<Iterator>::iterator_category());
        }
        
public:
        /*!
                Constructs an empty string.
         */
        string16_t() { }
        
        /*!
                Copy constructor.
         */     
        string16_t(const string16_t& s) : storage_m(s.storage_m) { }

    /*!
        Move constructor.
         */
    string16_t(move_from<string16_t> x) : storage_m(::adobe::move(x.source.storage_m)) { }

        /*!
                Constructs a string16_t from a null-terminated sequence of 16-bit elements.
         
                \param[in] s null-terminated sequence from which to construct the string16_t
         */
        string16_t(const boost::uint16_t* s);

        /*!
                Constructs a string from a sequence of 16-bit elements.
         
                \param[in] s sequence of 16-bit elements from which to construct the string16_t
                \param[in] length number of elements from s from which to construct the string16_t
         */
        string16_t(const boost::uint16_t* s, std::size_t length);
        
        /*!
                Constructs a string from a generic sequence.
         
                \param[in] first start of sequence from which to construct the string16_t
                \param[in] last end of sequence from which to construct the string16_t
         */
        template <typename Iterator>
        string16_t(Iterator first, Iterator last)
                { assign(first, last); }

        /*!
                Destructor.
         */
        ~string16_t() { }
        
        /*!
                Assignment operator.
         */
        string16_t& operator=(string16_t s)
                { storage_m = ::adobe::move(s.storage_m); return *this; }
        
        /*!
                Returns const pointer to a null-terminated sequence of 16-bit elements, identical to
                the string16_t.
         */
        const boost::uint16_t* c_str() const;
        
        /*!
                Appends one element to the end of the string.
         */
        void push_back(value_type c);
        
        /*!
                Appends a character sequence to the end of the string
         */
        template <typename Iterator>
        void append(Iterator first, Iterator last)
        {
                if (first != last)
                {
                        if (!storage_m.empty())
                                storage_m.pop_back();
                        
                        storage_m.insert(storage_m.end(), first, last);
                        storage_m.push_back(0);
                }
        }
        
        /*!
                Appends another string this one
         */
        void append(const string16_t& s)
                { append(s.begin(), s.end()); }
        
        /*!
                Appends null-terminated character sequence to the string
         */
        void append(const boost::uint16_t* s);
        
        /*!
                Appends a number of characters from a sequence to the string
         */
        void append(const boost::uint16_t* s, std::size_t length)
                { append(s, s + length); }
        
        /*!
                Appends another string this one
         */
        string16_t& operator+=(const string16_t& s)
                { append(s); return *this; }
        
        /*!
                Appends null-terminated character sequence to the string
         */
        string16_t& operator+=(const boost::uint16_t* s)
                { append(s); return *this; }
        
        /*!
                Returns an iterator to the first element of the string.
         */
        const_iterator begin() const
                { return storage_m.begin(); }
        
        /*!
                Returns an iterator just past the end of the string.
         */
        const_iterator end() const
                { return storage_m.empty() ? storage_m.end() : boost::prior(storage_m.end()); }
        
        /*!
                Returns a reverse_iterator to the end of the current string.
         */
        const_reverse_iterator rbegin() const
                { return const_reverse_iterator(end()); }
        
        /*!
                Returns a reverse_iterator to the beginning of the current string.
         */
        const_reverse_iterator rend() const
                { return const_reverse_iterator(begin()); }
        
        /*!
                Returns the size of the allocated storage space for the
                elements of the string.
         */
        size_type capacity() const;
        
        /*!
                Requests that the capacity of the allocated storage space for
                the elements of the string be at least enough to hold n
                elements.
         */
        void reserve(size_type n)
                { storage_m.reserve(0 == n ? 0 : n + 1); }
        
        /*!
                Changes the string to be zero-length.
         */
        void clear()
                { storage_m.clear(); }
        
        /*!
                Returns the number of elements in the current string.
         */
    size_type size() const
                { return storage_m.empty() ? 0 : storage_m.size() - 1; }
        
        /*!
                Returns true if the string has no elements, false otherwise.
         */
    bool empty() const
                { return storage_m.empty(); }
        
        /*!
                Exchanges the elements of the current string with those of from s.
         */
        void swap(string16_t& s)
                { storage_m.swap(s.storage_m); }
        
        friend bool operator==(const string16_t& x, const string16_t& y)
        {
                return x.storage_m == y.storage_m;
        }
        
        friend bool operator<(const string16_t& x, const string16_t& y)
        {
                return x.storage_m < y.storage_m;
        }
    
    friend inline void swap(string16_t& x, string16_t& y) { x.storage_m.swap(y.storage_m); }
};

/*!
        Concatenate an entity to a string.
        Anything that can be concatenated using string16_t::operator+=
        may also be concatenated to a string16_t using operator+.
 */

inline string16_t operator+(string16_t s1, const string16_t& s2)      { return ::adobe::move(s1 += s2); }
inline string16_t operator+(string16_t s1, const boost::uint16_t* s2) { return ::adobe::move(s1 += s2); }

//!@}

/*************************************************************************************************/

BOOST_STATIC_ASSERT(sizeof(string_t) == sizeof(vector<char>));
BOOST_STATIC_ASSERT(sizeof(string16_t) == sizeof(vector<boost::uint16_t>));

/*************************************************************************************************/

} // namespace version_1

/*************************************************************************************************/

using version_1::string_t;
using version_1::string16_t;

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

ADOBE_NAME_TYPE_0("string_t:version_1:adobe", adobe::version_1::string_t)
ADOBE_NAME_TYPE_0("string16_t:version_1:adobe", adobe::version_1::string16_t)

ADOBE_SHORT_NAME_TYPE('s','t','r','g', adobe::version_1::string_t)
ADOBE_SHORT_NAME_TYPE('s','t','1','6', adobe::version_1::string16_t)

/*************************************************************************************************/

namespace boost {

template <>
struct has_nothrow_constructor<adobe::version_1::string_t> : boost::mpl::true_ { };

template <>
struct has_nothrow_constructor<adobe::version_1::string16_t> : boost::mpl::true_ { };

} // namespace boost

/*************************************************************************************************/

#endif

/*************************************************************************************************/
