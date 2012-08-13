#include "ObjectListWnd.h"

#include "ClientUI.h"
#include "../util/MultiplayerCommon.h"

#include <GG/DrawUtil.h>

ObjectListWnd::ObjectListWnd(GG::X w, GG::Y h) :
    CUIWnd(UserString("MAP_BTN_OBJECTS"), GG::X1, GG::Y1, w - 1, h - 1, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE)
{}

void ObjectListWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    //if (old_size != GG::Wnd::Size())
    //    DoLayout();
}

