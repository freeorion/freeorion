#include "SidePanel.h"

#include "CUI_Wnd.h"
#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "GGDynamicGraphic.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../Empire/TechLevel.h"
#include "../util/Random.h"
#include "XMLDoc.h"
#include "GGBase.h"

#ifndef __GGScroll_h_
#include "GGScroll.h"
#endif

#include "../universe/Fleet.h"
#include "../universe/Ship.h"

 
namespace {
int CircleXFromY(double y, double r) {return static_cast<int>(std::sqrt(r * r - y * y) + 0.5);}
}


////////////////////////////////////////////////
// SidePanel::PlanetPanel
////////////////////////////////////////////////
namespace {
  const int MAX_PLANET_SIZE = 128; // size of a huge planet, in on-screen pixels
  const int MIN_PLANET_SIZE = MAX_PLANET_SIZE / 3; // size of a tiny planet, in on-screen pixels
  const int IMAGES_PER_PLANET_TYPE = 3; // number of planet images available per planet type (named "type1.png", "type2.png", ...)
  const int SYSTEM_NAME_FONT_SIZE = static_cast<int>(ClientUI::PTS*1.4);
}



namespace {
  boost::shared_ptr<GG::Texture> IconBalance   () {return GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "icons/focusbuttoncrossicon.png"        );}
  boost::shared_ptr<GG::Texture> IconPopulation() {return GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "icons/pop.png"        );}
  boost::shared_ptr<GG::Texture> IconIndustry  () {return GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "icons/industry.png"   );}
  boost::shared_ptr<GG::Texture> IconResearch  () {return GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "icons/research.png"   );}
  boost::shared_ptr<GG::Texture> IconMining    () {return GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "icons/mining.png"     );}
  boost::shared_ptr<GG::Texture> IconFarming   () {return GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "icons/farming.png"    );}
  boost::shared_ptr<GG::Texture> IconDefense   () {return GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "icons/defensebase.png");}


  std::string Format(const char *fmt,...)
  {
    char buffer[1024]; va_list args;
    
    va_start(args,fmt);
    vsprintf(buffer,fmt,args);

    return buffer;
  }

  struct SystemRow : public GG::ListBox::Row
  {
    public:
      SystemRow(int system_id) : m_system_id(system_id) {data_type = "SystemID";}

      int m_system_id;
  };

  struct ConstructionRow : public GG::ListBox::Row
  {
    public:
      ConstructionRow(ProdCenter::BuildType build_type) : m_build_type(build_type) {data_type = "BuildType";}

      ProdCenter::BuildType m_build_type;
  };

  boost::shared_ptr<GG::Texture> GetPlanetTextureStatic(const Planet &planet)
  {
    std::string planet_image = ClientUI::ART_DIR + "planets/";
    switch (planet.Type())
    {
      case Planet::PT_SWAMP     : planet_image += "swamp"     ; break;
      case Planet::PT_TOXIC     : planet_image += "toxic"     ; break;
      case Planet::PT_INFERNO   : planet_image += "inferno"   ; break;
      case Planet::PT_RADIATED  : planet_image += "radiated"  ; break;
      case Planet::PT_BARREN    : planet_image += "barren"    ; break;
      case Planet::PT_TUNDRA    : planet_image += "tundra"    ; break;
      case Planet::PT_DESERT    : planet_image += "desert"    ; break;
      case Planet::PT_TERRAN    : planet_image += "terran"    ; break;
      case Planet::PT_OCEAN     : planet_image += "ocean"     ; break;
      case Planet::PT_GAIA      : planet_image += "gaia"      ; break;
      case Planet::PT_ASTEROIDS : planet_image += "asteroids" ; break;
      case Planet::PT_GASGIANT  : planet_image += "gasgiant"  ; break;    
      default                   : planet_image += "barren"    ; break;
    }
    planet_image += boost::lexical_cast<std::string>((planet.ID() % IMAGES_PER_PLANET_TYPE) + 1) + ".png";
    try
    {
      return HumanClientApp::GetApp()->GetTexture(planet_image);
    }
    catch(...)
    {
      return HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "planets/terran1.png");
    }
  }

  GG::XMLElement GetXMLChild(GG::XMLElement &node,const std::string &child_path)
  {
    int index;

    if(-1==(index=child_path.find_first_of('.')))
      return node.ContainsChild(child_path)?node.Child(child_path):GG::XMLElement();
    else
      return node.ContainsChild(child_path.substr(0,index))
              ?GetXMLChild(node.Child(child_path.substr(0,index)),child_path.substr(index+1,child_path.length()-index-1))
              :GG::XMLElement();
  }

  int GetPlanetTexturesDynamic(const Planet &planet,std::vector<boost::shared_ptr<GG::Texture> > &textures, int &start_frame, double &fps)
  {
    GG::XMLDoc planetart_doc;
    std::ifstream ifs((ClientUI::ART_DIR + "planets/planets.xml").c_str());
    planetart_doc.ReadDoc(ifs);
    ifs.close();

    std::string plt_art_node_name = "ArtPlanets.";
    switch (planet.Type())
    {
      case Planet::PT_SWAMP     : plt_art_node_name += "Swamp"     ; break;
      case Planet::PT_TOXIC     : plt_art_node_name += "Toxic"     ; break;
      case Planet::PT_INFERNO   : plt_art_node_name += "Inferno"   ; break;
      case Planet::PT_RADIATED  : plt_art_node_name += "Radiated"  ; break;
      case Planet::PT_BARREN    : plt_art_node_name += "Barren"    ; break;
      case Planet::PT_TUNDRA    : plt_art_node_name += "Tundra"    ; break;
      case Planet::PT_DESERT    : plt_art_node_name += "Desert"    ; break;
      case Planet::PT_TERRAN    : plt_art_node_name += "Terran"    ; break;
      case Planet::PT_OCEAN     : plt_art_node_name += "Ocean"     ; break;
      case Planet::PT_GAIA      : plt_art_node_name += "Gaia"      ; break;
      case Planet::PT_ASTEROIDS : plt_art_node_name += "Asteroids" ; break;
      case Planet::PT_GASGIANT  : plt_art_node_name += "GasGiant"  ; break;    
      default                   : plt_art_node_name += "Barren"    ; break;
    }

    GG::XMLElement plt_art_node = GetXMLChild(planetart_doc.root_node,plt_art_node_name);

    if(plt_art_node.Tag().length()>0)
    {
      std::vector<GG::XMLElement> plt_art_vec;

      for(int i=0; i< plt_art_node.NumChildren();i++)
      {
        GG::XMLElement single_plt_art =plt_art_node.Child(i);
        if(0==single_plt_art.Tag().compare("Art"))
          plt_art_vec.push_back(single_plt_art);
      }

      if(plt_art_vec.size()>0)
      {
        GG::XMLElement chosen_plt_art = plt_art_vec[planet.ID() % plt_art_vec.size()];
       
        if(chosen_plt_art.ContainsChild("File"))
        {
          GG::XMLElement file(chosen_plt_art.Child("File"));

          std::string filename = file.ContainsAttribute("Filename")?file.Attribute("Filename"):"";
          int from = file.ContainsAttribute("From")?boost::lexical_cast<int>(file.Attribute("From")):0,
              to   = file.ContainsAttribute("To"  )?boost::lexical_cast<int>(file.Attribute("To"  )):0;

          if(std::string::npos==filename.find('%'))
            textures.push_back(HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "planets/"+filename));
          else
          {
            if(from<=to)
              for(int i=from; i<=to; i++)
              {
                std::string filename_image = filename,
                            index          = boost::lexical_cast<std::string>(i);

                filename_image.replace(filename_image.find_last_of('%')-index.length()+1,index.length(),index);
                while(filename_image.find('%')!=std::string::npos)
                  filename_image.replace(filename_image.find('%'),1,"0");

                textures.push_back(HumanClientApp::GetApp()->GetTexture(ClientUI::ART_DIR + "planets/"+filename_image));
              }
          }
          if(chosen_plt_art.ContainsChild("FPS"))
          {
            std::string plt_size_name;
            switch (planet.Size())
            {
              case Planet::SZ_TINY      : plt_size_name = "Tiny"     ; break;
              case Planet::SZ_SMALL     : plt_size_name = "Small"    ; break;
              case Planet::SZ_MEDIUM    : plt_size_name = "Medium"   ; break;
              case Planet::SZ_LARGE     : plt_size_name = "Large"    ; break;
              case Planet::SZ_HUGE      : plt_size_name = "Huge"     ; break;
              case Planet::SZ_ASTEROIDS : plt_size_name = "Asteroids"; break;
              case Planet::SZ_GASGIANT  : plt_size_name = "GasGiant";  break;
              default                   : plt_size_name = "Default"  ; break;
            }

            if(chosen_plt_art.Child("FPS").ContainsAttribute(plt_size_name))
              fps = boost::lexical_cast<double>(chosen_plt_art.Child("FPS").Attribute(plt_size_name));
          }

          if(chosen_plt_art.ContainsChild("StartFrame"))
          {
            if(chosen_plt_art.Child("StartFrame").ContainsAttribute("value"))
              start_frame=boost::lexical_cast<int>(chosen_plt_art.Child("StartFrame").Attribute("value"));
          }
        }
      }
    }

    if(textures.size()==0)
      textures.push_back(GetPlanetTextureStatic(planet));

    return textures.size();
  }

  int GetPlanetTextures(const Planet &planet,std::vector<boost::shared_ptr<GG::Texture> > &textures, int &start_frame, double &fps)
  {
    try
    {
      return GetPlanetTexturesDynamic(planet,textures,start_frame,fps);
    }
    catch(...)
    {
      textures.push_back(GetPlanetTextureStatic(planet));
      return textures.size();
    }
  }

  std::string GetPlanetSizeName(const Planet &planet)
  {
    switch (planet.Size())
    {
      case Planet::SZ_TINY      : return ClientUI::String("PL_SZ_TINY"  );
      case Planet::SZ_SMALL     : return ClientUI::String("PL_SZ_SMALL" ); 
      case Planet::SZ_MEDIUM    : return ClientUI::String("PL_SZ_MEDIUM"); 
      case Planet::SZ_LARGE     : return ClientUI::String("PL_SZ_LARGE" ); 
      case Planet::SZ_HUGE      : return ClientUI::String("PL_SZ_HUGE"  ); 
      case Planet::SZ_ASTEROIDS : return "";
      case Planet::SZ_GASGIANT  : return "";
      default                   : return "ERROR ";
    }
  }

  std::string GetPlanetTypeName(const Planet &planet)
  {
    switch (planet.Type())
    {
      case Planet::PT_SWAMP     : return ClientUI::String("PL_SWAMP"    );
      case Planet::PT_TOXIC     : return ClientUI::String("PL_TOXIC"    );
      case Planet::PT_INFERNO   : return ClientUI::String("PL_INFERNO"  );
      case Planet::PT_RADIATED  : return ClientUI::String("PL_RADIATED" );
      case Planet::PT_BARREN    : return ClientUI::String("PL_BARREN"   );
      case Planet::PT_TUNDRA    : return ClientUI::String("PL_TUNDRA"   );
      case Planet::PT_DESERT    : return ClientUI::String("PL_DESERT"   );
      case Planet::PT_TERRAN    : return ClientUI::String("PL_TERRAN"   );
      case Planet::PT_OCEAN     : return ClientUI::String("PL_OCEAN"    );
      case Planet::PT_GAIA      : return ClientUI::String("PL_GAIA"     );
      case Planet::PT_ASTEROIDS : return ClientUI::String("PL_ASTEROIDS");
      case Planet::PT_GASGIANT  : return ClientUI::String("PL_GASGIANT" );        
      default                   : return "ERROR ";
    }
  }

  Ship* FindColonyShip(int system_id)
  {
    const System *system = dynamic_cast<const System *>(GetUniverse().Object(system_id));
    if(system==0)
      return 0;//UniverseObject::INVALID_OBJECT_ID;

    std::vector<const Fleet*> flt_vec = system->FindObjects<Fleet>();

    for(unsigned int i=0;i<flt_vec.size();i++)
      if(flt_vec[i]->Owners().find(HumanClientApp::GetApp()->PlayerID()) != flt_vec[i]->Owners().end())
      {
        Ship* ship;
        for(Fleet::const_iterator it = flt_vec[i]->begin(); it != flt_vec[i]->end(); ++it)
          if(   (ship=dynamic_cast<Ship*>(GetUniverse().Object(*it)))
             && ship->Design().colonize)
            return ship;//ship->ID();
      }

    return 0;//UniverseObject::INVALID_OBJECT_ID;
  }

  void AngledCornerRectangle(int x1, int y1, int x2, int y2, GG::Clr color, GG::Clr border, int thick, 
                             int upper_left_angle_offset, int upper_right_angle_offset, 
                             int lower_right_angle_offset, int lower_left_angle_offset)
  {
    glDisable(GL_TEXTURE_2D);

    int inner_x1 = x1 + thick;
    int inner_y1 = y1 + thick;
    int inner_x2 = x2 - thick;
    int inner_y2 = y2 - thick;

    // these are listed in CCW order for convenience
    int ul_corner_x1 = x1 + upper_left_angle_offset ; int ul_corner_y1 = y1;
    int ul_corner_x2 = x1                           ; int ul_corner_y2 = y1 + upper_left_angle_offset;
    int lr_corner_x1 = x2 - lower_right_angle_offset; int lr_corner_y1 = y2;
    int lr_corner_x2 = x2                           ; int lr_corner_y2 = y2 - lower_right_angle_offset;

    int ll_corner_x1 = x1 + lower_left_angle_offset ; int ll_corner_y1 = y2;
    int ll_corner_x2 = x1                           ; int ll_corner_y2 = y2 - lower_left_angle_offset;
    int ur_corner_x1 = x2 - upper_right_angle_offset; int ur_corner_y1 = y1;
    int ur_corner_x2 = x2                           ; int ur_corner_y2 = y1 + upper_right_angle_offset;

    int inner_ul_corner_x1 = ul_corner_x1 + thick; int inner_ul_corner_y1 = ul_corner_y1 + thick;
    int inner_ul_corner_x2 = ul_corner_x2 + thick; int inner_ul_corner_y2 = ul_corner_y2 + thick;
    int inner_lr_corner_x1 = lr_corner_x1 - thick; int inner_lr_corner_y1 = lr_corner_y1 - thick;
    int inner_lr_corner_x2 = lr_corner_x2 - thick; int inner_lr_corner_y2 = lr_corner_y2 - thick;

    int inner_ll_corner_x1 = ll_corner_x1 + thick; int inner_ll_corner_y1 = ll_corner_y1 - thick;
    int inner_ll_corner_x2 = ll_corner_x2 + thick; int inner_ll_corner_y2 = ll_corner_y2 - thick;
    int inner_ur_corner_x1 = ur_corner_x1 - thick; int inner_ur_corner_y1 = ur_corner_y1 + thick;
    int inner_ur_corner_x2 = ur_corner_x2 - thick; int inner_ur_corner_y2 = ur_corner_y2 + thick;

    // draw beveled edges
    if (thick) 
    {
      glBegin(GL_QUADS);
      glColor4ubv(border.v);

      // the top =>
      if(upper_right_angle_offset>0)
      {
        glVertex2i(inner_ur_corner_x1, inner_ur_corner_y1);
        glVertex2i(ur_corner_x1, ur_corner_y1);
      }
      else
      {
        glVertex2i(inner_x2, inner_y1);
        glVertex2i(x2, y1);
      }

      if(upper_left_angle_offset>0)
      {
        glVertex2i(ul_corner_x1, ul_corner_y1);
        glVertex2i(inner_ul_corner_x1, inner_ul_corner_y1);
	  } 
      else
      {
        glVertex2i(x1, y1);
        glVertex2i(inner_x1, inner_y1);
      }
      // <= the top 

      // the left side => 
      if(upper_left_angle_offset>0)
      {
        glVertex2i(inner_ul_corner_x2, inner_ul_corner_y2);
        glVertex2i(ul_corner_x2, ul_corner_y2);
      } 
      else
      {
        glVertex2i(inner_x1, inner_y1);
        glVertex2i(x1, y1);
      }

      if(lower_left_angle_offset>0)
      {
        glVertex2i(ll_corner_x2,ll_corner_y2);
        glVertex2i(inner_ll_corner_x2,inner_ll_corner_y2);
      }
      else
      {
        glVertex2i(x1, y2);
        glVertex2i(inner_x1, inner_y2);
      }
      // <= the left side 

      // the bottom =>
      if(lower_left_angle_offset>0)
      {
        glVertex2i(inner_ll_corner_x1,inner_ll_corner_y1);
        glVertex2i(ll_corner_x1,ll_corner_y1);
      }
      else
      {
        glVertex2i(inner_x1, inner_y2);
        glVertex2i(x1, y2);
      }
      if(lower_right_angle_offset>0)
      {
        glVertex2i(lr_corner_x1, lr_corner_y1);
        glVertex2i(inner_lr_corner_x1, inner_lr_corner_y1);
      }
      else
      {
        glVertex2i(x2, y2);
        glVertex2i(inner_x2, inner_y2);
      }
      // <= the bottom 

      // the right side =>
      if(lower_right_angle_offset>0)
      {
        glVertex2i(inner_lr_corner_x2, inner_lr_corner_y2);
        glVertex2i(lr_corner_x2, lr_corner_y2);
      }
      else
      {
        glVertex2i(inner_x2, inner_y2);
        glVertex2i(x2, y2);
      }
      if(upper_right_angle_offset>0)
      {
        glVertex2i(ur_corner_x2, ur_corner_y2);
        glVertex2i(inner_ur_corner_x2, inner_ur_corner_y2);
      }
      else
      {
        glVertex2i(x2, y1);
        glVertex2i(inner_x2, inner_y1);
      }
      // <= the right side


      // the lower-right angled side
      if(lower_right_angle_offset>0) 
      {
        glVertex2i(inner_lr_corner_x1, inner_lr_corner_y1);
        glVertex2i(lr_corner_x1, lr_corner_y1);
        glVertex2i(lr_corner_x2, lr_corner_y2);
        glVertex2i(inner_lr_corner_x2, inner_lr_corner_y2);
      }

      // the upper-left angled side
      if(upper_left_angle_offset>0) 
      {
        glVertex2i(inner_ul_corner_x1, inner_ul_corner_y1);
        glVertex2i(ul_corner_x1, ul_corner_y1);
        glVertex2i(ul_corner_x2, ul_corner_y2);
        glVertex2i(inner_ul_corner_x2, inner_ul_corner_y2);
      }

      // the upper-right angled side
      if(upper_right_angle_offset>0) 
      {
        glVertex2i(inner_ur_corner_x1, inner_ur_corner_y1);
        glVertex2i(ur_corner_x1, ur_corner_y1);
        glVertex2i(ur_corner_x2, ur_corner_y2);
        glVertex2i(inner_ur_corner_x2, inner_ur_corner_y2);
      }

      // the lower-left angled side
      if(lower_left_angle_offset>0) 
      {
        glVertex2i(inner_ll_corner_x1, inner_ll_corner_y1);
        glVertex2i(ll_corner_x1, ll_corner_y1);
        glVertex2i(ll_corner_x2, ll_corner_y2);
        glVertex2i(inner_ll_corner_x2, inner_ll_corner_y2);
      }

      glEnd();
    }

    // draw interior of rectangle
    glColor4ubv(color.v);
    glBegin(GL_POLYGON);
    if(upper_left_angle_offset>0)
    {
      glVertex2i(inner_ul_corner_x1, inner_ul_corner_y1);
      glVertex2i(inner_ul_corner_x2, inner_ul_corner_y2);
    } 
    else 
      glVertex2i(inner_x1, inner_y1);


    if(lower_left_angle_offset>0)
    {
      glVertex2i(inner_ll_corner_x2, inner_ll_corner_y2);
      glVertex2i(inner_ll_corner_x1, inner_ll_corner_y1);
    } 
    else 
      glVertex2i(inner_x1, inner_y2);

    if(lower_right_angle_offset>0)
    {
      glVertex2i(inner_lr_corner_x1, inner_lr_corner_y1);
      glVertex2i(inner_lr_corner_x2, inner_lr_corner_y2);
    }
    else
      glVertex2i(inner_x2, inner_y2);

    if(upper_right_angle_offset>0)
    {
      glVertex2i(inner_ur_corner_x2, inner_ur_corner_y2);
      glVertex2i(inner_ur_corner_x1, inner_ur_corner_y1);
    }
    else
      glVertex2i(inner_x2, inner_y1);

    glEnd();

    glEnable(GL_TEXTURE_2D);
  }

  bool InAngledCornerRect(const GG::Pt& pt, int x1, int y1, int x2, int y2, 
                             int upper_left_angle_offset, int upper_right_angle_offset, 
                             int lower_right_angle_offset, int lower_left_angle_offset)
  {
    GG::Pt ul(x1, y1),lr(x2, y2);
    if(!(ul <= pt && pt < lr))
      return false;

    GG::Pt dist = pt-GG::Pt(x1,y1);
    if((upper_left_angle_offset>0) && (dist.x*dist.x + dist.y*dist.y < upper_left_angle_offset*upper_left_angle_offset))
      return false;

    dist = pt-GG::Pt(x2,y1);
    if((upper_right_angle_offset>0) && (dist.x*dist.x + dist.y*dist.y < upper_right_angle_offset*upper_right_angle_offset))
      return false;

    dist = pt-GG::Pt(x2,y2);
    if((lower_right_angle_offset>0) && (dist.x*dist.x + dist.y*dist.y < lower_right_angle_offset*lower_right_angle_offset))
      return false;

    dist = pt-GG::Pt(x1,y2);
    if((lower_left_angle_offset>0) && (dist.x*dist.x + dist.y*dist.y < lower_left_angle_offset*lower_left_angle_offset))
      return false;

    return true;
  }
}

  class CUIIconButton : public GG::Button
  {
    public:
      enum Styles {IBS_LEFT,IBS_RIGHT,IBS_HCENTER,IBS_TOP,IBS_BOTTOM,IBS_VCENTER,IBS_OVERLAPPED};

      /** \name Structors */ //@{
      CUIIconButton(int x, int y, int w, int h,const boost::shared_ptr<GG::Texture> &icon, const std::string& font_filename = ClientUI::FONT, int pts = ClientUI::PTS, 
            GG::Clr color = ClientUI::BUTTON_COLOR, GG::Clr border = ClientUI::CTRL_BORDER_COLOR, int thick = 1, 
            GG::Clr text_color = ClientUI::TEXT_COLOR, Uint32 flags = GG::Wnd::CLICKABLE); ///< basic ctor
      //CUIIconButton(const GG::XMLElement& elem); ///< ctor that constructs a CUIScroll::ScrollTab object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUIScroll::ScrollTab object
      //@}

      /** \name Accessors */ //@{
      virtual bool            InWindow(const GG::Pt& pt) const;
      //virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a CUIScroll::ScrollTab object
      ClickedSignalType& RClickedSignal() const                    {return m_rclicked_sig;} ///< returns the clicked signal object for this Button

      const GG::SubTexture& Icon() const {return m_icon;}

      //@}

      /** \name Mutators control */ //@{
      virtual void   SetBorderColor(GG::Clr c) {m_border_color=c;}   ///< sets the control's border color;
      
      void SetValue(double value); ///< sets the value to be displayed
      void SetDecimalsShown(int d)     {m_decimals_to_show = d; SetValue(m_value);} ///< sets the number of places after the decimal point to be shown
      void ShowSign(bool b)            {m_show_sign = b; SetValue(m_value);}        ///< sets whether a sign should always be shown, even for positive values
      void SetPositiveColor(GG::Clr c) {m_positive_color = c; SetValue(m_value);}   ///< sets the color that will be used to display positive values
      void SetNegativeColor(GG::Clr c) {m_negative_color = c; SetValue(m_value);}   ///< sets the color that will be used to display negative values
      
      void SetAngledCornerUpperLeft (int angled_corner) {m_angled_corner_upperleft =angled_corner;} 
      void SetAngledCornerUpperRight(int angled_corner) {m_angled_corner_upperright=angled_corner;} 
      void SetAngledCornerLowerRight(int angled_corner) {m_angled_corner_lowerright=angled_corner;} 
      void SetAngledCornerLowerLeft (int angled_corner) {m_angled_corner_lowerleft =angled_corner;} 

      void SetIconRect(const GG::Rect &rect) {m_icon_rect=rect;}
      void SetTextRect(const GG::Rect &rect) {m_text_rect=rect;}
      
      //@}

    protected:
      /** \name Mutators control */ //@{
      virtual void RenderPressed();
      virtual void RenderRollover();
      virtual void RenderUnpressed();
      //@}

      virtual void   RButtonDown(const GG::Pt& pt, Uint32 keys)          {if (!Disabled()) SetState(BN_PRESSED);}
      virtual void   RDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys){if (!Disabled()) SetState(BN_PRESSED);}
      virtual void   RButtonUp(const GG::Pt& pt, Uint32 keys)            {if (!Disabled()) SetState(BN_UNPRESSED);}
      virtual void   RClick(const GG::Pt& pt, Uint32 keys)               {if (!Disabled()) {SetState(BN_UNPRESSED); m_rclicked_sig(); SetState(BN_UNPRESSED);}}

    private:
      void Refresh();

      mutable ClickedSignalType m_rclicked_sig;

      double m_value;
      int m_decimals_to_show;
      bool m_show_sign;
      GG::Clr m_positive_color;
      GG::Clr m_negative_color;

      GG::SubTexture m_icon;

      GG::Clr m_border_color;
      int     m_border_thick;

      int     m_angled_corner_upperleft,m_angled_corner_upperright,m_angled_corner_lowerright,m_angled_corner_lowerleft;

      GG::Rect  m_icon_rect,m_text_rect;
  };

  CUIIconButton::CUIIconButton( int x, int y, int w, int h,const boost::shared_ptr<GG::Texture> &icon,const std::string& font_filename, 
                                int pts, GG::Clr color, GG::Clr border, int thick, GG::Clr text_color, Uint32 flags)
  :  Button(x, y, w, h, "", font_filename, pts, color, text_color, flags),
     m_border_color(border), m_border_thick(thick),m_angled_corner_upperleft(0),m_angled_corner_upperright(0),m_angled_corner_lowerright(0),m_angled_corner_lowerleft(0),
     m_icon(GG::SubTexture(icon,0,0,icon->Width()-1,icon->Height()-1)),
     m_value(0.0),m_decimals_to_show(0),m_show_sign(true),m_positive_color(text_color),m_negative_color(text_color),
     m_icon_rect(2,2,h-2,h-2),m_text_rect(m_icon_rect.LowerRight().x,2,w-2,h-2)
  {
    SetValue(0.0);
  }

  bool CUIIconButton::InWindow(const GG::Pt& pt) const
  {
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    return InAngledCornerRect(pt,ul.x, ul.y, lr.x, lr.y,m_angled_corner_upperleft,m_angled_corner_upperright,m_angled_corner_lowerright,m_angled_corner_lowerleft);    
  }

  void CUIIconButton::SetValue(double value) 
  {
    m_value = value;
    if(m_decimals_to_show)
    { 
      char buf[128];
      sprintf(buf, (m_show_sign ? "%+#.*g" : "%#.*g"), m_decimals_to_show, value);
      SetText(buf);
    } 
    else
      SetText((m_show_sign?(m_value < 0.0?"-":"+"):"") + boost::lexical_cast<std::string>(static_cast<int>(value)));
    
    SetTextColor(m_value < 0.0 ? m_negative_color : m_positive_color);
  }

  void CUIIconButton::RenderPressed()
  {
    OffsetMove(1, 1);
    RenderUnpressed();
    OffsetMove(-1, -1);
  }

  void CUIIconButton::RenderRollover()
  {
    RenderUnpressed();
  }

  void CUIIconButton::RenderUnpressed()
  { 
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Clr border_color_to_use = Disabled() ? DisabledColor(m_border_color) : m_border_color;

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y,color_to_use,border_color_to_use,m_border_thick,
                          m_angled_corner_upperleft,m_angled_corner_upperright,m_angled_corner_lowerright,m_angled_corner_lowerleft);    
    //glColor4ubv(Disabled() ? DisabledColor(m_color).v : m_color.v);
    
    glColor4ubv(GG::CLR_WHITE.v);
    m_icon.OrthoBlit(UpperLeft()+m_icon_rect.UpperLeft(),UpperLeft()+m_icon_rect.LowerRight(),false);

    std::string text; int x,y;GG::SubTexture icon;

    Uint32 format = TextFormat();
    
    glColor4ubv(TextColor().v);
    GetFont()->RenderText(UpperLeft()+m_text_rect.UpperLeft(),UpperLeft()+m_text_rect.LowerRight(), *this, format, 0, true);
 }

SidePanel::PlanetPanel::PlanetPanel(int x, int y, int w, int h,const Planet &planet)
: Wnd(0, y, w, h, GG::Wnd::CLICKABLE),
  m_planet_id(planet.ID()),
  m_planet_name(0),m_planet_info(0),
  m_button_colonize(0),
  m_construction(0),
  m_planet_graphic(0),
  m_button_food(0),m_button_mining(0),m_button_industry(0),m_button_research(0),m_button_balanced(0)
{
  Reset(planet);
}

void SidePanel::PlanetPanel::Reset()
{
  if(m_planet_id==UniverseObject::INVALID_OBJECT_ID)
    return;

  const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));
  
  Reset(*planet);  
}

void SidePanel::PlanetPanel::Reset(const Planet &planet)
{
  DeleteChildren();

  SetText(ClientUI::String("PLANET_PANEL"));

  GG::Clr owner_color(ClientUI::TEXT_COLOR);
  if(planet.Owners().size()>0)
    owner_color = HumanClientApp::Empires().Lookup(*planet.Owners().begin())->Color();

  m_planet_name = new GG::TextControl(MAX_PLANET_SIZE-15,10,planet.Name(),ClientUI::FONT,ClientUI::SIDE_PANEL_PLANET_NAME_PTS,owner_color);
  AttachChild(m_planet_name);

  GG::Pt ul = UpperLeft(), lr = LowerRight();
  int planet_image_sz = PlanetDiameter();
  GG::Pt planet_image_pos(MAX_PLANET_SIZE / 2 - planet_image_sz / 2, Height() / 2 - planet_image_sz / 2);

  std::vector<boost::shared_ptr<GG::Texture> > textures; int start_frame; double fps;

  GetPlanetTextures(planet,textures,start_frame=-1,fps=0.0);
  m_planet_graphic = new GG::DynamicGraphic(planet_image_pos.x,planet_image_pos.y,planet_image_sz,planet_image_sz,true,textures.back()->Width(),textures.back()->Height(),0,textures, GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
  if(start_frame==-1 && 1<textures.size())
    start_frame = RandSmallInt(0,textures.size()-1);

  if(start_frame!=-1 && fps!=0.0)
    m_planet_graphic->SetTimeIndex(start_frame * 1000.0 / m_planet_graphic->FPS());

  AttachChild(m_planet_graphic);
  m_planet_graphic->Play();

  if(planet.Owners().size()==0)  
    ConstructUnhabited(planet);
  else 
    if(!planet.OwnedBy(HumanClientApp::GetApp()->PlayerID()))     
      ConstructInhabited(planet);
    else
      ConstructOwned    (planet);

  MoveChildDown(m_planet_graphic);
  UpdateControls(planet);
}


void SidePanel::PlanetPanel::UpdateControls(const Planet &planet)
{
  if(planet.OwnedBy(HumanClientApp::GetApp()->PlayerID()))
  {
    m_button_food    ->SetValue(0.0);
    m_button_mining  ->SetValue(0.0);
    m_button_industry->SetValue(planet.ProdPoints());
    m_button_research->SetValue(0.0);

    m_button_food    ->SetColor((planet.PrimaryFocus()==Planet::BALANCED || planet.PrimaryFocus()==Planet::FARMING || planet.SecondaryFocus()==Planet::BALANCED || planet.SecondaryFocus()==Planet::FARMING)
                                ?GG::Clr(100,100,  0,200):GG::CLR_ZERO);
    m_button_mining  ->SetColor((planet.PrimaryFocus()==Planet::BALANCED || planet.PrimaryFocus()==Planet::MINING || planet.SecondaryFocus()==Planet::BALANCED || planet.SecondaryFocus()==Planet::MINING)
                                ?GG::Clr(100,  0,  0,200):GG::CLR_ZERO);
    m_button_industry->SetColor((planet.PrimaryFocus()==Planet::BALANCED || planet.PrimaryFocus()==Planet::INDUSTRY || planet.SecondaryFocus()==Planet::BALANCED || planet.SecondaryFocus()==Planet::INDUSTRY)
                                ?GG::Clr(  0,  0,100,200):GG::CLR_ZERO);
    m_button_research->SetColor((planet.PrimaryFocus()==Planet::BALANCED || planet.PrimaryFocus()==Planet::SCIENCE || planet.SecondaryFocus()==Planet::BALANCED || planet.SecondaryFocus()==Planet::SCIENCE)
                                ?GG::Clr(  0,100,  0,200):GG::CLR_ZERO);
    
    GG::Clr color;

    color = ClientUI::CTRL_BORDER_COLOR;
    if(planet.PrimaryFocus()==Planet::BALANCED || planet.PrimaryFocus()==Planet::FARMING)
      color = (planet.SecondaryFocus()==Planet::BALANCED || planet.SecondaryFocus()==Planet::FARMING)
             ?GG::CLR_WHITE:GG::Clr(255,255,0,255);
    m_button_food->SetBorderColor(color);

    color = ClientUI::CTRL_BORDER_COLOR;
    if(planet.PrimaryFocus()==Planet::BALANCED || planet.PrimaryFocus()==Planet::MINING)
      color = (planet.SecondaryFocus()==Planet::BALANCED || planet.SecondaryFocus()==Planet::MINING)
             ?GG::CLR_WHITE:GG::Clr(255,0,0,255);
    m_button_mining->SetBorderColor(color);

    color = ClientUI::CTRL_BORDER_COLOR;
    if(planet.PrimaryFocus()==Planet::BALANCED || planet.PrimaryFocus()==Planet::INDUSTRY)
      color = (planet.SecondaryFocus()==Planet::BALANCED || planet.SecondaryFocus()==Planet::INDUSTRY)
             ?GG::CLR_WHITE:GG::Clr(0,0,255,255);
    m_button_industry->SetBorderColor(color);

    color = ClientUI::CTRL_BORDER_COLOR;
    if(planet.PrimaryFocus()==Planet::BALANCED || planet.PrimaryFocus()==Planet::SCIENCE)
      color = (planet.SecondaryFocus()==Planet::BALANCED || planet.SecondaryFocus()==Planet::SCIENCE)
             ?GG::CLR_WHITE:GG::Clr(0,255,0,255);
    m_button_research->SetBorderColor(color);
  }
}

void SidePanel::PlanetPanel::SetPrimaryFocus(Planet::FocusType focus)
{
  Planet* planet = dynamic_cast<Planet*>(GetUniverse().Object(m_planet_id));

  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::Render: planet not found!");

  planet->SetPrimaryFocus(focus);
  UpdateControls(*planet);
}

void SidePanel::PlanetPanel::SetSecondaryFocus(Planet::FocusType focus)
{
  Planet* planet = dynamic_cast<Planet*>(GetUniverse().Object(m_planet_id));

  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::Render: planet not found!");

  planet->SetSecondaryFocus(focus);
  UpdateControls(*planet);
} 

void SidePanel::PlanetPanel::ConstructUnhabited(const Planet &planet)
{
  std::string text;
  text = GetPlanetSizeName(planet);
  if(text.length()>0)
    text+=" ";
  text+= GetPlanetTypeName(planet);
  
  text+="\n";
  if(planet.MaxPop()==0) text+= ClientUI::String("PL_UNHABITABLE");
  else                   text+= "("+ClientUI::String("PL_UNINHABITED")+" 0/"+boost::lexical_cast<std::string>(planet.MaxPop())+")";

  m_planet_info = new GG::TextControl(m_planet_name->UpperLeft().x-UpperLeft().x+10,m_planet_name->LowerRight().y-UpperLeft().y,text,ClientUI::FONT,ClientUI::SIDE_PANEL_PTS,ClientUI::TEXT_COLOR,GG::TF_LEFT | GG::TF_TOP);
  AttachChild(m_planet_info);

  if(planet.Owners().size()==0 && planet.MaxPop()>0 && FindColonyShip(planet.SystemID()))
  {
    m_button_colonize = new CUIButton((Width()/3)*2,(Height()-ClientUI::SIDE_PANEL_PTS)/2,60,ClientUI::String("PL_COLONIZE"),ClientUI::FONT,ClientUI::SIDE_PANEL_PTS,ClientUI::BUTTON_COLOR,ClientUI::CTRL_BORDER_COLOR,1,ClientUI::TEXT_COLOR,GG::Wnd::CLICKABLE);
    Connect(m_button_colonize->ClickedSignal(), &SidePanel::PlanetPanel::ClickColonize, this);
    AttachChild(m_button_colonize);
  }
}

void SidePanel::PlanetPanel::ConstructInhabited(const Planet &planet)
{
  std::string text;
  text = GetPlanetSizeName(planet);
  if(text.length()>0)
    text+=" ";
  text+= GetPlanetTypeName(planet);
  
  m_planet_info = new GG::TextControl(m_planet_name->UpperLeft().x-UpperLeft().x+10,m_planet_name->LowerRight().y-UpperLeft().y,text,ClientUI::FONT,ClientUI::SIDE_PANEL_PTS,ClientUI::TEXT_COLOR,GG::TF_LEFT|GG::TF_TOP);
  AttachChild(m_planet_info);
}

void SidePanel::PlanetPanel::ConstructOwned    (const Planet &planet)
{
  const int RESOURCE_DISPLAY_HEIGHT = 2*ClientUI::PTS;
  const int RESOURCE_DISPLAY_WIDTH  = (Width()-MAX_PLANET_SIZE/2)/6;
  const int RESOURCE_DISPLAY_MARGIN = 8;

  std::string text;
  text = GetPlanetSizeName(planet);
  if(text.length()>0)
    text+="\n";
  text+= GetPlanetTypeName(planet);
  
  m_planet_info = new GG::TextControl(m_planet_name->UpperLeft().x-UpperLeft().x+10,m_planet_name->LowerRight().y-UpperLeft().y,text,ClientUI::FONT,ClientUI::SIDE_PANEL_PTS,ClientUI::TEXT_COLOR,GG::TF_LEFT|GG::TF_TOP/*|GG::TF_LINEWRAP*/);
  AttachChild(m_planet_info);

  GG::Pt ul(Width()-2*RESOURCE_DISPLAY_WIDTH-2*RESOURCE_DISPLAY_MARGIN,m_planet_name->LowerRight().y-UpperLeft().y/*-(m_planet_name->LowerRight().y-m_planet_name->UpperLeft().y)/2*/);
  m_button_food     = new CUIIconButton(ul.x                                               ,ul.y                                                ,RESOURCE_DISPLAY_WIDTH,RESOURCE_DISPLAY_HEIGHT,IconFarming (),ClientUI::FONT,static_cast<int>(ClientUI::PTS*0.9),GG::CLR_ZERO);
  m_button_mining   = new CUIIconButton(ul.x+RESOURCE_DISPLAY_WIDTH+RESOURCE_DISPLAY_MARGIN,ul.y                                                ,RESOURCE_DISPLAY_WIDTH,RESOURCE_DISPLAY_HEIGHT,IconMining  (),ClientUI::FONT,static_cast<int>(ClientUI::PTS*0.9),GG::CLR_ZERO);
  m_button_research = new CUIIconButton(ul.x                                               ,ul.y+RESOURCE_DISPLAY_HEIGHT+RESOURCE_DISPLAY_MARGIN,RESOURCE_DISPLAY_WIDTH,RESOURCE_DISPLAY_HEIGHT,IconResearch(),ClientUI::FONT,static_cast<int>(ClientUI::PTS*0.9),GG::CLR_ZERO);
  m_button_industry = new CUIIconButton(ul.x+RESOURCE_DISPLAY_WIDTH+RESOURCE_DISPLAY_MARGIN,ul.y+RESOURCE_DISPLAY_HEIGHT+RESOURCE_DISPLAY_MARGIN,RESOURCE_DISPLAY_WIDTH,RESOURCE_DISPLAY_HEIGHT,IconIndustry(),ClientUI::FONT,static_cast<int>(ClientUI::PTS*0.9),GG::CLR_ZERO);
  m_button_balanced = new CUIIconButton(ul.x+RESOURCE_DISPLAY_WIDTH-6                      ,ul.y+RESOURCE_DISPLAY_HEIGHT-6                      ,19                    ,19                     ,IconBalance (),ClientUI::FONT,ClientUI::PTS                      ,GG::CLR_ZERO,GG::CLR_ZERO);

  boost::shared_ptr<GG::Texture>  icon = IconBalance ();
  m_button_balanced->SetIconRect(GG::Rect((-2),(-2),m_button_balanced->Width()-(-2),m_button_balanced->Height()-(-2)));

  m_button_food     ->SetAngledCornerLowerRight(7);
  m_button_mining   ->SetAngledCornerLowerLeft (7);
  m_button_research ->SetAngledCornerUpperRight(7);
  m_button_industry ->SetAngledCornerUpperLeft (7);

  m_button_balanced ->SetAngledCornerLowerRight(7);
  m_button_balanced ->SetAngledCornerLowerLeft (7);
  m_button_balanced ->SetAngledCornerUpperRight(7);
  m_button_balanced ->SetAngledCornerUpperLeft (7);

  m_button_food    ->SetPositiveColor(GG::CLR_GREEN);m_button_food    ->SetNegativeColor(GG::CLR_RED );
  //m_button_mining  ->SetPositiveColor(GG::CLR_ZERO );m_button_mining  ->SetNegativeColor(GG::CLR_ZERO);
  //m_button_research->SetPositiveColor(GG::CLR_ZERO );m_button_research->SetNegativeColor(GG::CLR_ZERO);
  //m_button_industry->SetPositiveColor(GG::CLR_ZERO );m_button_industry->SetNegativeColor(GG::CLR_ZERO);
  m_button_balanced->SetPositiveColor(GG::CLR_ZERO );m_button_balanced->SetNegativeColor(GG::CLR_ZERO);

  Connect(m_button_food    ->ClickedSignal (), &SidePanel::PlanetPanel::LClickFarming , this);
  Connect(m_button_food    ->RClickedSignal(), &SidePanel::PlanetPanel::RClickFarming , this);
  Connect(m_button_mining  ->ClickedSignal (), &SidePanel::PlanetPanel::LClickMining  , this);
  Connect(m_button_mining  ->RClickedSignal(), &SidePanel::PlanetPanel::RClickMining  , this);
  Connect(m_button_research->ClickedSignal (), &SidePanel::PlanetPanel::LClickResearch, this);
  Connect(m_button_research->RClickedSignal(), &SidePanel::PlanetPanel::RClickResearch, this);
  Connect(m_button_industry->ClickedSignal (), &SidePanel::PlanetPanel::LClickIndustry, this);
  Connect(m_button_industry->RClickedSignal(), &SidePanel::PlanetPanel::RClickIndustry, this);
  Connect(m_button_balanced->ClickedSignal (), &SidePanel::PlanetPanel::LClickBalanced, this);
  Connect(m_button_balanced->RClickedSignal(), &SidePanel::PlanetPanel::RClickBalanced, this);

  AttachChild(m_button_food);AttachChild(m_button_mining);AttachChild(m_button_industry);
  AttachChild(m_button_research);AttachChild(m_button_balanced);
  

  // for v.1 some of these only appear after tech is researched
  Empire *empire = HumanClientApp::Empires().Lookup( HumanClientApp::GetApp()->PlayerID() );

  m_construction = new CUIDropDownList(m_planet_name->UpperLeft ().x+10, 
                                       m_button_research->LowerRight().y-UpperLeft().y+10,
                                       100, ClientUI::SIDE_PANEL_PTS + 4, 
                                       (ClientUI::SIDE_PANEL_PTS + 4) * 5, GG::CLR_ZERO);


  m_construction->SetStyle(GG::LB_NOSORT);
  //m_construction->OffsetMove(0, -m_construction->Height());
  AttachChild(m_construction);

  ////////////////////// v0.1 only!! (in v0.2 and later build this list from the production capabilities of the planet)
  GG::ListBox::Row* row = new ConstructionRow(ProdCenter::NOT_BUILDING);
  row->push_back("No Building", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
  m_construction->Insert(row);

  row = new ConstructionRow(ProdCenter::INDUSTRY_BUILD);
  row->push_back("Industry", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
  m_construction->Insert(row);

  row = new ConstructionRow(ProdCenter::RESEARCH_BUILD);
  row->push_back("Research", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
  m_construction->Insert(row);

  row = new ConstructionRow(ProdCenter::SCOUT);
  row->push_back("Scout", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
  m_construction->Insert(row);

  row = new ConstructionRow(ProdCenter::COLONY_SHIP);
  row->push_back("Colony Ship", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
  m_construction->Insert(row);

  row = new ConstructionRow(ProdCenter::MARKI);
  row->push_back("MarkI", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
  m_construction->Insert(row);

  if( empire->HasTech( Tech::TECH_MARK2 ) )
  {
    row = new ConstructionRow(ProdCenter::MARKII);
    row->push_back("MarkII", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
    m_construction->Insert(row);
  }

  if( empire->HasTech( Tech::TECH_MARK3 ) )
  {
    row = new ConstructionRow(ProdCenter::MARKIII);
    row->push_back("MarkIII", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
    m_construction->Insert(row);
  }

  if( empire->HasTech( Tech::TECH_MARK4 ) )
  {
    row = new ConstructionRow(ProdCenter::MARKIV);
    row->push_back("MarkIV", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
    m_construction->Insert(row);
  }

  if( empire->HasTech( Tech::TECH_BASE ) )
  {
    row = new ConstructionRow(ProdCenter::DEF_BASE);
    row->push_back("DefBase", ClientUI::FONT, ClientUI::SIDE_PANEL_PTS, ClientUI::TEXT_COLOR);
    m_construction->Insert(row);
  }

  // find the index we need to set in the list 
  int selection_idx = -1;
  for(int i = 0; i< m_construction->NumRows();i++)
    if(static_cast<ConstructionRow&>(m_construction->GetRow(i)).m_build_type ==  planet.CurrentlyBuilding() )
       selection_idx =i;
  
  if(selection_idx!=-1)
    m_construction->Select( selection_idx );
  Connect(m_construction->SelChangedSignal(), &SidePanel::PlanetPanel::BuildSelected, this);
  ////////////////////// v0.1 only!!
}

SidePanel::PlanetPanel::~PlanetPanel()
{
  delete m_planet_graphic;
}

void SidePanel::PlanetPanel::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
  GG::Wnd *parent;
  if(parent=Parent())
    parent->MouseWheel(pt,move,keys);
}

bool SidePanel::PlanetPanel::InWindow(const GG::Pt& pt) const
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    ul.x += MAX_PLANET_SIZE / 2;
    return ((ul <= pt && pt < lr) || InPlanet(pt));
}

void SidePanel::PlanetPanel::LClick(const GG::Pt& pt, Uint32 keys) 
{
  if(InPlanet(pt))
	m_left_clicked_sig( m_planet_id );
}

bool SidePanel::PlanetPanel::RenderUnhabited(const Planet &planet)
{
  return true;
}

bool SidePanel::PlanetPanel::RenderInhabited(const Planet &planet)
{
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT,ClientUI::SIDE_PANEL_PTS);
  Uint32 format = GG::TF_LEFT | GG::TF_BOTTOM;

  std::string text; int x,y;

  x = m_planet_name->UpperLeft ().x+10;
  y = m_planet_name->LowerRight().y+ 5;

  //text = GetPlanetSizeName(planet);
  //font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0, false);
  y+=font->Height();

  //text = GetPlanetTypeName(planet);
  //font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0, false);
  y+=font->Height();

  int population=static_cast<int>(planet.PopPoints());

  const int RESOURCE_DISPLAY_HEIGHT = font->Height()+4;
  const int RESOURCE_DISPLAY_WIDTH  = (Width()-MAX_PLANET_SIZE/2)/5;
  boost::shared_ptr<GG::Texture> icon;
  const int ICON_MARGIN    =  5;
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.2));

  //population
  //x = m_planet_name->UpperLeft ().x+10; y = m_planet_name->LowerRight().y + RESOURCE_DISPLAY_HEIGHT+3;
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconPopulation(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  x+=font->Height();
  text = boost::lexical_cast<std::string>(population)+"/"+boost::lexical_cast<std::string>(planet.MaxPop());
  font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0, true);
  x+=font->TextExtent(text, format).x+ICON_MARGIN;

  return true;
}

bool SidePanel::PlanetPanel::RenderOwned(const Planet &planet)
{
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT,ClientUI::SIDE_PANEL_PTS);
  Uint32 format = GG::TF_LEFT | GG::TF_BOTTOM;

  std::string text; int x,y;

  x = m_planet_name->UpperLeft ().x+10;
  y = m_planet_name->LowerRight().y+ 5;

  //text = GetPlanetSizeName(planet);
  //font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0, false);
  y+=font->Height();

  //text = GetPlanetTypeName(planet);
  //font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0, false);
  y+=font->Height();


  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*0.9));
  text = ClientUI::String("PL_PRIMARY_FOCUS");
  switch(planet.PrimaryFocus())
  {
    case Planet::BALANCED : text+=" "+ClientUI::String("PL_BALANCED");break;
    case Planet::FARMING  : text+=" "+ClientUI::String("PL_FARMING" );break;
    case Planet::INDUSTRY : text+=" "+ClientUI::String("PL_INDUSTRY");break;
    case Planet::MINING   : text+=" "+ClientUI::String("PL_MINING"  );break;
    case Planet::SCIENCE  : text+=" "+ClientUI::String("PL_SCIENCE" );break;
  }
  font->RenderText(m_button_food->UpperLeft().x,
                   m_button_food->UpperLeft().y-font->Height(),
                   m_button_food->UpperLeft().x+ 500,
                   m_button_food->UpperLeft().y, text, format, 0, false);

  text = ClientUI::String("PL_SECONDARY_FOCUS");
  switch(planet.SecondaryFocus())
  {
    case Planet::BALANCED : text+=" "+ClientUI::String("PL_BALANCED");break;
    case Planet::FARMING  : text+=" "+ClientUI::String("PL_FARMING" );break;
    case Planet::INDUSTRY : text+=" "+ClientUI::String("PL_INDUSTRY");break;
    case Planet::MINING   : text+=" "+ClientUI::String("PL_MINING"  );break;
    case Planet::SCIENCE  : text+=" "+ClientUI::String("PL_SCIENCE" );break;
  }
  font->RenderText(m_button_research->UpperLeft ().x,
                   m_button_research->LowerRight().y,
                   m_button_research->UpperLeft ().x+ 500,
                   m_button_research->LowerRight().y+font->Height(), text, format, 0, false);

  int farming=0,mining=0,research=0,industry=0,defense=0,population=0;

  //farming   +=;
  //mining    +=;
  //research  +=;
  industry  +=static_cast<int>(planet.ProdPoints());;
  //defense   +=;
  population+=static_cast<int>(planet.PopPoints());


  const int RESOURCE_DISPLAY_HEIGHT = font->Height()+4;
  const int RESOURCE_DISPLAY_WIDTH  = (Width()-MAX_PLANET_SIZE/2)/5;
  boost::shared_ptr<GG::Texture> icon;
  const int ICON_MARGIN    =  5;
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.2));

  //population
  //x = m_planet_name->UpperLeft ().x+10; y = m_planet_name->LowerRight().y + RESOURCE_DISPLAY_HEIGHT+3;
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconPopulation(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  x+=font->Height();
  text = boost::lexical_cast<std::string>(population)+"/"+boost::lexical_cast<std::string>(planet.MaxPop());
  font->RenderText(x,y,x + 500, y+font->Height(), text, format, 0, true);
  x+=font->TextExtent(text, format).x+ICON_MARGIN;


  if(ProdCenter::SCOUT <= planet.CurrentlyBuilding())
  {
    // construction progress bar
    // TODO : get the costs of the item from the list of available technologies
    const int PROD_COSTS[] = {0, 0, 0, 50, 250, 100, 200, 375, 700, 200};
    int cost = PROD_COSTS[planet.CurrentlyBuilding()];
    double percent_complete = cost ? (planet.BuildProgress()+planet.Rollover()) / cost : 0.0;

    int x1 = m_construction->UpperLeft ().x;
    int x2 = m_construction->LowerRight().x;
    int y1,y2;
    y1 = m_construction->LowerRight().y;
    y2 = y1 + 5;
    GG::FlatRectangle(x1, y1, x2, y2, GG::CLR_ZERO, ClientUI::CTRL_BORDER_COLOR, 1);
    GG::FlatRectangle(x1, y1, x1 + static_cast<int>((x2 - x1 - 2) * percent_complete), y2,
                      ClientUI::SIDE_PANEL_BUILD_PROGRESSBAR_COLOR, LightColor(ClientUI::SIDE_PANEL_BUILD_PROGRESSBAR_COLOR), 1);

    // construction progress text
    font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
    format = GG::TF_LEFT | GG::TF_VCENTER;
    text = "";
    if(cost && !planet.ProdPoints()) text = ClientUI::String("PL_PRODUCTION_TIME_NEVER");
    else
      if(cost)
        text = Format(ClientUI::String("PL_PRODUCTION_TIME_TURNS").c_str(),static_cast<int>(std::ceil((cost - planet.BuildProgress()) / planet.ProdPoints())));

    x1 = m_construction->LowerRight().x;
    y1 = m_construction->UpperLeft ().y;
    y2 = m_construction->LowerRight().y;
    glColor4ubv(ClientUI::TEXT_COLOR.v);
    font->RenderText(x1, y1, x1+500, y2, text, format, 0, false);
  }

  return true;
}

bool SidePanel::PlanetPanel::Render()
{
  const Planet* planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));

  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::Render: planet not found!");

  if(planet->Owners().size()==0)  
    RenderUnhabited(*planet);
  else 
    if(!planet->OwnedBy(HumanClientApp::GetApp()->PlayerID()))     
      RenderInhabited(*planet);
    else
      RenderOwned    (*planet);
  return true;
}

int SidePanel::PlanetPanel::PlanetDiameter() const
{
  const Planet *planet = dynamic_cast<const
  Planet*>(GetUniverse().Object(m_planet_id));

  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::PlanetDiameter planet not found!");

  double scale=0.0;
  switch(planet->Size())
  {
    case Planet::SZ_TINY      : scale = 0.0/5.0; break;
    case Planet::SZ_SMALL     : scale = 1.0/5.0; break;
    case Planet::SZ_MEDIUM    : scale = 2.0/5.0; break;
    case Planet::SZ_LARGE     : scale = 3.0/5.0; break;
    case Planet::SZ_HUGE      : scale = 4.0/5.0; break;
    case Planet::SZ_GASGIANT  : scale = 5.0/5.0; break;
    case Planet::SZ_ASTEROIDS : scale = 5.0/5.0; break;
    default                   : scale = 2.0/5.0; break;
  }

  return MIN_PLANET_SIZE + (MAX_PLANET_SIZE - MIN_PLANET_SIZE)*scale;
}

bool SidePanel::PlanetPanel::InPlanet(const GG::Pt& pt) const
{
    GG::Pt center = UpperLeft() + GG::Pt(MAX_PLANET_SIZE / 2, Height() / 2);
    GG::Pt diff = pt - center;
    int r_squared = PlanetDiameter() * PlanetDiameter() / 4;
    return diff.x * diff.x + diff.y * diff.y <= r_squared;
}

void SidePanel::PlanetPanel::BuildSelected(int idx) const
{
  const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));
  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::BuildSelected planet not found!");

  HumanClientApp::Orders().IssueOrder(new PlanetBuildOrder(*planet->Owners().begin(), planet->ID(),static_cast<ConstructionRow&>(m_construction->GetRow(idx)).m_build_type));
}

void SidePanel::PlanetPanel::ClickColonize()
{
  const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));
  Ship *ship=FindColonyShip(planet->SystemID());
  if(ship==0)
    throw std::runtime_error("SidePanel::PlanetPanel::ClickColonize ship not found!");

  HumanClientApp::Orders().IssueOrder(new FleetColonizeOrder( HumanClientApp::GetApp()->PlayerID(), ship->GetFleet()->ID(), planet->ID() ));

  Reset(*planet);
}

void SidePanel::PlanetPanel::RClick(const GG::Pt& pt, Uint32 keys)
{
  const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));

  GG::MenuItem menu_contents;
  menu_contents.next_level.push_back(GG::MenuItem("Rename Planet", 1, false, false));
  GG::PopupMenu popup(pt.x, pt.y, GG::App::GetApp()->GetFont(ClientUI::FONT, ClientUI::PTS), menu_contents, ClientUI::TEXT_COLOR);

  if(popup.Run()) 
    switch (popup.MenuID())
    {
      case 1: 
      { // rename planet
        std::string plt_name = planet->Name();
        CUIEditWnd edit_wnd(350, "Enter new planet name", plt_name);
        edit_wnd.Run();
        if(edit_wnd.Result() != "")
        {
          HumanClientApp::Orders().IssueOrder(new RenameOrder(HumanClientApp::GetApp()->PlayerID(), planet->ID(), edit_wnd.Result()));
          m_planet_name->SetText(planet->Name());
        }
        break;
      }
      default:
        break;
    }
}

 ////////////////////////////////////////////////
// SidePanel::PlanetPanelContainer
////////////////////////////////////////////////
SidePanel::PlanetPanelContainer::PlanetPanelContainer(int x, int y, int w, int h)
: Wnd(x-MAX_PLANET_SIZE/2, y, w+MAX_PLANET_SIZE/2, h, GG::Wnd::CLICKABLE),m_planet_panels(),
  m_vscroll(new CUIScroll(Width()-10,0,10,Height(),GG::Scroll::VERTICAL))
{
  EnableChildClipping(true);
  AttachChild(m_vscroll);
  Connect(m_vscroll->ScrolledSignal(), &SidePanel::PlanetPanelContainer::VScroll,this);
}

bool SidePanel::PlanetPanelContainer::InWindow(const GG::Pt& pt) const
{
  if(pt.y<UpperLeft().y)
    return false;

  bool retval = UpperLeft()+GG::Pt(MAX_PLANET_SIZE/2,0) <= pt && pt < LowerRight();
  for(unsigned int i = 0; i < m_planet_panels.size() && !retval; ++i)
    if(m_planet_panels[i]->InWindow(pt))
      retval = true;

  return retval;
}
void SidePanel::PlanetPanelContainer::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
  if(m_vscroll)
    move<0?m_vscroll->ScrollLineIncr():m_vscroll->ScrollLineDecr();
}

void SidePanel::PlanetPanelContainer::Clear()
{
  m_planet_panels.clear();

  DetachChild(m_vscroll);
  DeleteChildren();
  AttachChild(m_vscroll);

}

void SidePanel::PlanetPanelContainer::SetPlanets(const std::vector<const Planet*> &plt_vec)
{
  Clear();

  int y = 0;
  const int PLANET_PANEL_HT = MAX_PLANET_SIZE;
  for (unsigned int i = 0; i < plt_vec.size(); ++i, y += PLANET_PANEL_HT) 
  {
    const Planet* planet = plt_vec[i];
    PlanetPanel* planet_panel = new PlanetPanel(0, y, Width()-m_vscroll->Width(), PLANET_PANEL_HT,*planet);
    AttachChild(planet_panel);
    m_planet_panels.push_back(planet_panel);
  }
  m_vscroll->SizeScroll(0,plt_vec.size()*PLANET_PANEL_HT,PLANET_PANEL_HT,Height());
}

void SidePanel::PlanetPanelContainer::VScroll(int from,int to,int range_min,int range_max)
{
  int y = -from;
  const int PLANET_PANEL_HT = MAX_PLANET_SIZE;
  for (unsigned int i = 0; i < m_planet_panels.size(); ++i, y += PLANET_PANEL_HT)
    m_planet_panels[i]->MoveTo(UpperLeft().x-m_planet_panels[i]->UpperLeft().x,y);
}

////////////////////////////////////////////////
// SidePanel::SystemResourceSummary
////////////////////////////////////////////////
SidePanel::SystemResourceSummary::SystemResourceSummary(int x, int y, int w, int h)
: Wnd(x, y, w, h, GG::Wnd::CLICKABLE),
  m_farming(0),m_mining(0),m_research(0),m_industry(0),m_defense(0)
{
}

bool SidePanel::SystemResourceSummary::Render()
{
  int farming=m_farming,mining=m_mining,research=m_research,industry=m_industry,defense=m_defense;

  std::string text; int x,y; boost::shared_ptr<GG::Texture> icon;
  boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.2));
  Uint32 format = GG::TF_LEFT | GG::TF_VCENTER;
  const int HALF_FONT_HT   = font->Height()/2;
  const int ICON_MARGIN    =  5;
  
  x=UpperLeft().x;y=UpperLeft().y;

  int info_elem_width = (Width()-(5+1)*ICON_MARGIN)/5;

  //farming
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconFarming(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = (farming<0?"-":"+") + boost::lexical_cast<std::string>(farming);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0, true);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;

  //mining
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconMining(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = (mining<0?"-":"+") + boost::lexical_cast<std::string>(mining);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0, true);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;

  //research
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconResearch(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = (research<0?"-":"+") + boost::lexical_cast<std::string>(research);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0, true);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;

  //industy
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconIndustry(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = (industry<0?"-":"+") + boost::lexical_cast<std::string>(industry);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0, true);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;

  //defense
  glColor4ubv(ClientUI::TEXT_COLOR.v);
  icon=IconDefense(); icon->OrthoBlit(x,y,x+font->Height(),y+font->Height(), 0, false);
  //x+=font->Height();
  text = boost::lexical_cast<std::string>(defense)+"/"+boost::lexical_cast<std::string>(defense*3);
  font->RenderText(x+font->Height(),y,x + 500, y+Height(), text, format, 0, true);
  //x+=font->TextExtent(text, format).x+ICON_MARGIN;
  x+=info_elem_width+ICON_MARGIN;

  return true;
}

////////////////////////////////////////////////
// SidePanel
////////////////////////////////////////////////
SidePanel::SidePanel(int x, int y, int w, int h) : 
    Wnd(x, y, w, h, GG::Wnd::CLICKABLE),
    m_system(0),
    m_star_graphic(0),
    m_system_name_unknown(new GG::TextControl(40, 0, w-80,SYSTEM_NAME_FONT_SIZE,ClientUI::String("SP_UNKNOWN_SYSTEM"),ClientUI::FONT,static_cast<int>(ClientUI::PTS*1.4),0,ClientUI::TEXT_COLOR)),
    m_system_name(new CUIDropDownList(40, 0, w-80,SYSTEM_NAME_FONT_SIZE, 10*SYSTEM_NAME_FONT_SIZE,GG::CLR_ZERO,GG::Clr(0, 0, 0, 0),ClientUI::SIDE_PANEL_COLOR)),
    m_button_prev(new GG::Button(40-SYSTEM_NAME_FONT_SIZE,0,SYSTEM_NAME_FONT_SIZE,SYSTEM_NAME_FONT_SIZE,"",ClientUI::FONT,SYSTEM_NAME_FONT_SIZE,GG::CLR_WHITE,GG::Wnd::CLICKABLE)),
    m_button_next(new GG::Button(40+w-80                 ,0,SYSTEM_NAME_FONT_SIZE,SYSTEM_NAME_FONT_SIZE,"",ClientUI::FONT,SYSTEM_NAME_FONT_SIZE,GG::CLR_WHITE,GG::Wnd::CLICKABLE)),
    m_static_text_systemproduction(new GG::TextControl(0,100-20-ClientUI::PTS-5,ClientUI::String("SP_SYSTEM_PRODUCTION"),ClientUI::FONT,ClientUI::PTS,ClientUI::TEXT_COLOR)),
    m_system_resource_summary(new SystemResourceSummary(0,100-20,w,20)),
    m_planet_panel_container(new PlanetPanelContainer(0,100,w,h-100-30))
{
  SetText(ClientUI::String("SIDE_PANEL"));

  m_system_name->DisableDropArrow();
  m_system_name->SetStyle(GG::LB_CENTER);

  m_button_prev->SetUnpressedGraphic(GG::SubTexture(GG::App::GetApp()->GetTexture( ClientUI::ART_DIR + "icons/leftarrownormal.png"   ), 0, 0, 32, 32));
  m_button_prev->SetPressedGraphic  (GG::SubTexture(GG::App::GetApp()->GetTexture( ClientUI::ART_DIR + "icons/leftarrowclicked.png"  ), 0, 0, 32, 32));
  m_button_prev->SetRolloverGraphic (GG::SubTexture(GG::App::GetApp()->GetTexture( ClientUI::ART_DIR + "icons/leftarrowmouseover.png"), 0, 0, 32, 32));

  m_button_next->SetUnpressedGraphic(GG::SubTexture(GG::App::GetApp()->GetTexture( ClientUI::ART_DIR + "icons/rightarrownormal.png"  ), 0, 0, 32, 32));
  m_button_next->SetPressedGraphic  (GG::SubTexture(GG::App::GetApp()->GetTexture( ClientUI::ART_DIR + "icons/rightarrowclicked.png"   ), 0, 0, 32, 32));
  m_button_next->SetRolloverGraphic (GG::SubTexture(GG::App::GetApp()->GetTexture( ClientUI::ART_DIR + "icons/rightarrowmouseover.png"), 0, 0, 32, 32));

  AttachChild(m_system_name_unknown);
  AttachChild(m_system_name);
  AttachChild(m_button_prev);
  AttachChild(m_button_next);
  AttachChild(m_static_text_systemproduction);
  AttachChild(m_system_resource_summary);
  AttachChild(m_planet_panel_container);

  GG::Connect(m_system_name->SelChangedSignal(), &SidePanel::SystemSelectionChanged, this);
  GG::Connect(m_button_prev->ClickedSignal(), &SidePanel::PrevButtonClicked, this);
  GG::Connect(m_button_next->ClickedSignal(), &SidePanel::NextButtonClicked, this);

  Hide();
}

bool SidePanel::InWindow(const GG::Pt& pt) const
{
  return (UpperLeft() <= pt && pt < LowerRight()) || m_planet_panel_container->InWindow(pt);
}

bool SidePanel::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::SIDE_PANEL_COLOR, GG::CLR_ZERO, 0);
    return true;
}

void SidePanel::SystemSelectionChanged(int selection)
{
  int system_id = UniverseObject::INVALID_OBJECT_ID;

  if(0<= selection && selection<m_system_name->NumRows())
    system_id = static_cast<const SystemRow&>(m_system_name->GetRow(selection)).m_system_id;

  if(SystemID()!=system_id)
    SetSystem(system_id);
}

void SidePanel::PrevButtonClicked()
{
  int selected = m_system_name->CurrentItemIndex();

  if(0< selected && selected<m_system_name->NumRows())
    m_system_name->Select(selected-1);
}

void SidePanel::NextButtonClicked()
{
  int selected = m_system_name->CurrentItemIndex();

  if(0<=selected && selected<m_system_name->NumRows()-1)
    m_system_name->Select(selected+1);
}

int SidePanel::SystemID() const {return m_system!=0?m_system->ID():UniverseObject::INVALID_OBJECT_ID;}

void SidePanel::SetSystem(int system_id)
{
    m_fleet_icons.clear();
    m_planet_panel_container->Clear();
    m_system_name->Clear();

    DeleteChild(m_star_graphic);m_star_graphic=0;

    Hide();

    m_system = dynamic_cast<const System*>(HumanClientApp::GetUniverse().Object(system_id));

    if (m_system)
    {
      std::vector<const System*> sys_vec = GetUniverse().FindObjects<const System>();
      GG::ListBox::Row *select_row=0;

      for (unsigned int i = 0; i < sys_vec.size(); i++) 
      {
        GG::ListBox::Row *row = new SystemRow(sys_vec[i]->ID());

        if(sys_vec[i]->Name().length()==0)
          continue;
 
        row->push_back(Format(ClientUI::String("SP_SYSTEM_NAME").c_str(),sys_vec[i]->Name().c_str()), ClientUI::FONT,static_cast<int>(ClientUI::PTS*1.4), ClientUI::TEXT_COLOR);
        m_system_name->Insert(row);

        if(sys_vec[i]->ID() == system_id)
          select_row = row;
      }

      for (int i = 0; i < m_system_name->NumRows(); i++) 
        if(select_row == &m_system_name->GetRow(i))
        {
          m_system_name->Select(i);
          break;
        }

      std::vector<boost::shared_ptr<GG::Texture> > textures;
      boost::shared_ptr<GG::Texture> graphic;
     
      std::string star_image = ClientUI::ART_DIR + "stars/";
      switch (m_system->Star())
      {
        case System::BLUE     : star_image += "blue2"; break;
        case System::WHITE    : star_image += "yellow1"; break;
        case System::YELLOW   : star_image += "yellow2"; break;
        case System::ORANGE   : star_image += "red4"; break;
        case System::RED      : star_image += "red3"; break;
        case System::NEUTRON  : star_image += "blue2"; break;
        case System::BLACK    : star_image += "blue2"; break;
        default               : star_image += "blue2"; break;
      }
      star_image += ".png";
      graphic = HumanClientApp::GetApp()->GetTexture(star_image);
      
      textures.push_back(graphic);

      m_star_graphic = new GG::DynamicGraphic((Width()*1)/3,-(Width()*1)/3,Width(),Width(),true,textures.back()->Width(),textures.back()->Height(),0,textures, GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);

      AttachChild(m_star_graphic);MoveChildDown(m_star_graphic);

      // TODO: add fleet icons
      std::vector<const Fleet*> flt_vec = m_system->FindObjects<Fleet>();
      for(unsigned int i = 0; i < flt_vec.size(); i++) 
        GG::Connect(flt_vec[i]->StateChangedSignal(), &SidePanel::FleetsChanged, this);

      // add planets
      std::vector<const Planet*> plt_vec = m_system->FindObjects<Planet>();

      m_planet_panel_container->SetPlanets(plt_vec);
      for(unsigned int i = 0; i < plt_vec.size(); i++) 
        GG::Connect(plt_vec[i]->StateChangedSignal(), &SidePanel::PlanetsChanged, this);

      Show();PlanetsChanged();
      if(select_row==0)
      {
        m_system_name_unknown->Show();
        m_system_name->Hide();
        m_button_prev->Hide();
        m_button_next->Hide();
      }
      else
      {
        m_system_name_unknown->Hide();
        m_system_name->Show();
        m_button_prev->Show();
        m_button_next->Show();
      }
    }
}

void SidePanel::FleetsChanged()
{
  for(int i=0;i<m_planet_panel_container->PlanetPanels();i++)
    m_planet_panel_container->GetPlanetPanel(i)->Reset();
}

void SidePanel::PlanetsChanged()
{
  if(m_system)
  {
    std::vector<const Planet*> plt_vec = m_system->FindObjects<Planet>();
    int farming=0,mining=0,research=0,industry=0,defense=0,num_empire_planets=0;

    for(unsigned int i=0;i<plt_vec.size();i++)
    {
      //farming   +=;
      //mining    +=;
      //research  +=;
      industry  +=static_cast<int>(plt_vec[i]->ProdPoints());;
      defense   +=plt_vec[i]->DefBases();
      
      if(plt_vec[i]->Owners().find(HumanClientApp::GetApp()->PlayerID()) != plt_vec[i]->Owners().end())
        num_empire_planets++;
    }

    m_system_resource_summary->SetFarming (farming );
    m_system_resource_summary->SetMining  (mining  );
    m_system_resource_summary->SetResearch(research);
    m_system_resource_summary->SetIndustry(industry);
    m_system_resource_summary->SetDefense (defense );

    if(num_empire_planets==0)
    {
      m_system_resource_summary->Hide();
      m_static_text_systemproduction->Hide();
    }
    else
    {
      m_system_resource_summary->Show();
      m_static_text_systemproduction->Show();
    }

  }
}
