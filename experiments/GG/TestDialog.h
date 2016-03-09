#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <GG/Wnd.h>
#include <GG/FontFwd.h>

//! A Simple resizable dialog with a single child that fills it.
class TestDialog: public GG::Wnd{
public:
    TestDialog(GG::Wnd* child, const boost::shared_ptr<GG::Font>& font);

    virtual void Render();

    virtual GG::Pt ClientLowerRight() const override;

    virtual void SizeMove( const GG::Pt& ul, const GG::Pt& lr) override;
};

#endif // TESTDIALOG_H
