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
    m_last_added_orders.insert(retval);
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
            m_last_deleted_orders.insert(it->first);
            retval = true;
        }
    }
    return retval;
}

void OrderSet::Reset() {
    m_orders.clear();
    m_last_added_orders.clear();
    m_last_deleted_orders.clear();
}

std::pair<OrderSet, std::set<int>> OrderSet::ExtractChanges() {
    OrderSet added_orders;
    for(int added : m_last_added_orders) {
        auto it = m_orders.find(added);
        if (it != m_orders.end()) {
            added_orders.m_orders.insert(*it);
        } else {
            m_last_deleted_orders.insert(added);
        }
    }
    m_last_added_orders.clear();
    std::set<int> deleted_orders = std::move(m_last_deleted_orders);
    m_last_deleted_orders.clear();
    return {added_orders, deleted_orders};
}
