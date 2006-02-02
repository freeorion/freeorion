#include "ToolContainer.h"

#include "GGApp.h"
#include "GGWnd.h"
#include "../util/MultiplayerCommon.h"


namespace {
    bool temp_header_bool = RecordHeaderFile(ToolContainerRevision());
    bool temp_source_bool = RecordSourceFile("$Id$");
}


ToolContainer::ToolContainer(int delay/* = 1000*/) :
    m_timer_id(SDL_AddTimer(delay, &Callback, static_cast<void*>(this)))
{
}

ToolContainer::~ToolContainer()
{
}

bool ToolContainer::AttachToolWnd(GG::Wnd* parent, GG::Wnd* tool_tip)
{
    // this function attaches the tool tip wnd as a child to the parent it also adds it to the map

    if (!parent)
        return false;

    parent->AttachChild(tool_tip);

    if (!tool_tip) // if its NULL its the same as erasing
        m_tool_tip_wnds.erase(parent);
    else // add to the map...will overwrite if it exists
        m_tool_tip_wnds[parent] = tool_tip;

    return true;
}

Uint32 ToolContainer::Callback(Uint32 interval, void* param)
{
    static GG::Wnd* prev_tool_tip_wnd = 0;
    ToolContainer* this_ptr = reinterpret_cast<ToolContainer*>(param);
    
    GG::Wnd* wnd = GG::App::GetApp()->GetWindowUnder(GG::App::GetApp()->MousePosition());
    
    if (wnd == prev_tool_tip_wnd)
        return interval;
    
    // if there is an old tooltip window active, hide it
    std::map<GG::Wnd*, GG::Wnd*>::iterator it = this_ptr->m_tool_tip_wnds.find(prev_tool_tip_wnd);
    if (it != this_ptr->m_tool_tip_wnds.end()) {
        it->second->Hide();
    }

    // if wnd is in our list of tool tip windows, show it
    if ((it = this_ptr->m_tool_tip_wnds.find(wnd)) != this_ptr->m_tool_tip_wnds.end()) {
        it->second->Show();
    }

    prev_tool_tip_wnd = wnd;

    return interval;
}
