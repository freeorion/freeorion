// -*- C++ -*-
#ifndef _OrderSet_h_
#define _OrderSet_h_

#ifndef _Order_h_
#include "Order.h"
#endif

#include <map>
#include <vector>

/** a collection of orders that may be searched using arbitrary predicate functions and functors*/
class OrderSet
{
private:
    typedef std::map<int, Order*> OrderMap;

public:
    typedef OrderMap::const_iterator    const_iterator;   ///< defines a public const_iterator type for OrderSet
    typedef std::vector<const Order*>   OrderVec;         ///< the return type of FindOrders()

    /** \name Structors */ //@{
    OrderSet();
    ~OrderSet();
    //@}
   
    /** \name Accessors */ //@{
    const Order*   ExamineOrder(int order) const;   ///< returns a pointer to any order, so that it can be examined through its accessors; returns 0 if no such order exists
   
    /** returns all the orders that match \a pred.  Predicates used with this function must take a single const Order* 
        parameter and must return bool or a type for which there is a conversion to bool.*/
    template <class Pred>
    OrderVec FindOrders(Pred pred) const
    {
        OrderVec retval;
        for (OrderMap::const_iterator it = m_orders.begin(); it != m_orders.end(); ++it) {
            const Order* o = it->second;
            if (pred(o))
                retval.push_back(o);
        }
        return retval;
    }

    const_iterator begin() const {return m_orders.begin();} ///< returns the begin const_iterator for the OrderSet
    const_iterator end() const   {return m_orders.end();}   ///< returns the end const_iterator for the OrderSet
    //@}

    /** \name Mutators */ //@{
    /** executes a given Order, then stores it in the OrderSet. Returns an index that can be used to reference the
        order.  \warning The OrderSet assumes that the Order is allocated on the heap, and takes ownership of it.  Do
        not delete any Order* passed to OrderSet.*/
    int            IssueOrder(Order* order);

    /** Applies all Orders in the OrderSet.  As of this writing, this is needed only after deserializing an OrderSet
        client-side during game loading. */
    void           ApplyOrders();

    bool           RecindOrder(int order);    ///< removes the order from the OrderSet; returns true on success, false if there was no such order or the order is non-recindable
    void           Reset(); ///< clears all orders; should be called at the beginning of a new turn
    //@}

private:
    OrderMap m_orders;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// Template Implementations
template <class Archive>
void OrderSet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_orders);
}

#endif // _OrderSet_h_


