#ifndef _OrderSet_h_
#define _OrderSet_h_

/*#ifndef	_Order_h_
#include "Order.h"
#endif*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// ONLY TEMPORARY!!!!!
class Order {};
// ONLY TEMPORARY!!!!!
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <set>

/** a collection of orders that may be searched using arbitrary predicate functions */
class OrderSet : private std::map<int, Order>
{
public:
   typedef std::map<int, Order>::const_iterator iterator; ///< defines a public const iterator

   /** \name Structors */ //@{
   OrderSet();
   //@}
   
   /** \name Accessors */ //@{
   const Order*   ExamineOrder(int order) const;   ///< returns a pointer to any order, so that it can be examined through its accessors; returns 0 if no such order exists
   
   template <class Pred> const Order*            FindOrder(Pred p) const;  ///< returns the first order that matches predicate p, or 0 if none matches
   template <class Pred> std::set<const Order*>  FindOrders(Pred p) const; ///< returns a set of the orders that match p

   iterator begin() const {return std::map<int, Order>::begin();} ///< returns the begin (const) iterator for the OrderSet
   iterator end() const {return std::map<int, Order>::end();}     ///< returns the end (const) iterator for the OrderSet
   //@}

   /** \name Mutators */ //@{   
   int            IssueOrder(const Order& order);  ///< stores an order in the OrderSet; returns an index that can be used to reference the order
   bool           RecindOrder(int order);          ///< removes the order from the OrderSet; returns true on success, false if there was no such order or the order is non-recindable
   void           Reset(); ///< clears all orders; should be called at the beginning of a new turn
   //@}

private:
   int   m_lowest_free_index; ///< the lowest unused nonnegative index number
};

#endif // _OrderSet_h_

