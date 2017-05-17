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

int OrderSet::IssueOrder(const OrderPtr& order) {
    int retval = ((m_orders.rbegin() != m_orders.rend()) ? m_orders.rbegin()->first + 1 : 0);
    m_orders[retval] = order;

    order->Execute();

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
