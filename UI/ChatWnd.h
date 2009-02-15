// -*- C++ -*-
#ifndef _ChatWnd_h_
#define _ChatWnd_h_

#include <GG/Wnd.h>

#include <deque>
#include <string>


namespace GG { class MultiEdit; }
class CUIEdit;

class ChatWnd :
    public GG::Wnd
{
public:
    ChatWnd();

    void HandlePlayerChatMessage(const std::string& msg);
    void Clear();
    bool OpenForInput();
    void HideEdit();

    virtual void Render();
    virtual void KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);

    mutable boost::signal<void ()> MessageSentSignal;

private:
    GG::MultiEdit* m_display;
    CUIEdit* m_edit;
    int m_display_show_time;
    std::deque<std::string> m_history;
    int m_history_position;
};

ChatWnd* GetChatWnd();

#endif
