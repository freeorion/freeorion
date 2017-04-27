#ifndef _CheckSums_h_
#define _CheckSums_h_

#include <iostream>

namespace CheckSums {
    const unsigned int CHECKSUM_MODULUS = 10000000U;    // reasonably big number that should be well below UINT_MAX, which is ~4.29x10^9 for 32 bit unsigned int

    // unsigned types (eg. bool, unsigned int, unsigned long int, unsigned char)
    template <class T, typename std::enable_if<std::is_unsigned<T>::value, T>::type* = nullptr>
    void CheckSumCombine(unsigned int& sum, const T& t) {
        std::cout << "CheckSumCombine(unsigned T): " << typeid(t).name() << std::endl << std::endl;
        sum += static_cast<unsigned int>(t);
        sum %= CHECKSUM_MODULUS;
    }

    // signed types (eg. int, char) but not float or double which are covered by specialized functions
    template <class T, typename std::enable_if<std::is_signed<T>::value, T>::type* = nullptr>
    void CheckSumCombine(unsigned int& sum, const T& t) {
        //std::cout << "CheckSumCombine(signed T): " << typeid(t).name() << std::endl << std::endl;
        sum += static_cast<unsigned int>(std::abs(t));
        sum %= CHECKSUM_MODULUS;
    }

    // enums
    template <class T>
    void CheckSumCombine(unsigned int& sum, T t, typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr) {
        std::cout << "CheckSumCombine(enum): " << typeid(t).name() << std::endl << std::endl;
        CheckSumCombine(sum, static_cast<int>(t) + 10);
    }

    // pointer types
    template <class T>
    void CheckSumCombine(unsigned int& sum, T p,
                         decltype(*std::declval<T>())* val = nullptr)
    {
        std::cout << "CheckSumCombine(T*): " << typeid(p).name() << std::endl << std::endl;
        if (p)
            CheckSumCombine(sum, *p);
    }

    // fallback do nothing for unsupported types (without GetCheckSum functions)
    void CheckSumCombine(...)
    { std::cout << "CheckSumCombine(...)" << std::endl << std::endl; }

    // applies to pairs (including map value types)
    template <class C, class D>
    void CheckSumCombine(unsigned int& sum, const std::pair<C, D>& p)
    {
        std::cout << "CheckSumCombine(pair): " << typeid(p).name() << std::endl << std::endl;
        CheckSumCombine(sum, p.first);
        CheckSumCombine(sum, p.second);
    }

    // should apply to vectors, maps, and strings
    template <class C>
    void CheckSumCombine(unsigned int& sum, const C& c,
                         const typename C::const_iterator* it = nullptr,
                         const typename C::value_type* val = nullptr)
    {
        std::cout << "CheckSumCombine(C container): " << typeid(c).name() << std::endl << std::endl;
        for (const typename C::value_type& t : c)
            CheckSumCombine(sum, t);
        sum += c.size();
        sum %= CHECKSUM_MODULUS;
    }

    // applies to classes that have GetCheckSum methods
    template <class C>
    void CheckSumCombine(unsigned int& sum, C& c,
                         decltype(((C*)nullptr)->GetCheckSum())* val = nullptr)
    {
        std::cout << "CheckSumCombine(C with GetCheckSum): " << typeid(c).name() << std::endl << std::endl;
        sum += c.GetCheckSum();
        sum %= CHECKSUM_MODULUS;
    }

    // doubles
    void CheckSumCombine(unsigned int& sum, const double& t) {
        std::cout << "CheckSumCombine(double): " << typeid(t).name() << std::endl << std::endl;
        assert(DBL_MAX_10_EXP < 400);
        if (t == 0.0)
            return;
        // biggest and smallest possible double should be ~10^(+/-308)
        // taking log gives a number in the range +/- 309
        // adding 400 gives numbers in the range ~0 to 800
        // multiplying by 10'000 gives numbers in the range ~0 to 8'000'000
        sum += static_cast<unsigned int>((std::log10(std::abs(t)) + 400.0) * 10000.0);
        sum %= CHECKSUM_MODULUS;
    }

    // floats
    void CheckSumCombine(unsigned int& sum, const float& t) {
        std::cout << "CheckSumCombine(float): " << typeid(t).name() << std::endl << std::endl;
        assert(FLT_MAX_10_EXP < 40);
        if (t == 0.0f)
            return;
        // biggest and smallest possible float should be ~10^(+/-38)
        // taking log gives a number in the range +/- 39
        // adding 400 ives numbers in the range ~0 to 80
        // multiplying by 100'000 gives numbers in the range ~0 to 8'000'000
        sum += static_cast<unsigned int>((std::log10(std::abs(t)) + 40.0f) * 100000.0f);
        sum %= CHECKSUM_MODULUS;
    }
}

#endif // _CheckSums_h_
