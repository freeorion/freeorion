#include "OrderSet.h"

OrderSet::OrderSet() : 
   m_lowest_free_index(0)
{
}

const Order* OrderSet::ExamineOrder(int order) const
{
   const Order* retval = 0;
   return retval;
}
   
template <class Pred> const Order* OrderSet::FindOrder(Pred p) const
{
   const Order* retval = 0;
   return retval;
}

template <class Pred> std::set<const Order*> OrderSet::FindOrders(Pred p) const
{
   std::set<const Order*> retval;
   return retval;
}

int OrderSet::IssueOrder(const Order& order)
{
   int retval = 0;
   return retval;
}

bool OrderSet::RecindOrder(int order)
{
   bool retval = false;
   return retval;
}

void OrderSet::Reset()
{
}

