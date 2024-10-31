#include <GG/GUI.h>
#include <GG/Clr.h>
#include <GG/ClrConstants.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/StyleFactory.h>

#include "runner/Application.h"


int main(int argc, char* argv[])
{
    Application app(argc, argv);

    const std::string message = "Are <i>we</i> Готово yet?"; // That Russian word means "Done", ha.
    const auto charsets = GG::UnicodeCharsetsToRender(message);

    const auto font = GG::GUI::GetGUI()->GetStyleFactory().DefaultFont(12, charsets.data(), charsets.data() + charsets.size());

    auto quit_dlg = GG::Wnd::Create<GG::ThreeButtonDlg>(GG::X(10), GG::Y(10), message, font, GG::CLR_SHADOW,
                                                        GG::CLR_SHADOW, GG::CLR_SHADOW, GG::CLR_WHITE, 1);

    app.Run(quit_dlg);

    return 0;
}
