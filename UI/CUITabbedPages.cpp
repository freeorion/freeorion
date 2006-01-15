//CUITabbedPages.cpp

#include "CUITabbedPages.h"

#include "../util/MultiplayerCommon.h"

namespace {
    bool temp_header_bool = RecordHeaderFile(CUITabbedPagesRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");

	const int INNER_BUTTON_THICKNESS = 5;
	const int TABCONTROL_XOFFSET = 5;
}

////////////////////////////////////////////////
// CUITabControl::TabButton
////////////////////////////////////////////////
CUITabControl::TabButton::TabButton(int x, int y, const std::string& str) :
    CUIButton(x, y, 5, str),
    m_selected(false)
{
	if (GetFont()) {
		GG::Pt text_sz = GetFont()->TextExtent(WindowText(), GG::TF_LEFT | GG::TF_VCENTER, Width());
		text_sz.x += 2 * (BorderThickness() + INNER_BUTTON_THICKNESS);
		text_sz.y = Height();
		Resize(text_sz);
	}
}

bool CUITabControl::TabButton::IsSelected() const
{
    return m_selected;
}

void CUITabControl::TabButton::Render()
{
    switch (State())
    {
    case BN_PRESSED:
        RenderPressed();
        break;
    case BN_UNPRESSED:
        if (m_selected)
	        RenderRollover();
		else
			RenderUnpressed();
		break;
    case BN_ROLLOVER:
        RenderRollover();
        break;
    }
}

void CUITabControl::TabButton::SetSelected(bool bSelected/* = true*/)
{
    m_selected = bSelected;
}


////////////////////////////////////////////////
// CUITabControl::ScrollButton
////////////////////////////////////////////////
CUITabControl::ScrollButton::ScrollButton(int x, int y, int w, int h, ShapeOrientation orientation, GG::Clr color) :
    CUIArrowButton(x, y, w, h, orientation, color)
{}

////////////////////////////////////////////////
// CUITabControl::TabClickedFunctor
////////////////////////////////////////////////
CUITabControl::TabClickedFunctor::TabClickedFunctor(int index_, CUITabControl* wnd_) :
    index(index_),
    wnd(wnd_)
{}

void CUITabControl::TabClickedFunctor::operator()()
{
    wnd->TabClicked(index);
}

////////////////////////////////////////////////
// CUITabControl
////////////////////////////////////////////////
CUITabControl::CUITabControl(int x, int y, int w) : 
	GG::Wnd(x, y, w, 5, 0),
	m_container(new TabContainer(x, y, w, 5)),
	m_scrollLeft(new ScrollButton(0, 0, 1, 1, SHAPE_LEFT, ClientUI::DROP_DOWN_LIST_ARROW_COLOR)),
	m_scrollRight(new ScrollButton(0, 0, 1, 1, SHAPE_RIGHT, ClientUI::DROP_DOWN_LIST_ARROW_COLOR)),
    m_first_tab_shown(0),
    m_current_tab(-1)
{
	AttachChild(m_container);
	AttachChild(m_scrollLeft);
	AttachChild(m_scrollRight);
	m_scrollLeft->Hide();
	m_scrollRight->Hide();
	m_container->EnableChildClipping();

	GG::Connect(m_scrollLeft->ClickedSignal, &CUITabControl::OnScrollLeft, this);
	GG::Connect(m_scrollRight->ClickedSignal, &CUITabControl::OnScrollRight, this);
}

void CUITabControl::AddTab(const std::string& title)
{
	// Find position for the button
	int pos = 0;
	if (!m_buttons.empty())
		pos += (*m_buttons.rbegin())->LowerRight().x - m_container->UpperLeft().x + TABCONTROL_XOFFSET;
	// Create a new tab button
	TabButton* button = new TabButton(pos, 0, title);
	// Set height of control according to the height of the font:
	if (m_buttons.empty()) {
		Resize(GG::Pt(Width(), button->Height()));
		m_container->Resize(GG::Pt(Width(), button->Height()));
	}
	// Add the new button
	m_buttons.push_back(button);
	m_container->AttachChild(button);
	GG::Connect(button->ClickedSignal, TabClickedFunctor(m_buttons.size() - 1, this));

	// Show the left/right scroll buttons, if the width is too great:
	if ((button->LowerRight().x > Width()) && !m_scrollRight->Visible()) {
		int width = button->Height() / 2;
		int pos = Width() - width - 2;
		m_scrollRight->SizeMove(GG::Pt(pos, 0), GG::Pt(pos + width, button->Height()));
		pos -= width + TABCONTROL_XOFFSET;
		m_scrollLeft->SizeMove(GG::Pt(pos, 0), GG::Pt(pos + width, button->Height()));
        m_container->SizeMove(GG::Pt(0, 0), GG::Pt(pos - TABCONTROL_XOFFSET, Height()));
		m_scrollLeft->Show();
		m_scrollRight->Show();
	}

	if (m_buttons.size() == 1)
		button->SetSelected();
}

void CUITabControl::OnScrollRight()
{
    if (m_first_tab_shown < static_cast<int>(m_buttons.size()) - 1) {
        int offset = m_buttons[m_first_tab_shown + 1]->UpperLeft().x - m_buttons[m_first_tab_shown]->UpperLeft().x;
        for (std::vector<TabButton*>::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it) {
			(*it)->OffsetMove(GG::Pt(-offset, 0));
        }
        ++m_first_tab_shown;
    }
}

void CUITabControl::OnScrollLeft()
{
    if (0 < m_first_tab_shown) {
        int offset = m_buttons[m_first_tab_shown]->UpperLeft().x - m_buttons[m_first_tab_shown - 1]->UpperLeft().x;
        for (std::vector<TabButton*>::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it) {
			(*it)->OffsetMove(GG::Pt(offset, 0));
        }
        --m_first_tab_shown;
    }
}

void CUITabControl::SelectTab(int n)
{
	if (n < 0 || static_cast<int>(m_buttons.size()) <= n)
		return;
    TabClicked(n);
}

void CUITabControl::TabClicked(int n)
{
    if (n == m_current_tab)
        return;
    int previous_tab = m_current_tab;
    m_current_tab = n;
    if (previous_tab != -1)
        m_buttons[previous_tab]->SetSelected(false);
    m_buttons[m_current_tab]->SetSelected(true);
    TabClickedSignal(m_current_tab, previous_tab);
}


////////////////////////////////////////////////
// CUITabbedPages
////////////////////////////////////////////////
CUITabbedPages::CUITabbedPages(int x, int y, int w, int h) :
    GG::Wnd(x, y, w, h, 0)
{
	m_tabs = new CUITabControl(0, 0, w);
	AttachChild(m_tabs);
	GG::Connect(m_tabs->TabClickedSignal, &CUITabbedPages::OnTabSelected, this);
}

void CUITabbedPages::AddPage(GG::Wnd* page, const std::string& title)
{
	m_tabs->AddTab(title);
	m_pages.push_back(page);
	AttachChild(page);
	page->SizeMove(GG::Pt(0, m_tabs->Height()), GG::Pt(Width(), Height() - m_tabs->Height()));
	if (m_pages.size() == 1)
		m_tabs->SelectTab(0);
    else
        page->Hide();
}

void CUITabbedPages::OnTabSelected(int tab_new, int tab_old)
{
	if (tab_old != -1)
		m_pages[tab_old]->Hide();
	m_pages[tab_new]->Show();
}
