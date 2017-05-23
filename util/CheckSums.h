#ifndef _CheckSums_h_
#define _CheckSums_h_

#include "Export.h"
#include "Logger.h"

#include <iostream>
#include <typeinfo>
#include <set>
#include <map>
#include <vector>

namespace CheckSums {
    const unsigned int CHECKSUM_MODULUS = 10000000U;    // reasonably big number that should be well below UINT_MAX, which is ~4.29x10^9 for 32 bit unsigned int

    FO_COMMON_API void CheckSumCombine(unsigned int& sum, double t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, float t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const char* s);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const std::string& c);

    // integeral types
    template <class T>
    void CheckSumCombine(unsigned int& sum, T t,
                         typename std::enable_if<std::is_signed<T>::value, T>::type* = nullptr)
    {
        sum += static_cast<unsigned int>(std::abs(t));
        sum %= CHECKSUM_MODULUS;
    }
    template <class T>
    void CheckSumCombine(unsigned int& sum, T t,
                         typename std::enable_if<std::is_unsigned<T>::value, T>::type* = nullptr)
    {
        sum += static_cast<unsigned int>(t);
        sum %= CHECKSUM_MODULUS;
    }

    // classes that have GetCheckSum methods
    template <class C>
    void CheckSumCombine(unsigned int& sum, const C& c,
                         decltype(std::declval<C>().GetCheckSum())* = nullptr)
    {
        TraceLogger() << "CheckSumCombine(C with GetCheckSum): " << typeid(c).name();
        sum += c.GetCheckSum();
        sum %= CHECKSUM_MODULUS;
    }

    // enums
    template <class T>
    void CheckSumCombine(unsigned int& sum, T t,
                         typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr)
    {
        TraceLogger() << "CheckSumCombine(enum): " << typeid(t).name();
        CheckSumCombine(sum, static_cast<int>(t) + 10);
    }

    // pointer types
    template <class T>
    void CheckSumCombine(unsigned int& sum, const T* p)
    {
        TraceLogger() << "CheckSumCombine(T*): " << typeid(p).name();
        if (p)
            CheckSumCombine(sum, *p);
    }
    template <class T>
    void CheckSumCombine(unsigned int& sum, const typename std::shared_ptr<T>& p)
    {
        TraceLogger() << "CheckSumCombine(shared_ptr<T>): " << typeid(p).name();
        if (p)
            CheckSumCombine(sum, *p);
    }
    template <class T>
    void CheckSumCombine(unsigned int& sum, const typename std::unique_ptr<T>& p)
    {
        TraceLogger() << "CheckSumCombine(unique_ptr<T>): " << typeid(p).name();
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

    // iterable containers
    template <class C>
    void CheckSumCombine(unsigned int& sum, const C& c,
                         decltype(std::declval<C>().begin())* = nullptr,
                         decltype(std::declval<C>().end())* = nullptr)
    {
        TraceLogger() << "CheckSumCombine(Container C): " << typeid(c).name();
        for (const auto& t : c)
            CheckSumCombine(sum, t);
        sum += c.size();
        sum %= CHECKSUM_MODULUS;
    }
}

#endif // _CheckSums_h_
