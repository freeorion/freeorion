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

    FO_COMMON_API void CheckSumCombine(unsigned int& sum, bool t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, unsigned char t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, unsigned int t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, unsigned long int t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, char t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, int t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, long int t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, double t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, float t);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const char* s);
    FO_COMMON_API void CheckSumCombine(unsigned int& sum, const std::string& c);

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

    // pairs (including map value types)
    template <class C, class D>
    void CheckSumCombine(unsigned int& sum, const std::pair<C, D>& p)
    {
        TraceLogger() << "CheckSumCombine(pair): " << typeid(p).name();
        CheckSumCombine(sum, p.first);
        CheckSumCombine(sum, p.second);
    }

    // vectors, sets, maps
    template <class C>
    void CheckSumCombine(unsigned int& sum, const typename std::vector<C>& v)
    {
        TraceLogger() << "CheckSumCombine(vector<C>): " << typeid(v).name();
        for (const auto& t : v)
            CheckSumCombine(sum, t);
        sum += v.size();
        sum %= CHECKSUM_MODULUS;
    }
    template <class C>
    void CheckSumCombine(unsigned int& sum, const typename std::set<C>& s)
    {
        TraceLogger() << "CheckSumCombine(set<C>): " << typeid(s).name();
        for (const auto& t : s)
            CheckSumCombine(sum, t);
        sum += s.size();
        sum %= CHECKSUM_MODULUS;
    }
    template <class C, class D>
    void CheckSumCombine(unsigned int& sum, const typename std::map<C, D>& m)
    {
        TraceLogger() << "CheckSumCombine(map<C, D>): " << typeid(m).name();
        for (const auto& t : m)
            CheckSumCombine(sum, t);
        sum += m.size();
        sum %= CHECKSUM_MODULUS;
    }
}

#endif // _CheckSums_h_
