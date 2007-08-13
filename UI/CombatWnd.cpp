#include "CombatWnd.h"

#include "../combat/Combat.h"
#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include <boost/format.hpp>

namespace
{
    GG::SubTexture GetSubTexture(const boost::filesystem::path& path, bool mipmap = false)
    {
        boost::shared_ptr<GG::Texture> texture(ClientUI::GetTexture(path, mipmap));
        return GG::SubTexture(texture,0,0,texture->DefaultWidth(),texture->DefaultHeight());
    }
}

class CombatInfoControl : public GG::Control
{
  public:
    CombatInfoControl(int w, int h,const CombatUpdateMessage &combat_info) :
        Control(0, 0, w, h, GG::Flags<GG::WndFlag>()),
        m_combat_info(combat_info)
    {}

    void Update(const CombatUpdateMessage &combat_info) {m_combat_info=combat_info;}

    virtual void Render()
    {
      const int ITEM_WIDTH = 230;
      const GG::Rect  item_txt[4] ={GG::Rect(  7,27,100,45),
                                    GG::Rect(  4,84, 81,99),
                                    GG::Rect(124,49,205,67),
                                    GG::Rect(124,78,205,96)},
                      item_img_topic        ( 24,48,24+40,48+40),
                      item_img_arrow_split  ( 65,40,65+64,40+64),
                      rc_txt_empire         ( 25, 6,500,19);
                      const GG::Clr border_color = GG::FloatClr(0.5f,0.5f,0.5f,1.0f);
                      const GG::Clr bg_color = GG::FloatClr(0.25f,0.25f,0.25f,1.0f);
                      const GG::Clr bg_item_color = GG::FloatClr(0.15f,0.15f,0.15f,1.0f);

      GG::Pt ul(UpperLeft()),lr(LowerRight());
      GG::FlatRectangle(ul.x+1,ul.y+1,lr.x,lr.y,bg_color,border_color,2);

      boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::Font(), static_cast<int>(ClientUI::Pts()*1.0));
      GG::Flags<GG::TextFormat> format = GG::FORMAT_LEFT | GG::FORMAT_VCENTER;
      std::string text;

      GG::SubTexture img_topic[3],img_ship_civil,img_planet,img_arrow_split;

      img_topic[0]    = GetSubTexture(ClientUI::ArtDir() / "misc" / "mark2icon.png");
      img_topic[1]    = GetSubTexture(ClientUI::ArtDir() / "misc" / "colonyicon.png");
      img_topic[2]    = GetSubTexture(ClientUI::ArtDir() / "icons" / "colonymarker.png");
      img_arrow_split = GetSubTexture(ClientUI::ArtDir() / "misc" / "forkedarrow.png");

      int y=ul.y;

      GG::Rect rc;

      rc = GG::Rect(ul+GG::Pt(20,5),ul+GG::Pt(500,25));
      font = HumanClientApp::GetApp()->GetFont(ClientUI::Font(), static_cast<int>(ClientUI::Pts()*1.0));
      glColor(ClientUI::TextColor());
      format = GG::FORMAT_LEFT | GG::FORMAT_BOTTOM;
      font->RenderText(rc.UpperLeft(),rc.LowerRight(),UserString("COMBAT_BATTLE"), format, 0);

      rc = GG::Rect(ul+GG::Pt(20+50,5),ul+GG::Pt(500,28));
      font = HumanClientApp::GetApp()->GetFont(ClientUI::Font(), static_cast<int>(ClientUI::Pts()*1.7));
      glColor(ClientUI::TextColor());
      format = GG::FORMAT_LEFT | GG::FORMAT_BOTTOM;
      font->RenderText(rc.UpperLeft(),rc.LowerRight(),boost::io::str(boost::format(UserString("COMBAT_SYSTEM")) % m_combat_info.m_system), format, 0);
      

      for(unsigned int i=0;i<m_combat_info.m_opponents.size();i++,y+=font->Height()+2)
      {
        CombatUpdateMessage::EmpireCombatInfo *eci = &m_combat_info.m_opponents[i];

        GG::Rect area (ul.x+20,ul.y+30+i*110,ul.x+20+ITEM_WIDTH*3,ul.y+30+(i+1)*110-5);
        struct 
        {
            std::string txt;
            GG::Flags<GG::TextFormat> format;
            GG::Clr bg_clr;
            GG::Clr border_clr;
            unsigned int border_width;
        } entries[3][4] =
        {
          {
            {UserString("COMBAT_MILITARY_SHIPS") ,GG::FORMAT_CENTER | GG::FORMAT_VCENTER,GG::CLR_BLACK ,border_color ,1},
            {boost::lexical_cast<std::string>(eci->combat_ships          ) + UserString("COMBAT_REMAINING"),GG::FORMAT_CENTER | GG::FORMAT_VCENTER,GG::CLR_ZERO  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->combat_ships_retreated) + UserString("COMBAT_RETREATED"),GG::FORMAT_LEFT   | GG::FORMAT_VCENTER,(0==eci->combat_ships_retreated)?GG::CLR_ZERO:GG::CLR_ZERO  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->combat_ships_destroyed) + UserString("COMBAT_DESTROYED"),GG::FORMAT_LEFT   | GG::FORMAT_VCENTER,(0==eci->combat_ships_destroyed)?GG::CLR_ZERO:GG::CLR_RED   ,GG::CLR_ZERO ,0}
          },
          {
            {UserString("COMBAT_CIVILIAN_SHIPS") ,GG::FORMAT_CENTER | GG::FORMAT_VCENTER,GG::CLR_BLACK ,border_color ,1},
            {boost::lexical_cast<std::string>(eci->non_combat_ships          )+UserString("COMBAT_REMAINING")    ,GG::FORMAT_CENTER | GG::FORMAT_VCENTER,(0==0)?GG::CLR_ZERO:GG::CLR_ZERO  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->non_combat_ships_retreated)+UserString("COMBAT_RETREATED")    ,GG::FORMAT_LEFT   | GG::FORMAT_VCENTER,(0==eci->non_combat_ships_retreated)?GG::CLR_ZERO : GG::Clr(128,64,64,255)  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->non_combat_ships_destroyed)+UserString("COMBAT_DESTROYED")    ,GG::FORMAT_LEFT   | GG::FORMAT_VCENTER,(0==eci->non_combat_ships_destroyed)?GG::CLR_ZERO : GG::CLR_RED   ,GG::CLR_ZERO ,0}
          },
          {
            {UserString("COMBAT_PLANETS") ,GG::FORMAT_CENTER | GG::FORMAT_VCENTER,GG::CLR_BLACK ,border_color ,1},
            {boost::lexical_cast<std::string>(eci->planets            )+UserString("COMBAT_REMAINING")    ,GG::FORMAT_CENTER | GG::FORMAT_VCENTER,(0==0)?GG::CLR_ZERO:GG::CLR_ZERO  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->planets_defenseless)+UserString("COMBAT_DEFENSELESS")  ,GG::FORMAT_LEFT   | GG::FORMAT_VCENTER,(0==eci->planets_defenseless)?GG::CLR_ZERO:GG::CLR_ZERO  ,GG::CLR_ZERO ,0},
            {boost::lexical_cast<std::string>(eci->planets_lost       )+UserString("COMBAT_LOST")         ,GG::FORMAT_LEFT   | GG::FORMAT_VCENTER,(0==eci->planets_lost       )?GG::CLR_ZERO:GG::CLR_RED   ,GG::CLR_ZERO ,0}
          }
        };
        
        GG::FlatRectangle(area.Left(), area.Top(), area.Right()+2, area.Bottom(),bg_item_color,border_color, 2);

        font = HumanClientApp::GetApp()->GetFont(ClientUI::Font(), static_cast<int>(ClientUI::Pts()*1.2));

        rc = GG::Rect(area.UpperLeft()+rc_txt_empire.UpperLeft(),area.UpperLeft()+rc_txt_empire.LowerRight());
        glColor(ClientUI::TextColor());format = GG::FORMAT_LEFT | GG::FORMAT_VCENTER;
        font->RenderText(rc.UpperLeft(),rc.LowerRight(),m_combat_info.m_opponents[i].empire, format, 0);

        for(unsigned int c=0;c<3;c++)
        {
          GG::Rect col (area.Left()+c*ITEM_WIDTH, area.Top(),area.Left()+(c+1)*ITEM_WIDTH, area.Bottom());
          
          GG::FlatRectangle(col.Left(), col.Top()+38, col.Right()+2, col.Bottom(), GG::CLR_ZERO,border_color, 2);
          glColor(ClientUI::TextColor());

          img_topic[c]    .OrthoBlit(col.UpperLeft()+item_img_topic      .UpperLeft(),col.UpperLeft()+item_img_topic      .LowerRight());
          img_arrow_split .OrthoBlit(col.UpperLeft()+item_img_arrow_split.UpperLeft(),col.UpperLeft()+item_img_arrow_split.LowerRight());

          font = HumanClientApp::GetApp()->GetFont(ClientUI::Font(), static_cast<int>(ClientUI::Pts()*1.0));
          for(unsigned int j=0;j<4;j++)
          {
            rc = GG::Rect(col.UpperLeft()+item_txt[j].UpperLeft(),col.UpperLeft()+item_txt[j].LowerRight());
            GG::FlatRectangle(rc.Left(), rc.Top(), rc.Right(), rc.Bottom(),entries[c][j].bg_clr,entries[c][j].border_clr,entries[c][j].border_width);
            glColor(ClientUI::TextColor());
            font->RenderText(rc.UpperLeft(),rc.LowerRight(),entries[c][j].txt, entries[c][j].format, 0);
          }
        }
      }
    }

    const std::string& System() const {return m_combat_info.m_system;}
  private:
    CombatUpdateMessage m_combat_info;

};

struct CombatInfoRow : public GG::ListBox::Row
{
    CombatInfoRow(int w,const CombatUpdateMessage &combat_info) :
        GG::ListBox::Row(w, 30+combat_info.m_opponents.size()*110, "CombatInfo")
    {
        push_back(new CombatInfoControl(w, Height(), combat_info));
    }

    void Update(const CombatUpdateMessage &combat_info) {static_cast<CombatInfoControl*>(operator[](0))->Update(combat_info);}
    const std::string& System() const {return static_cast<CombatInfoControl*>(operator[](0))->System();}

};

////////////////////////////////////////////////
// CombatWnd
////////////////////////////////////////////////
CombatWnd::CombatWnd(int x,int y)
    : CUIWnd(UserString("COMBAT_WINDOW_TITLE"),x,y, WIDTH, HEIGHT,  GG::ONTOP | GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE | MINIMIZABLE)
{
  m_combats_lb = new CUIListBox(0,0,ClientWidth(),ClientHeight(),GG::CLR_ZERO,GG::CLR_ZERO);
  AttachChild(m_combats_lb);
}

CombatWnd::~CombatWnd( )
{
}

void CombatWnd::UpdateCombatTurnProgress(const std::string& message)
{
  std::stringstream stream(message);
  XMLDoc doc;
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
    m_combats_lb->Insert(new CombatInfoRow(m_combats_lb->Width() - ClientUI::ScrollWidth(),msg));


}
