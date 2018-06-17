#include "OrderSet.h"
#include "Logger.h"

#include "Order.h"


namespace {
    OrderPtr EMPTY_ORDER_PTR;
}

OrderPtr& OrderSet::operator[](std::size_t i) {
    auto it = m_orders.find(i);
    if (it == m_orders.end())
        return EMPTY_ORDER_PTR;
    return it->second;
}

int OrderSet::IssueOrder(const OrderPtr& order)
{ return IssueOrder(OrderPtr(order)); }

int OrderSet::IssueOrder(OrderPtr&& order) {
    int retval = ((m_orders.rbegin() != m_orders.rend()) ? m_orders.rbegin()->first + 1 : 0);

    // Insert the order into the m_orders map.  forward the rvalue to use the move constructor.
    auto inserted = m_orders.insert({retval, std::forward<OrderPtr>(order)});
    inserted.first->second->Execute();

    TraceLogger() << "OrderSetIssueOrder m_orders size: " << m_orders.size();

    return retval;
}

void OrderSet::ApplyOrders() {
    DebugLogger() << "OrderSet::ApplyOrders() executing " << m_orders.size() << " orders";
    for (auto& order : m_orders)
        order.second->Execute();
}

bool OrderSet::RescindOrder(int order) {
    bool retval = false;
    auto it = m_orders.find(order);
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
