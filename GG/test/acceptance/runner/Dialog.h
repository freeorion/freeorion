#ifndef Dialog_h
#define Dialog_h

#include <GG/Wnd.h>
#include <GG/FontFwd.h>


//! A Simple resizable dialog with a single child that fills it.
class Dialog: public GG::Wnd{
public:
    Dialog(GG::Wnd* child, const std::shared_ptr<GG::Font>& font);

    virtual void Render();

    virtual GG::Pt ClientLowerRight() const;

    virtual void SizeMove( const GG::Pt& ul, const GG::Pt& lr);
};

#endif // Dialog_h
