#ifndef Dialog_h
#define Dialog_h

#include <GG/Wnd.h>
#include <GG/FontFwd.h>


//! A Simple resizable dialog with a single child that fills it.
class Dialog: public GG::Wnd{
public:
    Dialog(std::shared_ptr<GG::Wnd> child, const std::shared_ptr<GG::Font>& font);

    void CompleteConstruction() override;
    void Render() override;

    GG::Pt ClientLowerRight() const override;

    void SizeMove( const GG::Pt& ul, const GG::Pt& lr) override;

private:
    std::shared_ptr<GG::Wnd> m_child;
};

#endif // Dialog_h
