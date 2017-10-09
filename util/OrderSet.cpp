#include "OrderSet.h"
#include "Logger.h"

#include "Order.h"


OrderSet::OrderSet()
{}

const OrderPtr OrderSet::ExamineOrder(int order) const {
    OrderPtr retval;
    auto it = m_orders.find(order);
    if (it != m_orders.end())
        retval = it->second;
    return retval;
}

int OrderSet::IssueOrder(const OrderPtr& order)
{ return IssueOrder(OrderPtr(order)); }

int OrderSet::IssueOrder(OrderPtr&& order) {
    int retval = ((m_orders.rbegin() != m_orders.rend()) ? m_orders.rbegin()->first + 1 : 0);

    // Insert the order into the m_orders map.  forward the rvalue to use the move constructor.
    auto inserted = m_orders.insert(std::make_pair(retval, std::forward<OrderPtr>(order)));
    inserted.first->second->Execute();

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

void OrderSet::Reset() {
    for (auto& order : m_orders) {
        if (!order.second->ShouldPersist()) {
            auto it = m_orders.find(order.first);
            m_orders.erase(it);
        }
        else {
            order.second->ResetExecutionStatus();
        }
    }
}

void OrderSet::ExecutePersistentOrders() {
    for (auto& order : m_orders) {
        if (order.second->ShouldPersist()) {
            order.second->Execute();
        }
    }
}
