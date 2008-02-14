#include "OrderSet.h"

#include "../util/MultiplayerCommon.h"



OrderSet::OrderSet()
{
}

const OrderPtr OrderSet::ExamineOrder(int order) const
{
    OrderPtr retval;
    OrderMap::const_iterator it = m_orders.find(order);
    if (it != m_orders.end()) {
        retval = it->second;
    }
    return retval;
}

int OrderSet::IssueOrder(OrderPtr order)
{
    int retval = ((m_orders.rbegin() != m_orders.rend()) ? m_orders.rbegin()->first + 1 : 0);
    m_orders[retval] = order;
    
    order->Execute();

    return retval;
}

void OrderSet::ApplyOrders()
{
    for (OrderMap::iterator it = m_orders.begin(); it != m_orders.end(); ++it) {
        it->second->Execute();
    }
}

bool OrderSet::RecindOrder(int order)
{
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
{
    m_orders.clear();
}
