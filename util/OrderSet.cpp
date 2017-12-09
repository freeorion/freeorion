#include "OrderSet.h"
#include "Logger.h"

#include "Order.h"


int OrderSet::IssueOrder(const OrderPtr& order)
{ return IssueOrder(OrderPtr(order)); }

int OrderSet::IssueOrder(OrderPtr&& order) {
    int retval = ((m_orders.rbegin() != m_orders.rend()) ? m_orders.rbegin()->first + 1 : 0);

    // Insert the order into the m_orders map.  forward the rvalue to use the move constructor.
    auto inserted = m_orders.insert(std::make_pair(retval, std::forward<OrderPtr>(order)));
    inserted.first->second->Execute();

    // DebugLogger() << "OrderSet::IssueOrder() issued order No. " << retval;

    return retval;
}

void OrderSet::ApplyOrders() {
    DebugLogger() << "OrderSet::ApplyOrders() executing " << m_orders.size() << " orders";
    for (auto& order : m_orders) {
        order.second->Execute();
        // DebugLogger() << "OrderSet::ApplyOrders(): Executing order No. " << order.first;
    }
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

void OrderSet::ResetNonPersistentOrders() {
    for (auto& order : m_orders) {
        if (!order.second->ShouldPersist()) {
            auto it = m_orders.find(order.first);
            if (it != m_orders.end())
                m_orders.erase(it);
        }
    }
    //DebugLogger() << "OrderSet::ResetNonPersistentOrders(): Persisting orders: " << m_orders.size();
}

void OrderSet::ExecutePersistentOrders() {
    for (auto& order : m_orders) {
        if (order.second->ShouldPersist()) {
            order.second->ResetExecutionStatus();
            order.second->Execute();
            //DebugLogger() << "OrderSet::ExecutePersistentOrders(): executed persistent order " << order.first;
        }
    }
}
