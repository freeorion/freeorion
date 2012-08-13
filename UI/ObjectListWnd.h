// -*- C++ -*-
#ifndef _ObjectListWnd_h_
#define _ObjectListWnd_h_

#include "CUIWnd.h"

namespace GG {
    class ListBox;
}
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
private:
    void            DoLayout();

    ObjectListBox*  m_list_box;
};

#endif // _ObjectListWnd_h_
