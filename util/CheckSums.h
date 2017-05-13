#ifndef _CheckSums_h_
#define _CheckSums_h_

#include "Export.h"
#include "Logger.h"

#include <iostream>
#include <typeinfo>

namespace CheckSums {
    const unsigned int CHECKSUM_MODULUS = 10000000U;    // reasonably big number that should be well below UINT_MAX, which is ~4.29x10^9 for 32 bit unsigned int

    FO_COMMON_API void CheckSumCombine(unsigned int& sum, bool t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, unsigned char t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, unsigned int t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, unsigned long int t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, char t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, int t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, long int t);

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

    // pairs (including map value types)
    template <class C, class D>
    void CheckSumCombine(unsigned int& sum, const std::pair<C, D>& p)
    {
        TraceLogger() << "CheckSumCombine(pair): " << typeid(p).name();
        CheckSumCombine(sum, p.first);
        CheckSumCombine(sum, p.second);
    }

    // vectors, maps, and strings
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

    // classes that have GetCheckSum methods
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
