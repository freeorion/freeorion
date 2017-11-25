#ifndef _OrderSet_h_
#define _OrderSet_h_


#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "Export.h"

#include <map>
#include <memory>


class Order;

/** The pointer type used to store Orders in OrderSets. */
typedef std::shared_ptr<Order> OrderPtr;

/** A collection of orders that may be searched using arbitrary predicate
    functions and functors.

    An OrderSet is typically composed on the client with IssueOrder(order), and
    then sent to the server to be executed again. IssueOrder(order) immediately
    executes the order to change the client's backend state.  The second
    execution on the server changes the server game state. Orders only executed
    on the client do not change the server save game state.

    On game reload, reloaded orders need to be re-executed on the client because
    the client state start from the last saved server state. Orders are
    saved/serialized as unexecuted so that when they are loaded they can be
    mixed with newly issued orders and both the loaded and newly issued orders
    will only execute once.

    RescindOrder(order) will try Order::Undo() and if that succeeds remove the
    order from the set.  */
class FO_COMMON_API OrderSet {
private:
    typedef std::map<int, OrderPtr> OrderMap;

public:
    typedef OrderMap::const_iterator    const_iterator;    ///< defines a public const_iterator type for OrderSet

    /** \name Accessors */ //@{
    const_iterator begin() const {return m_orders.begin();} ///< returns the begin const_iterator for the OrderSet
    const_iterator end() const   {return m_orders.end();}   ///< returns the end const_iterator for the OrderSet
    //@}

    /** \name Mutators */ //@{
    /** Execute the \p order immediately on the client.
        Store the \p order in the OrderSet to be executed later on the server.
        Return an index that can be used to reference the order. */
    int            IssueOrder(const OrderPtr& order);
    int            IssueOrder(OrderPtr&& order);

    /** Applies all Orders in the OrderSet.  As of this writing, this is needed only after deserializing an OrderSet
        client-side during game loading. */
    void           ApplyOrders();

    /** Try to Undo() \p order and if it succeeds remove the order from the
        set.  Return true if \p order exists and was successfully removed. */
    bool           RescindOrder(int order);
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
