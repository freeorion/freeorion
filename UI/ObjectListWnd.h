// -*- C++ -*-
#ifndef _ObjectListWnd_h_
#define _ObjectListWnd_h_

#include "CUIWnd.h"

#include <GG/GGFwd.h>
#include <GG/ListBox.h>

class ObjectListBox;


class ObjectListWnd : public CUIWnd {
public:
    //! \name Structors //!@{
    ObjectListWnd(GG::X default_x, GG::Y default_y,
                  GG::X default_w, GG::Y default_h,
                  const std::string& config_name = "");
    //!@}

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void            Refresh();
    //!@}

    mutable boost::signals2::signal<void ()>    SelectedObjectsChangedSignal;
    mutable boost::signals2::signal<void (int)> ObjectDoubleClickedSignal;
    mutable boost::signals2::signal<void (int)> ObjectDumpSignal;
    mutable boost::signals2::signal<void ()>    ClosingSignal;

private:
    void            DoLayout();

    void            ObjectSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            ObjectDoubleClicked(GG::ListBox::iterator it);
    void            ObjectRightClicked(GG::ListBox::iterator it, const GG::Pt& pt);
    int             ObjectInRow(GG::ListBox::iterator it) const;

    void            SetSelectedObjects(std::set<int> sel_ids);
    std::set<int>   SelectedObjectIDs() const;

    void            FilterClicked();
    void            CollapseExpandClicked();
    virtual void    CloseClicked();

    ObjectListBox*              m_list_box;
    GG::Button*                 m_filter_button;
    GG::Button*                 m_collapse_button;
};

#endif // _ObjectListWnd_h_
