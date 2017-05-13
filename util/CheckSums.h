#ifndef _CheckSums_h_
#define _CheckSums_h_

#include "Export.h"
#include "Logger.h"

#include <iostream>
#include <typeinfo>

namespace CheckSums {
    const unsigned int CHECKSUM_MODULUS = 10000000U;    // reasonably big number that should be well below UINT_MAX, which is ~4.29x10^9 for 32 bit unsigned int

    // unsigned types (eg. bool, unsigned int, unsigned long int, unsigned char)
    template <class T, typename std::enable_if<std::is_unsigned<T>::value, T>::type* = nullptr>
    void CheckSumCombine(unsigned int& sum, T t) {
        TraceLogger() << "CheckSumCombine(unsigned T): " << typeid(t).name();
        sum += static_cast<unsigned int>(t);
        sum %= CHECKSUM_MODULUS;
    }

    // signed types (eg. int, char) but not float or double which are covered by specialized functions
    template <class T, typename std::enable_if<std::is_signed<T>::value, T>::type* = nullptr>
    void CheckSumCombine(unsigned int& sum, T t) {
        //TraceLogger() << "CheckSumCombine(signed T): " << typeid(t).name();
        sum += static_cast<unsigned int>(std::abs(t));
        sum %= CHECKSUM_MODULUS;
    }

    // enums
    template <class T>
    void CheckSumCombine(unsigned int& sum, T t, typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr) {
        TraceLogger() << "CheckSumCombine(enum): " << typeid(t).name();
        CheckSumCombine(sum, static_cast<int>(t) + 10);
    }

    // pointer types
    template <class T>
    void CheckSumCombine(unsigned int& sum, T p,
                         decltype(*std::declval<T>())* val = nullptr)
    {
        TraceLogger() << "CheckSumCombine(T*): " << typeid(p).name();
        if (p)
            CheckSumCombine(sum, *p);
    }

    // applies to pairs (including map value types)
    template <class C, class D>
    void CheckSumCombine(unsigned int& sum, const std::pair<C, D>& p)
    {
        TraceLogger() << "CheckSumCombine(pair): " << typeid(p).name();
        CheckSumCombine(sum, p.first);
        CheckSumCombine(sum, p.second);
    }

    // should apply to vectors, maps, and strings
    template <class C>
    void CheckSumCombine(unsigned int& sum, const C& c,
                         const typename C::const_iterator* it = nullptr,
                         const typename C::value_type* val = nullptr)
    {
        TraceLogger() << "CheckSumCombine(C container): " << typeid(c).name();
        for (const typename C::value_type& t : c)
            CheckSumCombine(sum, t);
        sum += c.size();
        sum %= CHECKSUM_MODULUS;
    }

    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const char* s);

    // applies to classes that have GetCheckSum methods
    template <class C>
    void CheckSumCombine(unsigned int& sum, const C& c,
                         decltype(((C*)nullptr)->GetCheckSum())* val = nullptr)
    {
        TraceLogger() << "CheckSumCombine(C with GetCheckSum): " << typeid(c).name();
        sum += c.GetCheckSum();
        sum %= CHECKSUM_MODULUS;
    }

    FO_COMMON_API void CheckSumCombine(unsigned int& sum, double t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, float t);
}

#endif // _CheckSums_h_
