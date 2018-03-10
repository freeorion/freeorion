#include "SystemResourceSummaryBrowseWnd.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/ResourceCenter.h"
#include "../universe/System.h"
#include "../universe/Enums.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "CUIControls.h"

namespace {
    /** Returns how much of specified \a resource_type is being consumed by the
      * empire with id \a empire_id at the location of the specified
      * object \a obj. */
    double ObjectResourceConsumption(std::shared_ptr<const UniverseObject> obj,
                                     ResourceType resource_type,
                                     int empire_id = ALL_EMPIRES)
    {
        if (!obj) {
            ErrorLogger() << "ObjectResourceConsumption passed a null object";
            return 0.0;
        }
        if (resource_type == INVALID_RESOURCE_TYPE) {
            ErrorLogger() << "ObjectResourceConsumption passed a INVALID_RESOURCE_TYPE";
            return 0.0;
        }


        const Empire* empire = nullptr;

        if (empire_id != ALL_EMPIRES) {
            empire = GetEmpire(empire_id);

            if (!empire) {
                ErrorLogger() << "ObjectResourceConsumption requested consumption for empire " << empire_id << " but this empire was not found";
                return 0.0;     // requested a specific empire, but didn't find it in this client, so production is 0.0
            }

            if (!obj->OwnedBy(empire_id)) {
                DebugLogger() << "ObjectResourceConsumption requested consumption for empire " << empire_id << " but this empire doesn't own the object";
                return 0.0;     // if the empire doesn't own the object, assuming it can't be consuming any of the empire's resources.  May need to revisit this assumption later.
            }
        }


        //std::shared_ptr<const PopCenter> pc;
        double prod_queue_allocation_sum = 0.0;
        std::shared_ptr<const Building> building;

        switch (resource_type) {
        case RE_INDUSTRY:
            // PP (equal to mineral and industry) cost of objects on production queue at this object's location
            if (empire) {
                // add allocated PP for all production items at this location for this empire
                for (const auto& elem : empire->GetProductionQueue())
                    if (elem.location == obj->ID())
                        prod_queue_allocation_sum += elem.allocated_pp;

            } else {
                // add allocated PP for all production items at this location for all empires
                for (auto& entry : Empires()) {
                    empire = entry.second;
                    for (const ProductionQueue::Element& elem : empire->GetProductionQueue())
                        if (elem.location == obj->ID())
                            prod_queue_allocation_sum += elem.allocated_pp;
                }
            }
            return prod_queue_allocation_sum;
            break;

        case RE_TRADE:
        case RE_RESEARCH:
        case RE_STOCKPILE:
            // research/stockpile aren't consumed at a particular location, so none is consumed at any location
        default:
            // for INVALID_RESOURCE_TYPE just return 0.0.  Could throw an exception, I suppose...
            break;
        }
        return 0.0;
    }

    const int       EDGE_PAD(3);
    GG::X LabelWidth()
    { return GG::X(ClientUI::Pts()*18); }

    GG::X ValueWidth()
    { return GG::X(ClientUI::Pts()*4); }
}

SystemResourceSummaryBrowseWnd::SystemResourceSummaryBrowseWnd(ResourceType resource_type,
                                                               int system_id, int empire_id) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, LabelWidth() + ValueWidth(), GG::Y1),
    m_resource_type(resource_type),
    m_system_id(system_id),
    m_empire_id(empire_id),
    m_production_label(nullptr),
    m_allocation_label(nullptr),
    m_import_export_label(nullptr),
    row_height(1),
    production_label_top(0),
    allocation_label_top(0),
    import_export_label_top(0)
{}

bool SystemResourceSummaryBrowseWnd::WndHasBrowseInfo(const GG::Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void SystemResourceSummaryBrowseWnd::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::FlatRectangle(ul, lr, OpaqueColor(ClientUI::WndColor()), ClientUI::WndOuterBorderColor(), 1);       // main background
    GG::FlatRectangle(GG::Pt(ul.x, ul.y + production_label_top), GG::Pt(lr.x, ul.y + production_label_top + row_height),
                      ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);                 // production label background
    GG::FlatRectangle(GG::Pt(ul.x, ul.y + allocation_label_top), GG::Pt(lr.x, ul.y + allocation_label_top + row_height),
                      ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);                 // allocation label background
    GG::FlatRectangle(GG::Pt(ul.x, ul.y + import_export_label_top), GG::Pt(lr.x, ul.y + import_export_label_top + row_height),
                      ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);                 // import or export label background
}

void SystemResourceSummaryBrowseWnd::UpdateImpl(std::size_t mode, const GG::Wnd* target) {
    // fully recreate browse wnd for each viewing.  finding all the queues, resourcepools and (maybe?) individual
    // UniverseObject that would have ChangedSignals that would need to be connected to the object that creates
    // this BrowseWnd seems like more trouble than it's worth to avoid recreating the BrowseWnd every time it's shown
    // (the alternative is to only reinitialize when something changes that would affect what's displayed in the
    // BrowseWnd, which is how MeterBrowseWnd works)
    Clear();
    Initialize();
}

void SystemResourceSummaryBrowseWnd::Initialize() {
    row_height = GG::Y(ClientUI::Pts() * 3/2);
    const GG::X TOTAL_WIDTH = LabelWidth() + ValueWidth();

    GG::Y top = GG::Y0;


    production_label_top = top;
    m_production_label = GG::Wnd::Create<CUILabel>("", GG::FORMAT_RIGHT);
    m_production_label->MoveTo(GG::Pt(GG::X0, production_label_top));
    m_production_label->Resize(GG::Pt(TOTAL_WIDTH - EDGE_PAD, row_height));
    m_production_label->SetFont(ClientUI::GetBoldFont());
    AttachChild(m_production_label);
    top += row_height;
    UpdateProduction(top);


    allocation_label_top = top;
    m_allocation_label = GG::Wnd::Create<CUILabel>("", GG::FORMAT_RIGHT);
    m_allocation_label->MoveTo(GG::Pt(GG::X0, allocation_label_top));
    m_allocation_label->Resize(GG::Pt(TOTAL_WIDTH - EDGE_PAD, row_height));
    m_allocation_label->SetFont(ClientUI::GetBoldFont());
    AttachChild(m_allocation_label);
    top += row_height;
    UpdateAllocation(top);


    import_export_label_top = top;
    m_import_export_label = GG::Wnd::Create<CUILabel>("", GG::FORMAT_RIGHT);
    m_import_export_label->MoveTo(GG::Pt(GG::X0, import_export_label_top));
    m_import_export_label->Resize(GG::Pt(TOTAL_WIDTH - EDGE_PAD, row_height));
    m_import_export_label->SetFont(ClientUI::GetBoldFont());
    AttachChild(m_import_export_label);
    top += row_height;
    UpdateImportExport(top);


    Resize(GG::Pt(LabelWidth() + ValueWidth(), top));
}

void SystemResourceSummaryBrowseWnd::UpdateProduction(GG::Y& top) {
    // adds pairs of labels for ResourceCenter name and production of resource starting at vertical position \a top
    // and updates \a top to the vertical position after the last entry
    for (const auto& label_pair : m_production_labels_and_amounts) {
        DetachChild(label_pair.first);
        DetachChild(label_pair.second);
    }
    m_production_labels_and_amounts.clear();

    auto system = GetSystem(m_system_id);
    if (!system || m_resource_type == INVALID_RESOURCE_TYPE)
        return;


    m_production = 0.0;


    // add label-value pair for each resource-producing object in system to indicate amount of resource produced
    auto objects = Objects().FindObjects<const UniverseObject>(system->ContainedObjectIDs());

    for (auto& obj : objects) {
        // display information only for the requested player
        if (m_empire_id != ALL_EMPIRES && !obj->OwnedBy(m_empire_id))
            continue;   // if m_empire_id == -1, display resource production for all empires.  otherwise, skip this resource production if it's not owned by the requested player

        auto rc = std::dynamic_pointer_cast<const ResourceCenter>(obj);
        if (!rc) continue;

        std::string name = obj->Name();
        double production = rc->InitialMeterValue(ResourceToMeter(m_resource_type));
        m_production += production;

        std::string amount_text = DoubleToString(production, 3, false);


        auto label = GG::Wnd::Create<CUILabel>(name, GG::FORMAT_RIGHT);
        label->MoveTo(GG::Pt(GG::X0, top));
        label->Resize(GG::Pt(LabelWidth(), row_height));
        AttachChild(label);

        auto value = GG::Wnd::Create<CUILabel>(amount_text);
        value->MoveTo(GG::Pt(LabelWidth(), top));
        value->Resize(GG::Pt(ValueWidth(), row_height));
        AttachChild(value);

        m_production_labels_and_amounts.push_back({label, value});

        top += row_height;
    }


    if (m_production_labels_and_amounts.empty()) {
        // add "blank" line to indicate no production
        auto label = GG::Wnd::Create<CUILabel>(UserString("NOT_APPLICABLE"));
        label->MoveTo(GG::Pt(GG::X0, top));
        label->Resize(GG::Pt(LabelWidth(), row_height));
        AttachChild(label);

        auto value = GG::Wnd::Create<CUILabel>("");
        value->MoveTo(GG::Pt(LabelWidth(), top));
        value->Resize(GG::Pt(ValueWidth(), row_height));
        AttachChild(value);

        m_production_labels_and_amounts.push_back({label, value});

        top += row_height;
    }


    // set production label
    std::string resource_text = "";
    switch (m_resource_type) {
    case RE_INDUSTRY:
        resource_text = UserString("INDUSTRY_PRODUCTION");  break;
    case RE_RESEARCH:
        resource_text = UserString("RESEARCH_PRODUCTION");  break;
    case RE_TRADE:
        resource_text = UserString("TRADE_PRODUCTION");     break;
    case RE_STOCKPILE:
        resource_text = UserString("STOCKPILE_GENERATION"); break;
    default:
        resource_text = UserString("UNKNOWN_VALUE_SYMBOL"); break;
    }

    m_production_label->SetText(boost::io::str(FlexibleFormat(UserString("RESOURCE_PRODUCTION_TOOLTIP")) %
                                                              resource_text %
                                                              DoubleToString(m_production, 3, false)));

    // height of label already added to top outside this function
}

void SystemResourceSummaryBrowseWnd::UpdateAllocation(GG::Y& top) {
    // adds pairs of labels for allocation of resources in system, starting at vertical position \a top and
    // updates \a top to be the vertical position after the last entry
    for (const auto& label_pair : m_allocation_labels_and_amounts) {
        DetachChild(label_pair.first);
        DetachChild(label_pair.second);
    }
    m_allocation_labels_and_amounts.clear();

    auto system = GetSystem(m_system_id);
    if (!system || m_resource_type == INVALID_RESOURCE_TYPE)
        return;


    m_allocation = 0.0;


    // add label-value pair for each resource-consuming object in system to indicate amount of resource consumed
    for (auto& obj : Objects().FindObjects<const UniverseObject>(system->ContainedObjectIDs())) {
        // display information only for the requested player
        if (m_empire_id != ALL_EMPIRES && !obj->OwnedBy(m_empire_id))
            continue;   // if m_empire_id == ALL_EMPIRES, display resource production for all empires.  otherwise, skip this resource production if it's not owned by the requested player


        std::string name = obj->Name();


        double allocation = ObjectResourceConsumption(obj, m_resource_type, m_empire_id);


        // don't add summary entries for objects that consume no resource.  (otherwise there would be a loooong pointless list of 0's
        if (allocation <= 0.0) {
            if (allocation < 0.0)
                ErrorLogger() << "object " << obj->Name() << " is reported having negative " << boost::lexical_cast<std::string>(m_resource_type) << " consumption";
            continue;
        }


        m_allocation += allocation;

        std::string amount_text = DoubleToString(allocation, 3, false);


        auto label = GG::Wnd::Create<CUILabel>(name, GG::FORMAT_RIGHT);
        label->MoveTo(GG::Pt(GG::X0, top));
        label->Resize(GG::Pt(LabelWidth(), row_height));
        AttachChild(label);


        auto value = GG::Wnd::Create<CUILabel>(amount_text);
        value->MoveTo(GG::Pt(LabelWidth(), top));
        value->Resize(GG::Pt(ValueWidth(), row_height));
        AttachChild(value);

        m_allocation_labels_and_amounts.push_back({label, value});

        top += row_height;
    }


    if (m_allocation_labels_and_amounts.empty()) {
        // add "blank" line to indicate no allocation
        auto label = GG::Wnd::Create<CUILabel>(UserString("NOT_APPLICABLE"), GG::FORMAT_RIGHT);
        label->MoveTo(GG::Pt(GG::X0, top));
        label->Resize(GG::Pt(LabelWidth(), row_height));
        AttachChild(label);

        auto value = GG::Wnd::Create<CUILabel>("");
        value->MoveTo(GG::Pt(LabelWidth(), top));
        value->Resize(GG::Pt(ValueWidth(), row_height));
        AttachChild(value);

        m_allocation_labels_and_amounts.push_back({label, value});

        top += row_height;
    }


    // set consumption / allocation label
    std::string resource_text = "";
    switch (m_resource_type) {
    case RE_INDUSTRY:
        resource_text = UserString("INDUSTRY_CONSUMPTION"); break;
    case RE_RESEARCH:
        resource_text = UserString("RESEARCH_CONSUMPTION"); break;
    case RE_TRADE:
        resource_text = UserString("TRADE_CONSUMPTION");    break;
    case RE_STOCKPILE:
        resource_text = UserString("STOCKPILE_USE");        break;
    default:
        resource_text = UserString("UNKNOWN_VALUE_SYMBOL"); break;
    }

    std::string system_allocation_text = DoubleToString(m_allocation, 3, false);

    // for research and stockpiling, local allocation makes no sense
    if (m_resource_type == RE_RESEARCH && m_allocation == 0.0)
        system_allocation_text = UserString("NOT_APPLICABLE");
    if (m_resource_type == RE_STOCKPILE && m_allocation == 0.0)
        system_allocation_text = UserString("NOT_APPLICABLE");


    m_allocation_label->SetText(boost::io::str(FlexibleFormat(UserString("RESOURCE_ALLOCATION_TOOLTIP")) %
                                                              resource_text %
                                                              system_allocation_text));

    // height of label already added to top outside this function
}

void SystemResourceSummaryBrowseWnd::UpdateImportExport(GG::Y& top) {
    m_import_export_label->SetText(UserString("IMPORT_EXPORT_TOOLTIP"));

    const Empire* empire = nullptr;

    // check for early exit cases...
    bool abort = false;
    if (m_empire_id == ALL_EMPIRES ||
        m_resource_type == RE_RESEARCH ||
        m_resource_type == RE_STOCKPILE)
    {
        // multiple empires have complicated stockpiling which don't make sense to try to display.
        // Research use is nonlocalized, so importing / exporting doesn't make sense to display
        abort = true;
    } else {
        empire = GetEmpire(m_empire_id);
        if (!empire)
            abort = true;
    }


    std::string label_text = "", amount_text = "";


    if (!abort) {
        double difference = m_production - m_allocation;

        switch (m_resource_type) {
        case RE_TRADE:
        case RE_INDUSTRY:
            if (difference > 0.0) {
                // show surplus
                label_text = UserString("RESOURCE_EXPORT");
                amount_text = DoubleToString(difference, 3, false);
            } else if (difference < 0.0) {
                // show amount being imported
                label_text = UserString("RESOURCE_IMPORT");
                amount_text = DoubleToString(std::abs(difference), 3, false);
            } else {
                // show self-sufficiency
                label_text = UserString("RESOURCE_SELF_SUFFICIENT");
                amount_text = "";
            }
            break;
        case RE_RESEARCH:
        case RE_STOCKPILE:
        default:
            // show nothing
            abort = true;
            break;
        }
    }


    if (abort) {
        label_text = UserString("NOT_APPLICABLE");
        amount_text = "";   // no change
    }


    // add label and amount.  may be "NOT APPLIABLE" and nothing if aborted above
    auto label = GG::Wnd::Create<CUILabel>(label_text, GG::FORMAT_RIGHT);
    label->MoveTo(GG::Pt(GG::X0, top));
    label->Resize(GG::Pt(LabelWidth(), row_height));
    AttachChild(label);

    auto value = GG::Wnd::Create<CUILabel>(amount_text);
    value->MoveTo(GG::Pt(LabelWidth(), top));
    value->Resize(GG::Pt(ValueWidth(), row_height));
    AttachChild(value);

    m_import_export_labels_and_amounts.push_back({label, value});

    top += row_height;
}

void SystemResourceSummaryBrowseWnd::Clear() {
    DetachChildAndReset(m_production_label);
    DetachChildAndReset(m_allocation_label);
    DetachChildAndReset(m_import_export_label);

    for (const auto& label_pair : m_production_labels_and_amounts) {
        DetachChild(label_pair.first);
        DetachChild(label_pair.second);
    }
    m_production_labels_and_amounts.clear();

    for (const auto& label_pair : m_allocation_labels_and_amounts) {
        DetachChild(label_pair.first);
        DetachChild(label_pair.second);
    }
    m_allocation_labels_and_amounts.clear();

    for (const auto& label_pair : m_import_export_labels_and_amounts) {
        DetachChild(label_pair.first);
        DetachChild(label_pair.second);
    }
    m_import_export_labels_and_amounts.clear();
}
