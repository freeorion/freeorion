#ifndef _ObjectListWnd_h_
#define _ObjectListWnd_h_

#include "CUIWnd.h"

#include <GG/GGFwd.h>
#include <GG/ListBox.h>

class ObjectListBox;


class ObjectListWnd : public CUIWnd {
public:
    ObjectListWnd(std::string_view config_name = "");
    void CompleteConstruction() override;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void Refresh();

    mutable boost::signals2::signal<void ()>    SelectedObjectsChangedSignal;
    mutable boost::signals2::signal<void (int)> ObjectDoubleClickedSignal;
    mutable boost::signals2::signal<void (int)> ObjectDumpSignal;
    mutable boost::signals2::signal<void ()>    ClosingSignal;

private:
    void            CloseClicked() override;

    void            DoLayout();

    void            ObjectSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            ObjectDoubleClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    void            ObjectRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    int             ObjectInRow(GG::ListBox::iterator it) const;

    void            SetSelectedObjects(std::set<int> sel_ids);
    std::set<int>   SelectedObjectIDs() const;

    void            FilterClicked();
    void            CollapseExpandClicked();

    std::shared_ptr<ObjectListBox>  m_list_box;
    std::shared_ptr<GG::Button>     m_filter_button;
    std::shared_ptr<GG::Button>     m_collapse_button;
};


#endif
