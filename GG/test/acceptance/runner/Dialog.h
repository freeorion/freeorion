#ifndef Dialog_h
#define Dialog_h

#include <GG/Wnd.h>
#include <GG/FontFwd.h>


//! A Simple resizable dialog with a single child that fills it.
class Dialog: public GG::Wnd{
public:
    Dialog(GG::Wnd* child, const std::shared_ptr<GG::Font>& font);

    void Render() override;

    GG::Pt ClientLowerRight() const override;

    void SizeMove( const GG::Pt& ul, const GG::Pt& lr) override;
};

#endif // Dialog_h
