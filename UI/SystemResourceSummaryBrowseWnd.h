#ifndef _SystemResourceSummaryBrowseWnd_h_
#define _SystemResourceSummaryBrowseWnd_h_

#include <GG/GGFwd.h>
#include <GG/BrowseInfoWnd.h>

#include "../universe/ConstantsFwd.h"
#include "../universe/EnumsFwd.h"
#include "../util/Export.h"

/** Gives information about inporting and exporting of resources to and from this system when mousing
  * over the system resource production summary. */
class SystemResourceSummaryBrowseWnd : public GG::BrowseInfoWnd {
public:
    SystemResourceSummaryBrowseWnd(ResourceType resource_type, int system_id,
                                   int empire_id = ALL_EMPIRES);

    bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override;
    void Render() override;

private:
    void UpdateImpl(std::size_t mode, const GG::Wnd* target) override;
    void Clear();
    void Initialize();

    void UpdateProduction(GG::Y& top);  // adds pairs of labels for Planet name and production of resource starting at vertical position \a top and updates \a top to the vertical position after the last entry
    void UpdateAllocation(GG::Y& top);  // adds pairs of labels for allocation of resources in system, starting at vertical position \a top and updates \a top to be the vertical position after the last entry
    void UpdateImportExport(GG::Y& top);// sets m_import_export_label and m_import_export text and amount to indicate how much resource is being imported or exported from this system, and moves them to vertical position \a top and updates \a top to be the vertical position below these labels

    ResourceType    m_resource_type{0};
    int             m_system_id = INVALID_OBJECT_ID;
    int             m_empire_id = ALL_EMPIRES;
    double          m_production = 0.0; // set by UpdateProduction - used to store production in system so that import / export / unused can be more easily calculated
    double          m_allocation = 0.0; // set by UpdateAllocation - used like m_production

    std::shared_ptr<GG::Label> m_production_label;
    std::shared_ptr<GG::Label> m_allocation_label;
    std::shared_ptr<GG::Label> m_import_export_label;

    std::vector<std::pair<std::shared_ptr<GG::Label>, std::shared_ptr<GG::Label>>> m_production_labels_and_amounts;
    std::vector<std::pair<std::shared_ptr<GG::Label>, std::shared_ptr<GG::Label>>> m_allocation_labels_and_amounts;
    std::vector<std::pair<std::shared_ptr<GG::Label>, std::shared_ptr<GG::Label>>> m_import_export_labels_and_amounts;

    GG::Y row_height = GG::Y1;
    GG::Y production_label_top = GG::Y0;
    GG::Y allocation_label_top = GG::Y0;
    GG::Y import_export_label_top = GG::Y0;
};

#endif
