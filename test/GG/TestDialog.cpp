#include "TestDialog.h"

#include <GG/ClrConstants.h>
#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/Font.h>

TestDialog::TestDialog(GG::Wnd* child, const std::shared_ptr<GG::Font>& font):
    GG::Wnd(GG::X0, GG::Y0, GG::X(300), GG::Y(300), GG::DRAGABLE | GG::RESIZABLE | GG::MODAL | GG::INTERACTIVE) {
    SetLayout(new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 1, 2, 2));
    GetLayout()->SetColumnStretch(0, 1.0);
    //GetLayout()->SetRowStretch(0, 1.0);
    GetLayout()->Add(child, 0 , 0);
}

void TestDialog::Render() {
    GG::Wnd::Render();
    GG::FlatRectangle ( UpperLeft(), LowerRight(),
                        GG::CLR_GRAY, GG::CLR_SHADOW, 1u );
}

GG::Pt TestDialog::ClientLowerRight() const {
    return LowerRight() - GG::Pt(GG::X(10), GG::Y(10) );
}

void TestDialog::SizeMove( const GG::Pt& ul, const GG::Pt& lr) {
    GG::Wnd::SizeMove( ul, lr );
}
