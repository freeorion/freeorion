#include "OrderSet.h"

OrderSet::OrderSet()
{
}

OrderSet::~OrderSet()
{
   Reset();
}

const Order* OrderSet::ExamineOrder(int order) const
{
   const Order* retval = 0;
   OrderMap::const_iterator it = m_orders.find(order);
   if (it != m_orders.end()) {
      retval = it->second;
   }
   return retval;
}
   
int OrderSet::IssueOrder(Order* order)
{
   int retval = ((m_orders.rbegin() != m_orders.rend()) ? m_orders.rbegin()->first + 1 : 0);
   m_orders[retval] = order;
   return retval;
}

bool OrderSet::RecindOrder(int order)
{
   bool retval = false;
   OrderMap::iterator it = m_orders.find(order);
   if (it != m_orders.end()) {
      m_orders.erase(it);
      retval = true;
   }
   return retval;
}

void OrderSet::Reset()
{
   for (OrderMap::iterator it = m_orders.begin(); it != m_orders.end(); ++it) {
      delete it->second;
   }
   m_orders.clear();
}

