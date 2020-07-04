#ifndef _focs_hpp_
#define _focs_hpp_


namespace focs {

enum OpType : int;
enum ReferenceType : int;
enum StatisticType : int;
struct NameLookup;
struct ValueRefBase;
template <typename From> struct StringCast;
template <typename From> struct UserStringLookup;
template <typename From, typename To> struct StaticCast;
template <typename Type> struct ComplexVariable;
template <typename Type> struct Constant;
template <typename Type> struct Statistic;
template <typename Type> struct ValueRef;
template <typename Type> struct Variable;
template <typename Type> struct Operation;

}


#endif
