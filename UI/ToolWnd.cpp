//ToolWnd.cpp

#include "GGDrawUtil.h"
#include "../util/MultiplayerCommon.h"
#include "ToolWnd.h"


namespace {
    bool temp_header_bool = RecordHeaderFile(ToolWndRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


//ToolWnd::ToolWnd(int x, int y, char* text, GG::Clr clr):
ToolWnd::ToolWnd(int x, int y, const std::string& text, const GG::Clr& clr, const std::string& font_name /* = "arial.ttf"*/, int pts /*= 10*/):
    GG::Wnd(x,y,10,10,0)
{
    textwnd=new GG::TextControl(4,4,text.c_str(),font_name,pts);
    color=clr;
    //resize this window to match the size of the text field
    Resize(textwnd->GG::Wnd::Width()+8,textwnd->GG::Wnd::Height()+8);
    AttachChild(textwnd);
    Hide(); 
}

bool ToolWnd::Render()
{
    GG::FlatRectangle(UpperLeft().x,UpperLeft().y,LowerRight().x,LowerRight().y,color,GG::CLR_BLACK,1);
    //draw a wire rectangle around the text 1 pixel away
    GG::FlatRectangle(UpperLeft().x+2,UpperLeft().y+2,LowerRight().x-2,LowerRight().y-2,GG::CLR_ZERO,GG::CLR_BLACK);
    return true;
}


