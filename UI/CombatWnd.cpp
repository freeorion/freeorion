#include "CombatWnd.h"

#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "../client/human/HumanClientApp.h"

#include "../combat/Combat.h"

class CombatInfoControl : public GG::Control
{
  public:
    CombatInfoControl(int w, int h,const CombatUpdateMessage &combat_info) 
    : Control(0, 0, w, h, 0),m_combat_info(combat_info)
    {
    }

    void Update(const CombatUpdateMessage &combat_info) {m_combat_info=combat_info;}

    virtual bool Render()
    {
      GG::Pt ul(UpperLeft()),lr(LowerRight());

      boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
      Uint32 format = GG::TF_LEFT | GG::TF_VCENTER;
      std::string text;

      int y=ul.y;
      for(unsigned int i=0;i<m_combat_info.m_opponents.size();i++,y+=font->Height()+2)
      {
        int x = ul.x+5;
        glColor4ubv(ClientUI::TEXT_COLOR.v);
        font->RenderText(x,y,x+50,y+font->Height(),m_combat_info.m_opponents[i].empire, format, 0, true);
        x+=50;

        text= "combat ships "+boost::lexical_cast<std::string>(m_combat_info.m_opponents[i].combat_ships)
             + "("+boost::lexical_cast<std::string>(m_combat_info.m_opponents[i].combat_ships_hitpoints)+")";
        font->RenderText(x,y,x+130,y+font->Height(),text, format, 0, true);
        x+=110;

        text= "non combat ships "+boost::lexical_cast<std::string>(m_combat_info.m_opponents[i].non_combat_ships)
             + "("+boost::lexical_cast<std::string>(m_combat_info.m_opponents[i].non_combat_ships_hitpoints)+")";
        font->RenderText(x,y,x+130,y+font->Height(),text, format, 0, true);
        x+=130;

        text= "planets "+boost::lexical_cast<std::string>(m_combat_info.m_opponents[i].planets)
             + "("+boost::lexical_cast<std::string>(m_combat_info.m_opponents[i].planets_defence_bases)+")";
        font->RenderText(x,y,x+80,y+font->Height(),text, format, 0, true);
        x+=80;

        text= "destroyed/retreated/conquered "+boost::lexical_cast<std::string>(m_combat_info.m_opponents[i].destroyed_ships_destroyed)
             + "/"+boost::lexical_cast<std::string>(m_combat_info.m_opponents[i].retreated_ships)
             + "/"+boost::lexical_cast<std::string>(m_combat_info.m_opponents[i].defenseless_planets);
        font->RenderText(x,y,x+130,y+font->Height(),text, format, 0, true);
        x+=130;
      }


      GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, GG::CLR_WHITE, 1);
      return true;
    }

    const std::string& System() const {return m_combat_info.m_system;}
  private:
    CombatUpdateMessage m_combat_info;

};

struct CombatInfoRow : public GG::ListBox::Row
{
    CombatInfoRow(int w,const CombatUpdateMessage &combat_info)
    {
        height = 30 + 4;
        push_back(new CombatInfoControl(w,height,combat_info));
        data_type = "CombatInfo";
    }

    void Update(const CombatUpdateMessage &combat_info) {static_cast<CombatInfoControl*>(operator[](0))->Update(combat_info);}
    const std::string& System() const {return static_cast<CombatInfoControl*>(operator[](0))->System();}

};

////////////////////////////////////////////////
// CombatWnd
////////////////////////////////////////////////
CombatWnd::CombatWnd(int x,int y)
: CUI_Wnd("Combat Window",x,y, WIDTH, HEIGHT,  GG::Wnd::ONTOP | GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | CUI_Wnd::MINIMIZABLE)

{
  m_combats_lb = new CUIListBox(LeftBorder(),TopBorder(),Width()-(LeftBorder()+RightBorder()),Height()-(TopBorder()+BottomBorder()),GG::CLR_ZERO,GG::CLR_ZERO);
  AttachChild(m_combats_lb);
}

CombatWnd::~CombatWnd( )
{
}


void CombatWnd::UpdateCombatTurnProgress(const std::string& message)
{
  std::stringstream stream(message);
  GG::XMLDoc doc;
  doc.ReadDoc(stream);          

  CombatUpdateMessage msg(doc.root_node.Child("combat-update-message"));
  int r;
  for(r=0;r<m_combats_lb->NumRows();r++)
    if(static_cast<CombatInfoRow&>(m_combats_lb->GetRow(r)).System()==msg.m_system)
    {
      static_cast<CombatInfoRow&>(m_combats_lb->GetRow(r)).Update(msg);
      break;
    }

  if(r>=m_combats_lb->NumRows())
    m_combats_lb->Insert(new CombatInfoRow(m_combats_lb->Width() - ClientUI::SCROLL_WIDTH,msg));


}
