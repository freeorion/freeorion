//ToolContainer.cpp

#include "ToolContainer.h"

#include "GGApp.h"
#include "../util/MultiplayerCommon.h"


namespace {
    bool temp_header_bool = RecordHeaderFile(ToolContainerRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


ToolContainer::ToolContainer(int delay)
{
 
    startTime=1;
    currentTime=0;
    delayTime=delay;
    
    timerID=SDL_AddTimer(delayTime, &ShowCallback, this);
   
}

ToolContainer::~ToolContainer()
{
}

bool ToolContainer::AttachToolWnd(GG::Wnd* parent, ToolWnd* tool)
{
    //!this function attaches the toolwnd as a child to the parent
    //!it also adds it to the map
    
    if(!parent)
    {
        //we got a bad pointer, get out
        return false;
    }
    
    //add as a child
    parent->AttachChild(tool);
    
    //add to the map...will overwrite if it exists
    toolList[parent]=tool;    //if its NULL its the same as erasing
    //all clear, return true
    return true;
}

Uint32 ToolContainer::ShowCallback(Uint32 interval, void* param)
{
    static GG::Wnd* tooltarget=NULL;
    ToolContainer* container=(ToolContainer*)param;
    
    //find the window that is under the mouse
    //if it is in our list, make the toolwindow visible
    GG::Wnd* wnd = GG::App::GetApp()->GetWindowUnder(GG::App::GetApp()->MousePosition());
    std::map<GG::Wnd*, ToolWnd*>::iterator pos;
    
    if(wnd==tooltarget)
    {
        return interval;
    }
    
    if( (pos=container->toolList.find(tooltarget)) != container->toolList.end())
    {
        //we found the window
        pos->second->Hide();
    }
    
    if(wnd!=NULL)
    {     
        //find the old window and disable that toolwindow

        //keep going if window is valid
        if( (pos=container->toolList.find(wnd)) != container->toolList.end())
        {
            //we found the window
            pos->second->Show();
        }
 
    }

    tooltarget=wnd; 
    return interval;
}
