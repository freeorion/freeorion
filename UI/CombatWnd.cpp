#include "CombatWnd.h"

#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "../client/human/HumanClientApp.h"

#include "../combat/Combat.h"
namespace
{
  boost::shared_ptr<GG::Texture> GetTexture(const std::string& name, bool mipmap = false)
  {
    try
    {
      return HumanClientApp::GetApp()->GetTexture(name,mipmap);
    }
    catch(...)
    {
      return HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "misc/missing.png",mipmap);
    }
  }

  GG::SubTexture GetSubTexture(const std::string& name, bool mipmap = false)
  {
    boost::shared_ptr<GG::Texture> texture(GetTexture(name,mipmap));
    return GG::SubTexture(texture,0,0,texture->DefaultWidth(),texture->DefaultHeight());
  }

}

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
      const int ITEM_WIDTH = 230;
      const GG::Rect  item_txt[4] ={GG::Rect(  7,27,100,45),
                                    GG::Rect(  4,84, 81,99),
                                    GG::Rect(124,49,205,67),
                                    GG::Rect(124,78,205,96)},
                      item_img_topic        ( 24,48,24+40,48+40),
                      item_img_arrow_split  ( 65,40,65+64,40+64),
                      rc_txt_empire         ( 25, 6,500,19);
      const GG::Clr border_color(0.5,0.5,0.5,1.0),bg_color(0.25,0.25,0.25,1.0),bg_item_color(0.15,0.15,0.15,1.0);

      GG::Pt ul(UpperLeft()),lr(LowerRight());
      GG::FlatRectangle(ul.x+1,ul.y+1,lr.x,lr.y,bg_color,border_color,2);

      boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
      Uint32 format = GG::TF_LEFT | GG::TF_VCENTER;
      std::string text;

      GG::SubTexture img_topic[3],img_ship_civil,img_planet,img_arrow_split;

      img_topic[0]    = GetSubTexture(ClientUI::ART_DIR + "misc/mark2icon.png");
      img_topic[1]    = GetSubTexture(ClientUI::ART_DIR + "misc/colonyicon.png");
      img_topic[2]    = GetSubTexture(ClientUI::ART_DIR + "icons/colonymarker.png");
      img_arrow_split = GetSubTexture(ClientUI::ART_DIR + "misc/forkedarrow.png");

      int y=ul.y;

      GG::Rect rc;

      rc = GG::Rect(ul+GG::Pt(20,5),ul+GG::Pt(500,25));
      font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
      glColor4ubv(ClientUI::TEXT_COLOR.v);format = GG::TF_LEFT | GG::TF_BOTTOM;
      font->RenderText(rc.UpperLeft(),rc.LowerRight(),"BATTLE : ", format, 0, true);

      rc = GG::Rect(ul+GG::Pt(20+50,5),ul+GG::Pt(500,28));
      font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.7));
      glColor4ubv(ClientUI::TEXT_COLOR.v);format = GG::TF_LEFT | GG::TF_BOTTOM;
      font->RenderText(rc.UpperLeft(),rc.LowerRight(),"The " + m_combat_info.m_system +" System", format, 0, true);
      

      for(unsigned int i=0;i<m_combat_info.m_opponents.size();i++,y+=font->Height()+2)
      {
        CombatUpdateMessage::EmpireCombatInfo *eci = &m_combat_info.m_opponents[i];

        GG::Rect area (ul.x+20,ul.y+30+i*110,ul.x+20+ITEM_WIDTH*3,ul.y+30+(i+1)*110-5);
        struct 
        {
          std::string txt;Uint32 txt_fmt;
          //GG::Clr txt_clr;
          GG::Clr bg_clr;GG::Clr border_clr;unsigned int border_width;
        } entries[3][4] =
        {
          {
            {"Military Ships" ,GG::TF_CENTER | GG::TF_VCENTER,GG::CLR_BLACK ,border_color ,1},
            {boost::lexical_cast<std::string>(eci->combat_ships          ) + " Remaining",GG::TF_CENTER | GG::TF_VCENTER,GG::CLR_ZERO  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->combat_ships_retreated) + " Retreated",GG::TF_LEFT   | GG::TF_VCENTER,(0==eci->combat_ships_retreated)?GG::CLR_ZERO:GG::CLR_ZERO  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->combat_ships_destroyed) + " Destroyed",GG::TF_LEFT   | GG::TF_VCENTER,(0==eci->combat_ships_destroyed)?GG::CLR_ZERO:GG::CLR_RED   ,GG::CLR_ZERO ,0}
          },
          {
            {"Civilian Ships" ,GG::TF_CENTER | GG::TF_VCENTER,GG::CLR_BLACK ,border_color ,1},
            {boost::lexical_cast<std::string>(eci->non_combat_ships          )+" Remaining"    ,GG::TF_CENTER | GG::TF_VCENTER,(0==0)?GG::CLR_ZERO:GG::CLR_ZERO  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->non_combat_ships_retreated)+" Retreated"    ,GG::TF_LEFT   | GG::TF_VCENTER,(0==eci->non_combat_ships_retreated)?GG::CLR_ZERO : GG::Clr(128,64,64,255)  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->non_combat_ships_destroyed)+" Destroyed"    ,GG::TF_LEFT   | GG::TF_VCENTER,(0==eci->non_combat_ships_destroyed)?GG::CLR_ZERO : GG::CLR_RED   ,GG::CLR_ZERO ,0}
          },
          {
            {"Planets"        ,GG::TF_CENTER | GG::TF_VCENTER,GG::CLR_BLACK ,border_color ,1},
            {boost::lexical_cast<std::string>(eci->planets            )+" Remaining"    ,GG::TF_CENTER | GG::TF_VCENTER,(0==0)?GG::CLR_ZERO:GG::CLR_ZERO  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->planets_defenseless)+" Defenceless"  ,GG::TF_LEFT   | GG::TF_VCENTER,(0==eci->planets_defenseless)?GG::CLR_ZERO:GG::CLR_ZERO  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->planets_lost       )+" Lost!"        ,GG::TF_LEFT   | GG::TF_VCENTER,(0==eci->planets_lost       )?GG::CLR_ZERO:GG::CLR_RED   ,GG::CLR_ZERO ,0}
          }
        };
        
        GG::FlatRectangle(area.Left(), area.Top(), area.Right()+2, area.Bottom(),bg_item_color,border_color, 2);

        font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.2));

        rc = GG::Rect(area.UpperLeft()+rc_txt_empire.UpperLeft(),area.UpperLeft()+rc_txt_empire.LowerRight());
        glColor4ubv(ClientUI::TEXT_COLOR.v);format = GG::TF_LEFT | GG::TF_VCENTER;
        font->RenderText(rc.UpperLeft(),rc.LowerRight(),m_combat_info.m_opponents[i].empire, format, 0, true);

        for(unsigned int c=0;c<3;c++)
        {
          GG::Rect col (area.Left()+c*ITEM_WIDTH, area.Top(),area.Left()+(c+1)*ITEM_WIDTH, area.Bottom());
          
          GG::FlatRectangle(col.Left(), col.Top()+38, col.Right()+2, col.Bottom(), GG::CLR_ZERO,border_color, 2);
          glColor4ubv(ClientUI::TEXT_COLOR.v);

          img_topic[c]    .OrthoBlit(col.UpperLeft()+item_img_topic      .UpperLeft(),col.UpperLeft()+item_img_topic      .LowerRight());
          img_arrow_split .OrthoBlit(col.UpperLeft()+item_img_arrow_split.UpperLeft(),col.UpperLeft()+item_img_arrow_split.LowerRight());

          font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
          for(unsigned int j=0;j<4;j++)
          {
            rc = GG::Rect(col.UpperLeft()+item_txt[j].UpperLeft(),col.UpperLeft()+item_txt[j].LowerRight());
            GG::FlatRectangle(rc.Left(), rc.Top(), rc.Right(), rc.Bottom(),entries[c][j].bg_clr,entries[c][j].border_clr,entries[c][j].border_width);
            glColor4ubv(ClientUI::TEXT_COLOR.v);
            font->RenderText(rc.UpperLeft(),rc.LowerRight(),entries[c][j].txt, entries[c][j].txt_fmt, 0, true);
          }
        }
      }
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
        height = 30+combat_info.m_opponents.size()*110;//200;
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
: CUI_Wnd("Combat Window",x,y, WIDTH, HEIGHT,  GG::Wnd::ONTOP | GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::RESIZABLE | CUI_Wnd::MINIMIZABLE)

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
