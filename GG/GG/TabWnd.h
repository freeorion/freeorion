//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2006 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/TabWnd.h
//!
//! Contains the TabWnd class, which encapsulates a set of tabbed windows.

#ifndef _GG_TabWnd_h_
#define _GG_TabWnd_h_


#include <GG/Button.h>


namespace GG {

class TabBar;
class WndEvent;

/** \brief Contains several Wnds, and only displays the Wnd currently
    specified. */
class GG_API OverlayWnd : public Wnd
{
public:
    OverlayWnd(X x, Y y, X w, Y h, Flags<WndFlag> flags = NO_WND_FLAGS);
    ~OverlayWnd() = default;
    void CompleteConstruction() override;

    Pt MinUsableSize() const override;

    /** Returns true iff NumWnds() == 0. */
    bool Empty() const;

    /** Returns the number of Wnds currently in this OverlayWnd. */
    std::size_t NumWnds() const;

    /** Returns the Wnd currently visible in the OverlayWnd, or 0 if there is none. */
    std::shared_ptr<Wnd> CurrentWnd() const;

    /** Returns the index into the sequence of Wnds in this OverlayWnd of the Wnd
        currently shown.  NO_WND is returned if there is no Wnd currently
        visible. */
    std::size_t  CurrentWndIndex() const;

    /** Adds \a wnd to the sequence of Wnds in this OverlayWnd, with name \a name.
        \a name can be used later to remove the Wnd (\a name is not checked
        for uniqueness).  Returns the index at which \a wnd is placed. */
    std::size_t AddWnd(std::shared_ptr<Wnd> wnd);

    /** Adds \a wnd to the sequence of Wnds in this OverlayWnd, inserting it at
        the \a index location within the sequence.  \a name can be used later
        to remove the Wnd (\a name is not checked for uniqueness).  Not range
        checked. */
    void InsertWnd(std::size_t index, std::shared_ptr<Wnd> wnd);

    /** Removes and returns the Wnd at index \a index from the sequence of
        Wnds in this OverlayWnd, or 0 if there is no Wnd at that index. */
    Wnd* RemoveWnd(std::size_t index);

    /** Removes and returns \a wnd from the sequence of Wnds in this
        OverlayWnd, or 0 if there is no such Wnd in this OverlayWnd. */
    Wnd* RemoveWnd(Wnd* wnd);

    /** Sets the currently visible Wnd in the sequence to the Wnd in the \a
        index position within the sequence.  Not range checked. */
    void SetCurrentWnd(std::size_t index);

    /** The invalid Wnd position index that there is no currently-selected
        Wnd. */
    static constexpr std::size_t NO_WND = std::numeric_limits<std::size_t>::max();

private:
    std::vector<std::shared_ptr<Wnd>> m_wnds;
    std::size_t                       m_current_wnd_index = NO_WND;
};


/** \brief Contains several Wnds and a TabBar, and only displays the Wnd
    currently selected in the TabBar. */
class GG_API TabWnd : public Wnd
{
public:
    /** Emitted when the currently-selected Wnd has changed; the new selected
        Wnd's index in the group is provided (this may be NO_WND if no Wnd is
        currently selected). */
    typedef boost::signals2::signal<void (std::size_t)> TabChangedSignalType;

    TabWnd(X x, Y y, X w, Y h, const std::shared_ptr<Font>& font,
           Clr color, Clr text_color = CLR_BLACK);
    void CompleteConstruction() override;

    Pt MinUsableSize() const override;

    /** Returns true iff NumWnds() == 0. */
    bool            Empty() const;

    /** Returns the number of tabs currently in this TabWnd. */
    std::size_t     NumWnds() const;

    /** Returns the Wnd currently visible in the TabWnd, or 0 if there is none. */
    Wnd*            CurrentWnd() const;

    /** Returns the index into the sequence of Wnds in this TabWnd of the Wnd
        currently shown.  NO_WND is returned if there is no Wnd currently
        visible. */
    std::size_t     CurrentWndIndex() const;

    /** Adds \a wnd to the sequence of Wnds in this TabWnd, with name \a name.
        \a name can be used later to remove the Wnd (\a name is not checked
        for uniqueness).  Returns the index at which \a wnd is placed. */
    std::size_t     AddWnd(std::shared_ptr<Wnd> wnd, std::string name);

    /** Adds \a wnd to the sequence of Wnds in this TabWnd, inserting it at
        the \a index location within the sequence.  \a name can be used later
        to remove the Wnd (\a name is not checked for uniqueness).  Not range
        checked. */
    void            InsertWnd(std::size_t index, std::shared_ptr<Wnd> wnd, std::string name);

    /** Removes and returns the first Wnd previously added witht he name \a
        name from the sequence of Wnds in this TabWnd, or 0 if no such Wnd is
        found. */
    Wnd*            RemoveWnd(const std::string& name);

    /** Sets the currently visible Wnd in the sequence to the Wnd in the \a
        index position within the sequence.  Not range checked. */
    void            SetCurrentWnd(std::size_t index);

    mutable TabChangedSignalType TabChangedSignal; ///< The Wnd change signal object for this TabWnd

    /** The invalid Wnd position index that there is no currently-selected
        Wnd. */
    static constexpr std::size_t NO_WND = std::numeric_limits<std::size_t>::max();

protected:
    /** Returns the set of Wnds currently controlled by this TabWnd, indexed
        by name. */
    const std::map<std::string, Wnd*>&  WndNames() const;

private:
    void TabChanged(std::size_t tab_index, bool signal);

    std::shared_ptr<TabBar>     m_tab_bar;
    std::shared_ptr<OverlayWnd> m_overlay;
    std::map<std::string, Wnd*> m_named_wnds;
};


/** \brief Contains a sequence of buttons (hereafter "tabs") that act together
    in a RadioButtonGroup.

    This class is intended to be used to select the current Wnd in a
    TabWnd. */
class GG_API TabBar : public Control
{
public:
    /** Emitted when the currently-selected tab has changed; the new selected
        tab's index in the group is provided (this may be NO_TAB if no tab is
        currently selected). */
    typedef boost::signals2::signal<void (std::size_t)> TabChangedSignalType;

    TabBar(const std::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
           Flags<WndFlag> flags = INTERACTIVE);
    void CompleteConstruction() override;
public:

    Pt MinUsableSize() const override;

    /** Returns true iff NumWnds() == 0. */
    bool Empty() const;

    /** Returns the number of tabs currently in this TabWnd. */
    std::size_t NumTabs() const;

    /** Returns the index into the sequence of tabs in this TabBar of the tab
        currently selected.  NO_TAB is returned if there is no tab currently
        selected. */
    std::size_t CurrentTabIndex() const;

    /** Returns the color used to render the text in this TabBar. */
    Clr TextColor() const;

    void MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys) override;
    void SizeMove(Pt ul, Pt lr) override;
    void Render() override;

    virtual void DoLayout();

    /** Adds a tab called \a name to the sequence of tabs in this TabBar.  \a
        name can be used later to remove the tab (\a name is not checked for
        uniqueness).  Returns the index at which the tab is placed. */
    std::size_t AddTab(std::string name);

    /** Adds tab to the sequence of tabs in this TabBar, inserting it at the
        \a index location within the sequence.  \a name can be used later to
        remove the tab (\a name is not checked for uniqueness).  Not range
        checked. */
    void InsertTab(std::size_t index, std::string name);

    /** Removes the first tab previously added witht he name \a name from the
        sequence of tab in this TabBar. */
    void RemoveTab(const std::string& name);

    /** Sets the current tab in the sequence to the tab in the \a index
        position within the sequence.  Not range checked. */
    void SetCurrentTab(std::size_t index);

    mutable TabChangedSignalType TabChangedSignal; ///< The tab change signal object for this TabBar

    /** The invalid tab position index that there is no currently-selected
        tab. */
    static constexpr std::size_t NO_TAB = TabWnd::NO_WND;

protected:
    /** The default width to use for the left and right buttons. */
    X ButtonWidth() const;

    const Button*   LeftButton() const;
    const Button*   RightButton() const;

    bool EventFilter(Wnd* w, const WndEvent& event) override;

    /** Brings the currently-selected tab button to the top within the tab
        button group. */
    void RaiseCurrentTabButton();

private:
    virtual void DistinguishCurrentTab(const std::vector<StateButton*>& tab_buttons);

    void TabChanged(std::size_t index, bool signal);
    void LeftClicked();
    void RightClicked();
    void BringTabIntoView(std::size_t index);

    /** Shows or hides the left-right buttons based on whether they are currently needed. */
    void RecalcLeftRightButton();

    std::shared_ptr<RadioButtonGroup>           m_tabs;
    std::vector<std::shared_ptr<StateButton>>   m_tab_buttons;
    std::shared_ptr<Font>                       m_font;
    std::shared_ptr<Button>                     m_left_button;
    std::shared_ptr<Button>                     m_right_button;
    std::shared_ptr<Layout>                     m_left_right_button_layout;
    Flags<TextFormat>                           m_format;
    Clr                                         m_text_color;
    std::size_t                                 m_first_tab_shown = 0;
};

}


#endif
