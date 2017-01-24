#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <GG/Wnd.h>
#include <GG/FontFwd.h>

//! A Simple resizable dialog with a single child that fills it.
class TestDialog: public GG::Wnd{
public:
    TestDialog(GG::Wnd* child, const std::shared_ptr<GG::Font>& font);

    virtual void Render();

    virtual GG::Pt ClientLowerRight() const;

    virtual void SizeMove( const GG::Pt& ul, const GG::Pt& lr);
};

#endif // TESTDIALOG_H
