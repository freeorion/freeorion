#include "OrderSet.h"
#include "Logger.h"

#include "Order.h"


OrderSet::OrderSet()
{}

const OrderPtr OrderSet::ExamineOrder(int order) const {
    OrderPtr retval;
    OrderMap::const_iterator it = m_orders.find(order);
    if (it != m_orders.end())
        retval = it->second;
    return retval;
}

int OrderSet::IssueOrder(const OrderPtr& order, bool suppress_immediate_execution /*= false*/)
{ return IssueOrder(OrderPtr(order), suppress_immediate_execution); }

int OrderSet::IssueOrder(OrderPtr&& order, bool suppress_immediate_execution /*= false*/) {
    int retval = ((m_orders.rbegin() != m_orders.rend()) ? m_orders.rbegin()->first + 1 : 0);

    // Insert the order into the m_orders map.  forward the rvalue to use the move constructor.
    auto inserted = m_orders.insert(std::make_pair(retval, std::forward<OrderPtr>(order)));

    if (!suppress_immediate_execution)
        inserted.first->second->Execute();

    return retval;
}

void OrderSet::ApplyOrders() {
    DebugLogger() << "OrderSet::ApplyOrders() executing " << m_orders.size() << " orders";
    for (OrderMap::value_type& order : m_orders)
        order.second->Execute();
}

bool OrderSet::RescindOrder(int order) {
    bool retval = false;
    OrderMap::iterator it = m_orders.find(order);
    if (it != m_orders.end()) {
        if (it->second->Undo()) {
            m_orders.erase(it);
            retval = true;
        }
    }
    return retval;
}

void OrderSet::Reset()
{ m_orders.clear(); }
