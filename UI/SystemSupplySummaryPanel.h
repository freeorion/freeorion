#ifndef _SystemSupplySummaryPanel_h_
#define _SystemSupplySummaryPanel_h_

#include <GG/Wnd.h>

#include "../universe/EnumsFwd.h"

/** Display the supply icon a popup with the system supply details. */
class SystemSupplySummaryPanel : public GG::Wnd {
public:
    /**Use \p system_id to display supply details.*/
    SystemSupplySummaryPanel(const int system_id);
    ~SystemSupplySummaryPanel();

    void Render() override;

    /** Update the data for the popup.*/
    void            Update();

    /**Return true if the system_id is mapped to a System.*/
    bool            IsEmpty();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif
