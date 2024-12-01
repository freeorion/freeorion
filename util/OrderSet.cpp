#include "OrderSet.h"
#include "Logger.h"

#include "Order.h"


std::string OrderSet::Dump() const {
    std::string retval;
    for (const auto& [order_id, order] : m_orders)
        retval += std::to_string(order_id) + ": " + order->Dump() + "\n";
    return retval;
}

void OrderSet::IssueOrder(OrderPtr order, ScriptingContext& context) {
    const int order_id = (!m_orders.empty() ? m_orders.rbegin()->first + 1 : 0);

    // Insert the order into the m_orders map.  forward the rvalue to use the move constructor.
    auto [it, insert_ran] = m_orders.emplace(order_id, std::move(order));
    if (!insert_ran)
        ErrorLogger() << "OrderSet::IssueOrder unexpected didn't succeed inserting order";

    m_last_added_orders.insert(order_id);

    try {
        it->second->Execute(context);
    } catch (const std::exception& e) {
        ErrorLogger() << "OrderSet::IssueOrder caught exception issuing order: " << e.what();
    }

    TraceLogger() << "OrderSetIssueOrder m_orders size: " << m_orders.size();
}

void OrderSet::ApplyOrders(ScriptingContext& context) {
    DebugLogger() << "OrderSet::ApplyOrders() executing " << m_orders.size() << " orders";
    unsigned int executed_count = 0, failed_count = 0, already_executed_count = 0;

    for (auto& [order_id, order] : m_orders) {
        if (order->Executed()) {
            DebugLogger() << "Order " << order_id << " already executed";
            ++already_executed_count;
            continue;
        }

        try {
            const auto order_empire_id = order->EmpireID();
            const auto order_empire = context.GetEmpire(order_empire_id);
            if (!order_empire) {
                ErrorLogger() << "ApplyOrders couldn't get empire with id " << order_empire_id;
                return;
            }
            const auto source = order_empire->Source(context.ContextObjects()).get();
            ScriptingContext empire_context(context, ScriptingContext::Source{}, source);
            order->Execute(empire_context);
            ++executed_count;
        } catch (const std::exception& e) {
            ++failed_count;
            ErrorLogger() << "Caught exception executing order " << order << ": " << e.what();
        }
    }
    DebugLogger() << "OrderSet::ApplyOrders() successfully executed " << executed_count
                  << " orders, skipped " << already_executed_count
                  << " already executed orders, and tried but failed to execute " << failed_count
                  << " orders";
}

bool OrderSet::RescindOrder(int order, ScriptingContext& context) {
    auto it = m_orders.find(order);
    if (it != m_orders.end() && it->second->Undo(context)) {
        m_last_deleted_orders.emplace(it->first);
        m_orders.erase(it);  // Invalidates it
        return true;
    }
    return false;
}

void OrderSet::Reset() {
    m_orders.clear();
    m_last_added_orders.clear();
    m_last_deleted_orders.clear();
}

std::pair<OrderSet, std::set<int>> OrderSet::ExtractChanges() {
    OrderSet added_orders;
    for (int added : m_last_added_orders) {
        auto it = m_orders.find(added);
        if (it != m_orders.end())
            added_orders.m_orders.emplace(*it);
        else
            m_last_deleted_orders.emplace(added);
    }
    m_last_added_orders.clear();
    auto deleted_orders = std::move(m_last_deleted_orders);
    m_last_deleted_orders.clear();
    return {std::move(added_orders), std::move(deleted_orders)};
}
