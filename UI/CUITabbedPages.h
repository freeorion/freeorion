// -*- C++ -*-
//CUITabbedPages.h
#ifndef _CUITabbedPages_h_
#define _CUITabbedPages_h_

#ifndef _CUIControls_h_
#include "CUIControls.h"
#endif

class CUITabControl : public GG::Wnd
{
public:
	CUITabControl(int x, int y, int w);

    /** \name Signal Types */ //@{
    typedef boost::signal<void (int, int)> TabClickedSignalType; ///< emitted when one of the tab buttons is clicked by the user
    //@}

    /** \name Slot Types */ //@{
    typedef TabClickedSignalType::slot_type TabClickedSlotType; ///< type of functor(s) invoked on a TabClickedSignalType
    //@}

	void	AddTab(const std::string& title);
	void	SelectTab(int tab);

    mutable TabClickedSignalType TabClickedSignal;

protected:
	class TabButton : public CUIButton
	{
	public:
		TabButton(int x, int y, const std::string& str);

		bool			IsSelected() const;

	    virtual bool	Render();
		void			SetSelected(bool bSelected = true);

	protected:
		bool			m_selected;
	};

	class TabContainer : public GG::Wnd
	{
	public:
		TabContainer(int x, int y, int w, int h) : GG::Wnd(x, y, w, h, 0) {};
	};

	class ScrollButton : public CUIArrowButton
	{
	public:
		ScrollButton(int x, int y, int w, int h, ShapeOrientation orientation, GG::Clr color);
	};

	std::vector<TabButton*> m_buttons;
	TabContainer*			m_container;
	ScrollButton*			m_scrollLeft;
    ScrollButton*			m_scrollRight;
    int                     m_first_tab_shown;
    int                     m_current_tab;

	void	OnTabSelected(const TabButton* button);
	void	OnScrollLeft();
	void	OnScrollRight();

private:
    struct TabClickedFunctor
    {
        TabClickedFunctor(int index_, CUITabControl* wnd_);
        void operator()();
        const int index;
        CUITabControl* const wnd;
    };
    void TabClicked(int n);

    friend struct TabClickedFunctor;
};

class CUITabbedPages : public GG::Wnd
{
public:
	CUITabbedPages(int x, int y, int w, int h);

	void		AddPage(GG::Wnd* page, const std::string& title);
	virtual void OnTabSelected(int tab_new, int tab_old);

protected:
	CUITabControl*			m_tabs;
	std::vector<GG::Wnd*>	m_pages;
};

inline std::pair<std::string, std::string> CUITabbedPagesRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _CUITabbedPages_h_
