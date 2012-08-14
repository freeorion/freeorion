// -*- C++ -*-
#ifndef _ObjectListWnd_h_
#define _ObjectListWnd_h_

#include "CUIWnd.h"

#include <GG/ListBox.h>

class ObjectListBox;

class ObjectListWnd : public CUIWnd {
public:
    //! \name Structors //!@{
    ObjectListWnd(GG::X w, GG::Y h);
    //!@}

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void            Update();
    //!@}

    mutable boost::signal<void (int)>   ObjectDoubleClickedSignal;
    mutable boost::signal<void (int)>   ObjectDumpSignal;

private:
    void            DoLayout();

    void            ObjectDoubleClicked(GG::ListBox::iterator it);
    void            ObjectRightClicked(GG::ListBox::iterator it, const GG::Pt& pt);
    int             ObjectInRow(GG::ListBox::iterator it) const;

    ObjectListBox*  m_list_box;
};

#endif // _ObjectListWnd_h_
