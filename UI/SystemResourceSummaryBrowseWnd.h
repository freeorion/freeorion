// -*- C++ -*-
#ifndef _SystemResourceSummaryBrowseWnd_h_
#define _SystemResourceSummaryBrowseWnd_h_

#include <GG/BrowseInfoWnd.h>

#include "CUIControls.h"

/** Gives information about inporting and exporting of resources to and from this system when mousing
  * over the system resource production summary. */
class SystemResourceSummaryBrowseWnd : public GG::BrowseInfoWnd {
public:
    SystemResourceSummaryBrowseWnd(ResourceType resource_type, int system_id, int empire_id = ALL_EMPIRES);

    virtual bool    WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const;

    virtual void    Render();

private:
    void            Clear();
    void            Initialize();
    virtual void    UpdateImpl(std::size_t mode, const GG::Wnd* target);

    void            UpdateProduction(GG::Y& top);   // adds pairs of labels for ResourceCenter name and production of resource starting at vertical position \a top and updates \a top to the vertical position after the last entry
    void            UpdateAllocation(GG::Y& top);   // adds pairs of labels for allocation of resources in system, starting at vertical position \a top and updates \a top to be the vertical position after the last entry
    void            UpdateImportExport(GG::Y& top); // sets m_import_export_label and m_import_export text and amount to indicate how much resource is being imported or exported from this system, and moves them to vertical position \a top and updates \a top to be the vertical position below these labels

    ResourceType        m_resource_type;
    int                 m_system_id;
    int                 m_empire_id;

    double              m_production;               // set by UpdateProduction - used to store production in system so that import / export / unused can be more easily calculated
    double              m_allocation;               // set by UpdateAllocation - used like m_production

    CUILabel*           m_production_label;
    CUILabel*           m_allocation_label;
    CUILabel*           m_import_export_label;

    std::vector<std::pair<CUILabel*, CUILabel*> > m_production_labels_and_amounts;
    std::vector<std::pair<CUILabel*, CUILabel*> > m_allocation_labels_and_amounts;
    std::vector<std::pair<CUILabel*, CUILabel*> > m_import_export_labels_and_amounts;

    GG::Y               row_height;

    GG::Y               production_label_top;
    GG::Y               allocation_label_top;
    GG::Y               import_export_label_top;
};

#endif
