#ifndef _OrderSet_h_
#define _OrderSet_h_


#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "Export.h"

#include <map>
#include <memory>
#include <set>
#include <string>

class Order;
struct ScriptingContext;

/** The pointer type used to store Orders in OrderSets. */
typedef std::shared_ptr<Order> OrderPtr; // TODO: can this be unique_ptr ?

/** A collection of orders that may be searched using arbitrary predicate
    functions and functors.

    An OrderSet is typically composed on the client with IssueOrder(order), and
    then sent to the server to be executed again. IssueOrder(order) immediately
    executes the order to change the client's backend state.  The second
    execution on the server changes the server game state. Orders only executed
    on the client do not change the server save game state.

    On game reload, reloaded orders need to be re-executed on the client because
    the client state starts from the last saved server state. Orders are
    saved/serialized as unexecuted so that when they are loaded they can be
    mixed with newly issued orders and both the loaded and newly issued orders
    will only execute once.

    RescindOrder(order) will try Order::Undo() and if that succeeds remove the
    order from the set.  */
class FO_COMMON_API OrderSet {
private:
    typedef std::map<int, OrderPtr> OrderMap;

public:
    typedef OrderMap::const_iterator const_iterator;
    typedef OrderMap::iterator iterator;
    typedef OrderMap::value_type value_type;
    typedef OrderMap::size_type size_type;
    typedef OrderMap::key_type key_type;
    typedef OrderMap::difference_type difference_type;
    typedef OrderMap::key_compare key_compare;

    [[nodiscard]] const_iterator begin() const noexcept        { return m_orders.begin(); }
    [[nodiscard]] const_iterator end() const noexcept          { return m_orders.end(); }
    [[nodiscard]] iterator       begin() noexcept              { return m_orders.begin(); }
    [[nodiscard]] iterator       end() noexcept                { return m_orders.end(); }
    [[nodiscard]] std::size_t    size() const noexcept         { return m_orders.size(); }
    [[nodiscard]] bool           empty() const noexcept        { return m_orders.empty(); }
    [[nodiscard]] iterator       find(const key_type& k)       { return m_orders.find(k); }
    auto                         insert(const value_type& val) { return m_orders.insert(val); }
    auto                         insert(iterator begin, iterator end) { return m_orders.insert(begin, end); }
    auto                         erase(const key_type& k)      { return m_orders.erase(k); }

    [[nodiscard]] std::string Dump() const;

    /** Execute the \p order immediately on the client.
      * Store the \p order in the OrderSet to be executed later on the server. */
    void IssueOrder(OrderPtr order, ScriptingContext& context);

    /** Construct and execute an order of specified type on the client.
      * Store the order in the OrderSet to be executed later on the server.
      * Returns a pointer to the order. */

    template <typename OrderType, typename... ParamTs>
    auto IssueOrder(ScriptingContext& context, ParamTs&&... params)
    {
        static_assert(std::is_base_of_v<Order, std::decay_t<OrderType>>);
        if constexpr (requires { OrderType(std::forward<ParamTs>(params)..., context); }) {
            auto order = std::make_shared<OrderType>(std::forward<ParamTs>(params)..., context);
            IssueOrder(order, context);
            return order;
        } else {
            auto order = std::make_shared<OrderType>(std::forward<ParamTs>(params)...);
            IssueOrder(order, context);
            return order;
        }
    }

    /** Applies all Orders in the OrderSet.  As of this writing, this is needed only after deserializing an OrderSet
        client-side during game loading. */
    void ApplyOrders(ScriptingContext& context);

    /** Try to Undo() \p order and if it succeeds remove the order from the
        set.  Return true if \p order exists and was successfully removed. */
    bool RescindOrder(int order, ScriptingContext& context);
    void Reset(); ///< clears all orders; should be called at the beginning of a new turn

    std::pair<OrderSet, std::set<int>> ExtractChanges(); ///< extract and clear changed orders

private:
    OrderMap      m_orders;
    std::set<int> m_last_added_orders; ///< set of ids added/updated orders
    std::set<int> m_last_deleted_orders; ///< set of ids deleted orders

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// Template Implementations
template <typename Archive>
void OrderSet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_orders);
    if constexpr (Archive::is_loading::value) {
        m_last_added_orders.clear();
        m_last_deleted_orders.clear();
    }
}


#endif
