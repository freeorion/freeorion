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
#include "../util/OptionsDB.h"

#include "MapWnd.h"

#include <io.h>

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

  boost::shared_ptr<GG::Texture> IconBalance   () {return GetTexture(ClientUI::ART_DIR + "icons/focusbuttoncrossicon.png"        );}
  boost::shared_ptr<GG::Texture> IconPopulation() {return GetTexture(ClientUI::ART_DIR + "icons/pop.png"        );}
  boost::shared_ptr<GG::Texture> IconIndustry  () {return GetTexture(ClientUI::ART_DIR + "icons/industry.png"   );}
  boost::shared_ptr<GG::Texture> IconResearch  () {return GetTexture(ClientUI::ART_DIR + "icons/research.png"   );}
  boost::shared_ptr<GG::Texture> IconMining    () {return GetTexture(ClientUI::ART_DIR + "icons/mining.png"     );}
  boost::shared_ptr<GG::Texture> IconFarming   () {return GetTexture(ClientUI::ART_DIR + "icons/farming.png"    );}
  boost::shared_ptr<GG::Texture> IconDefense   () {return GetTexture(ClientUI::ART_DIR + "icons/defensebase.png");}

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
      return GetTexture(ClientUI::ART_DIR + "planets/terran1.png");
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

  int GetPlanetTexturesDynamicLights(const Planet &planet,std::vector<boost::shared_ptr<GG::Texture> > &textures, int &start_frame, double &fps)
  {
    GG::XMLDoc planetart_doc;
    std::ifstream ifs((ClientUI::ART_DIR + "planets/planets.xml").c_str());
    planetart_doc.ReadDoc(ifs);
    ifs.close();

    std::string plt_art_node_name = "ArtPlanets.Lights";

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
              case Planet::SZ_GASGIANT  : plt_size_name = "Gasgigant"; break;
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
      case Planet::PT_GASGIANT  : plt_art_node_name += "Gasgiant"  ; break;    
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
              case Planet::SZ_GASGIANT  : plt_size_name = "Gasgigant"; break;
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
      case Planet::SZ_ASTEROIDS : return ""; //ClientUI::String("PL_SZ_ASTEROIDS"); break;
      case Planet::SZ_GASGIANT  : return ""; //ClientUI::String("PL_SZ_GASGIANT"); break;
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
    if(!(color==GG::CLR_ZERO))
    {
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
    }

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
   m_icon_rect(1+m_border_thick,1+m_border_thick,h-(1+m_border_thick),h-(1+m_border_thick)),m_text_rect(m_icon_rect.LowerRight().x,2,w-2,h-2)
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
SidePanel::PlanetPanel::PlanetPanel(int x, int y, int w, int h,const Planet &planet)
: Wnd(0, y, w, h, GG::Wnd::CLICKABLE),
  m_planet_id(planet.ID()),
  m_planet_name(0),m_planet_info(0),
  m_button_colonize(0),
  m_construction(0),
  m_planet_graphic(0),m_planet_graphic_lights(0),
  m_button_food(0),m_button_mining(0),m_button_industry(0),m_button_research(0),m_button_balanced(0)
{
  SetText(ClientUI::String("PLANET_PANEL"));

  GG::Clr owner_color(ClientUI::TEXT_COLOR);
  if(planet.Owners().size()>0)
    owner_color = HumanClientApp::Empires().Lookup(*planet.Owners().begin())->Color();

  m_planet_name = new GG::TextControl(MAX_PLANET_SIZE-15,10,planet.Name(),ClientUI::FONT,ClientUI::SIDE_PANEL_PLANET_NAME_PTS,ClientUI::TEXT_COLOR);
  AttachChild(m_planet_name);

  GG::Pt ul = UpperLeft(), lr = LowerRight();
  int planet_image_sz = PlanetDiameter();
  GG::Pt planet_image_pos(MAX_PLANET_SIZE / 2 - planet_image_sz / 2, Height() / 2 - planet_image_sz / 2);

  std::vector<boost::shared_ptr<GG::Texture> > textures; int start_frame; double fps;

  GetPlanetTextures(planet,textures,start_frame=-1,fps=0.0);
  m_planet_graphic = new GG::DynamicGraphic(planet_image_pos.x,planet_image_pos.y,planet_image_sz,planet_image_sz,true,64,64,0,textures, GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
  if(start_frame==-1 && 1<textures.size())
    start_frame = RandSmallInt(0,textures.size()-1);

  if(start_frame!=-1 && fps!=0.0)
    m_planet_graphic->SetTimeIndex(start_frame * 1000.0 / m_planet_graphic->FPS());
  
  //lights
  
  textures.clear();
  GetPlanetTexturesDynamicLights(planet,textures,start_frame=-1,fps=0.0);
  m_planet_graphic_lights = new GG::DynamicGraphic(planet_image_pos.x,planet_image_pos.y,planet_image_sz,planet_image_sz,true,textures.back()->Width(),textures.back()->Height(),0,textures, GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
  if(start_frame==-1 && 1<textures.size())
    start_frame = RandSmallInt(0,textures.size()-1);

  if(start_frame!=-1 && fps!=0.0)
    m_planet_graphic_lights->SetTimeIndex(start_frame * 1000.0 / m_planet_graphic_lights->FPS());

  AttachChild(m_planet_graphic);m_planet_graphic->Play();
  AttachChild(m_planet_graphic_lights);m_planet_graphic_lights->Play();

  m_planet_info = new GG::TextControl(m_planet_name->UpperLeft().x-UpperLeft().x+10,m_planet_name->LowerRight().y-UpperLeft().y,"",ClientUI::FONT,ClientUI::SIDE_PANEL_PTS,ClientUI::TEXT_COLOR,GG::TF_LEFT|GG::TF_TOP);
  AttachChild(m_planet_info);

  m_button_colonize = new CUIButton((Width()/3)*2,(Height()-ClientUI::SIDE_PANEL_PTS)/2,60,ClientUI::String("PL_COLONIZE"),ClientUI::FONT,ClientUI::SIDE_PANEL_PTS,ClientUI::BUTTON_COLOR,ClientUI::CTRL_BORDER_COLOR,1,ClientUI::TEXT_COLOR,GG::Wnd::CLICKABLE);
  Connect(m_button_colonize->ClickedSignal(), &SidePanel::PlanetPanel::ClickColonize, this);
  AttachChild(m_button_colonize);

  const int RESOURCE_DISPLAY_HEIGHT = 2*ClientUI::PTS;
  const int RESOURCE_DISPLAY_WIDTH  = (Width()-MAX_PLANET_SIZE/2)/6;
  const int RESOURCE_DISPLAY_MARGIN = 8;

  ul=GG::Pt(Width()-2*RESOURCE_DISPLAY_WIDTH-2*RESOURCE_DISPLAY_MARGIN,m_planet_name->LowerRight().y-UpperLeft().y/*-(m_planet_name->LowerRight().y-m_planet_name->UpperLeft().y)/2*/);
  m_button_food     = new CUIIconButton(ul.x                                               ,ul.y                                                ,RESOURCE_DISPLAY_WIDTH,RESOURCE_DISPLAY_HEIGHT,IconFarming (),ClientUI::FONT,static_cast<int>(ClientUI::PTS*0.9),GG::CLR_ZERO,ClientUI::CTRL_BORDER_COLOR,2);
  m_button_mining   = new CUIIconButton(ul.x+RESOURCE_DISPLAY_WIDTH+RESOURCE_DISPLAY_MARGIN,ul.y                                                ,RESOURCE_DISPLAY_WIDTH,RESOURCE_DISPLAY_HEIGHT,IconMining  (),ClientUI::FONT,static_cast<int>(ClientUI::PTS*0.9),GG::CLR_ZERO,ClientUI::CTRL_BORDER_COLOR,2);
  m_button_research = new CUIIconButton(ul.x                                               ,ul.y+RESOURCE_DISPLAY_HEIGHT+RESOURCE_DISPLAY_MARGIN,RESOURCE_DISPLAY_WIDTH,RESOURCE_DISPLAY_HEIGHT,IconResearch(),ClientUI::FONT,static_cast<int>(ClientUI::PTS*0.9),GG::CLR_ZERO,ClientUI::CTRL_BORDER_COLOR,2);
  m_button_industry = new CUIIconButton(ul.x+RESOURCE_DISPLAY_WIDTH+RESOURCE_DISPLAY_MARGIN,ul.y+RESOURCE_DISPLAY_HEIGHT+RESOURCE_DISPLAY_MARGIN,RESOURCE_DISPLAY_WIDTH,RESOURCE_DISPLAY_HEIGHT,IconIndustry(),ClientUI::FONT,static_cast<int>(ClientUI::PTS*0.9),GG::CLR_ZERO,ClientUI::CTRL_BORDER_COLOR,2);
  m_button_balanced = new CUIIconButton(ul.x+RESOURCE_DISPLAY_WIDTH-6                      ,ul.y+RESOURCE_DISPLAY_HEIGHT-6                      ,19                    ,19                     ,IconBalance (),ClientUI::FONT,ClientUI::PTS                      ,GG::CLR_ZERO,GG::CLR_ZERO               ,2);

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
                                       100, ClientUI::SIDE_PANEL_PTS, 
                                       (ClientUI::SIDE_PANEL_PTS) * 5, GG::CLR_ZERO);

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

  MoveChildDown(m_planet_graphic);

  const Planet *plt = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));

  m_connection_planet_changed = GG::Connect(plt->StateChangedSignal(), &SidePanel::PlanetPanel::PlanetChanged, this);
  m_connection_planet_production_changed= GG::Connect(plt->ProdCenterChangedSignal(), &SidePanel::PlanetPanel::PlanetProdCenterChanged, this);

  Update();
}

SidePanel::PlanetPanel::~PlanetPanel()
{
  for(unsigned int i=0;i<m_vec_unused_controls.size();i++)
    delete m_vec_unused_controls[i];
  m_vec_unused_controls.clear();
}

Planet* SidePanel::PlanetPanel::GetPlanet()
{
  Planet *planet = dynamic_cast<Planet*>(GetUniverse().Object(m_planet_id));
  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::GetPlanet: planet not found!");
  return planet;
}

const Planet* SidePanel::PlanetPanel::GetPlanet() const
{
  const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));
  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::GetPlanet: planet not found!");
  return planet;
}

void SidePanel::PlanetPanel::Update()
{
  PlanetChanged();PlanetProdCenterChanged();
}

void SidePanel::PlanetPanel::EnableControl(GG::Wnd *control,bool enable)
{
  std::vector<GG::Wnd*>::iterator it = std::find(m_vec_unused_controls.begin(),m_vec_unused_controls.end(),control);
  if(it != m_vec_unused_controls.end())
  {
    if(enable)
    {
      m_vec_unused_controls.erase(it);
      AttachChild(control);
      control->Show();
    }
  }
  else
  {
    if(!enable)
    {
      m_vec_unused_controls.push_back(control);
      DetachChild(control);
      control->Hide();
    }
  }
}

void SidePanel::PlanetPanel::PlanetChanged()
{
  const Planet *planet = GetPlanet();

  enum OWNERSHIP {OS_NONE,OS_FOREIGN,OS_SELF} owner = OS_NONE;

  std::string text;
  if(planet->Owners().size()==0) 
  { //uninhabited
    owner = OS_NONE;
    text = GetPlanetSizeName(*planet);
    if(text.length()>0)
      text+=" ";
    text+= GetPlanetTypeName(*planet);
  
    text+="\n";
    if(planet->MaxPop()==0) text+= ClientUI::String("PL_UNHABITABLE");
    else                    text+= "("+ClientUI::String("PL_UNINHABITED")+" 0/"+boost::lexical_cast<std::string>(planet->MaxPop())+")";

    m_planet_info->SetText(text);
  }
  else 
    if(!planet->OwnedBy(HumanClientApp::GetApp()->PlayerID()))
    {//inhabited
      owner = OS_FOREIGN;
      text = GetPlanetSizeName(*planet);
      if(text.length()>0)
        text+=" ";
      text+= GetPlanetTypeName(*planet);
    
      m_planet_info->SetText(text);
    }
    else
    {//Owned
      owner = OS_SELF;
      text = GetPlanetSizeName(*planet);
      if(text.length()>0)
        text+="\n";
      text+= GetPlanetTypeName(*planet);
      
      m_planet_info->SetText(text);
    }

  m_planet_name->SetTextColor(planet->Owners().size()>0?HumanClientApp::Empires().Lookup(*(planet->Owners().begin()))->Color():ClientUI::TEXT_COLOR);

  // visibility
  EnableControl(m_button_colonize,(owner==OS_NONE && planet->MaxPop()>0 && FindColonyShip(planet->SystemID())));
  EnableControl(m_button_food    ,(owner==OS_SELF));
  EnableControl(m_button_mining  ,(owner==OS_SELF));
  EnableControl(m_button_industry,(owner==OS_SELF));
  EnableControl(m_button_research,(owner==OS_SELF));
  EnableControl(m_button_balanced,(owner==OS_SELF));
  EnableControl(m_construction   ,(owner==OS_SELF));
}

void SidePanel::PlanetPanel::PlanetProdCenterChanged()
{
  const Planet *planet = GetPlanet();

  int i;
  for(i=0; i< m_construction->NumRows();i++)
    if(static_cast<ConstructionRow&>(m_construction->GetRow(i)).m_build_type ==  planet->CurrentlyBuilding() )
    {
      m_construction->Select(i);
      break;
    }

  m_button_food    ->SetValue(0.0);
  m_button_mining  ->SetValue(0.0);
  m_button_industry->SetValue(planet->ProdPoints());
  m_button_research->SetValue(0.0);

  m_button_food    ->SetColor((planet->PrimaryFocus()==Planet::BALANCED || planet->PrimaryFocus()==Planet::FARMING || planet->SecondaryFocus()==Planet::BALANCED || planet->SecondaryFocus()==Planet::FARMING )?GG::Clr(100,100,  0,200):GG::CLR_ZERO);
  m_button_mining  ->SetColor((planet->PrimaryFocus()==Planet::BALANCED || planet->PrimaryFocus()==Planet::MINING  || planet->SecondaryFocus()==Planet::BALANCED || planet->SecondaryFocus()==Planet::MINING  )?GG::Clr(100,  0,  0,200):GG::CLR_ZERO);
  m_button_industry->SetColor((planet->PrimaryFocus()==Planet::BALANCED || planet->PrimaryFocus()==Planet::INDUSTRY|| planet->SecondaryFocus()==Planet::BALANCED || planet->SecondaryFocus()==Planet::INDUSTRY)?GG::Clr(  0,  0,100,200):GG::CLR_ZERO);
  m_button_research->SetColor((planet->PrimaryFocus()==Planet::BALANCED || planet->PrimaryFocus()==Planet::SCIENCE || planet->SecondaryFocus()==Planet::BALANCED || planet->SecondaryFocus()==Planet::SCIENCE )?GG::Clr(  0,100,  0,200):GG::CLR_ZERO);
  
  GG::Clr color;

  color = ClientUI::CTRL_BORDER_COLOR;
  if(planet->PrimaryFocus()==Planet::BALANCED || planet->PrimaryFocus()==Planet::FARMING)
    color = (planet->SecondaryFocus()==Planet::BALANCED || planet->SecondaryFocus()==Planet::FARMING)?GG::CLR_WHITE:GG::Clr(255,255,0,255);
  m_button_food->SetBorderColor(color);

  color = ClientUI::CTRL_BORDER_COLOR;
  if(planet->PrimaryFocus()==Planet::BALANCED || planet->PrimaryFocus()==Planet::MINING)
    color = (planet->SecondaryFocus()==Planet::BALANCED || planet->SecondaryFocus()==Planet::MINING) ?GG::CLR_WHITE:GG::Clr(255,0,0,255);
  m_button_mining->SetBorderColor(color);

  color = ClientUI::CTRL_BORDER_COLOR;
  if(planet->PrimaryFocus()==Planet::BALANCED || planet->PrimaryFocus()==Planet::INDUSTRY)
    color = (planet->SecondaryFocus()==Planet::BALANCED || planet->SecondaryFocus()==Planet::INDUSTRY)?GG::CLR_WHITE:GG::Clr(0,0,255,255);
  m_button_industry->SetBorderColor(color);

  color = ClientUI::CTRL_BORDER_COLOR;
  if(planet->PrimaryFocus()==Planet::BALANCED || planet->PrimaryFocus()==Planet::SCIENCE)
    color = (planet->SecondaryFocus()==Planet::BALANCED || planet->SecondaryFocus()==Planet::SCIENCE) ?GG::CLR_WHITE:GG::Clr(0,255,0,255);
  m_button_research->SetBorderColor(color);

}

void SidePanel::PlanetPanel::SetPrimaryFocus(Planet::FocusType focus)
{
  Planet *planet = GetPlanet();
  planet->SetPrimaryFocus(focus);
}

void SidePanel::PlanetPanel::SetSecondaryFocus(Planet::FocusType focus)
{
  Planet *planet = GetPlanet();
  planet->SetSecondaryFocus(focus);
} 

void SidePanel::PlanetPanel::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
  GG::Wnd *parent;
  if(parent=Parent())
    parent->MouseWheel(pt,move,keys);
}
void SidePanel::PlanetPanel::MouseEnter(const GG::Pt& pt, Uint32 keys)
{
}

void SidePanel::PlanetPanel::MouseLeave(const GG::Pt& pt, Uint32 keys)
{
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
	m_planet_image_lclick_sig(m_planet_id);
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
  const Planet *planet = GetPlanet();

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
  const Planet *planet = GetPlanet();

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
  const Planet *planet = GetPlanet();

  HumanClientApp::Orders().IssueOrder(new PlanetBuildOrder(*planet->Owners().begin(), planet->ID(),static_cast<ConstructionRow&>(m_construction->GetRow(idx)).m_build_type));
}

void SidePanel::PlanetPanel::ClickColonize()
{
  const Planet *planet = GetPlanet();
  Ship *ship=FindColonyShip(planet->SystemID());
  if(ship==0)
    throw std::runtime_error("SidePanel::PlanetPanel::ClickColonize ship not found!");

  HumanClientApp::Orders().IssueOrder(new FleetColonizeOrder( HumanClientApp::GetApp()->PlayerID(), ship->GetFleet()->ID(), planet->ID() ));
}

void SidePanel::PlanetPanel::RClick(const GG::Pt& pt, Uint32 keys)
{
  const Planet *planet = GetPlanet();
  
  if(!planet->OwnedBy(HumanClientApp::GetApp()->PlayerID()))
    return;


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
// SidePanel::PlanetView
////////////////////////////////////////////////
SidePanel::PlanetView::PlanetView(int x, int y, int w, int h,const Planet &plt)
: Wnd(x, y, w, h, GG::Wnd::CLICKABLE | GG::Wnd::ONTOP),
  m_planet_graphic(0),
  m_bg_image(),
  m_build_image(),m_planet_id(plt.ID()),
  m_radio_btn_primary_focus  (0),
  m_radio_btn_secondary_focus(0),
  m_bShowUI(false),m_fadein_start(0),m_fadein_span(0)
{
  Planet *planet = dynamic_cast<Planet*>(GetUniverse().Object(m_planet_id));

  EnableChildClipping(true);
  std::vector<boost::shared_ptr<GG::Texture> > textures; int start_frame; double fps;

  GetPlanetTextures(*planet,textures,start_frame=-1,fps=0.0);
  m_planet_graphic = new GG::DynamicGraphic(-35,-20,80,80,true,textures[0]->DefaultWidth(),textures[0]->DefaultHeight(),0,textures, GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
  if(start_frame==-1 && 1<textures.size())
    start_frame = RandSmallInt(0,textures.size()-1);

  if(start_frame!=-1 && fps!=0.0)
    m_planet_graphic->SetTimeIndex(start_frame * 1000.0 / m_planet_graphic->FPS());

  AttachChild(m_planet_graphic);m_planet_graphic->Play();

  boost::shared_ptr<GG::Texture> texture;
  switch(planet->Type())
  {
    case Planet::PT_SWAMP     : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/swamp1.png");break;
    case Planet::PT_TOXIC     : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/toxic1.png");break;
    case Planet::PT_INFERNO   : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/inferno1.png");break;
    case Planet::PT_RADIATED  : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/irradiated1.png");break;
    case Planet::PT_BARREN    : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/barren1.png");break;
    case Planet::PT_TUNDRA    : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/tundra1.png");break;
    case Planet::PT_DESERT    : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/desert1.png");break;
    case Planet::PT_TERRAN    : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/terran1.png");break;
    case Planet::PT_OCEAN     : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/ocean1.png");break;
    case Planet::PT_GAIA      : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/terran1.png");break;
    case Planet::PT_ASTEROIDS : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/terran1.png");break;
    case Planet::PT_GASGIANT  : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/terran1.png");break;       
    default                   : texture=GetTexture(ClientUI::ART_DIR + "planets_bg/terran1.png");break;
  }
  m_bg_image = GG::SubTexture(texture,0,0,texture->DefaultWidth(),texture->DefaultHeight());

  texture=GetTexture(ClientUI::ART_DIR + "misc/planetpanelpictures.png");
  m_foci_image = GG::SubTexture(texture,0,0,texture->DefaultWidth(),texture->DefaultHeight());

  GG::Pt ul = UpperLeft();

  m_radio_btn_primary_focus = new GG::RadioButtonGroup(10,150);
  m_radio_btn_primary_focus->AddButton(new CUIStateButton(0,  0,10,10,"",0,CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON,GG::CLR_WHITE));
  m_radio_btn_primary_focus->AddButton(new CUIStateButton(0, 35,10,10,"",0,CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON,GG::CLR_WHITE));
  m_radio_btn_primary_focus->AddButton(new CUIStateButton(0, 70,10,10,"",0,CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON,GG::CLR_WHITE));
  m_radio_btn_primary_focus->AddButton(new CUIStateButton(0,105,10,10,"",0,CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON,GG::CLR_WHITE));
  m_radio_btn_primary_focus->AddButton(new CUIStateButton(0,130,10,10,"",0,CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON,GG::CLR_WHITE));
  AttachChild(m_radio_btn_primary_focus);
  m_connection_btn_primary_focus_changed = GG::Connect(m_radio_btn_primary_focus->ButtonChangedSignal(), &SidePanel::PlanetView::PrimaryFocusClicked, this);

  m_radio_btn_secondary_focus = new GG::RadioButtonGroup(30,150);
  m_radio_btn_secondary_focus->AddButton(new CUIStateButton(0,  0,10,10,"",0,CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON,GG::CLR_WHITE));
  m_radio_btn_secondary_focus->AddButton(new CUIStateButton(0, 35,10,10,"",0,CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON,GG::CLR_WHITE));
  m_radio_btn_secondary_focus->AddButton(new CUIStateButton(0, 70,10,10,"",0,CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON,GG::CLR_WHITE));
  m_radio_btn_secondary_focus->AddButton(new CUIStateButton(0,105,10,10,"",0,CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON,GG::CLR_WHITE));
  m_radio_btn_secondary_focus->AddButton(new CUIStateButton(0,130,10,10,"",0,CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON,GG::CLR_WHITE));
  AttachChild(m_radio_btn_secondary_focus);
  m_connection_btn_secondary_focus_changed = GG::Connect(m_radio_btn_secondary_focus->ButtonChangedSignal(), &SidePanel::PlanetView::SecondaryFocusClicked, this);

  m_construction = new CUIDropDownList(260,20,150,ClientUI::SIDE_PANEL_PTS,(ClientUI::SIDE_PANEL_PTS) * 15, GG::CLR_ZERO,ClientUI::DROP_DOWN_LIST_INT_COLOR,ClientUI::DROP_DOWN_LIST_INT_COLOR);

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


  Empire *empire = HumanClientApp::Empires().Lookup( HumanClientApp::GetApp()->PlayerID() );
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
    if(static_cast<ConstructionRow&>(m_construction->GetRow(i)).m_build_type ==  planet->CurrentlyBuilding() )
      selection_idx =i;
  
  if(selection_idx!=-1)
    m_construction->Select( selection_idx );
  Connect(m_construction->SelChangedSignal(), &SidePanel::PlanetView::BuildSelected, this);
  ////////////////////// v0.1 only!!

  GG::Connect(planet->StateChangedSignal(), &SidePanel::PlanetView::PlanetChanged, this);
  m_connection_planet_production_changed=GG::Connect(planet->ProdCenterChangedSignal(), &SidePanel::PlanetView::PlanetProdCenterChanged, this);
  PlanetChanged();PlanetProdCenterChanged();
 }

void SidePanel::PlanetView::PlanetChanged()
{
  Planet *planet = dynamic_cast<Planet*>(GetUniverse().Object(m_planet_id));
  if(planet==0)
    throw std::runtime_error("SidePanel::PlanetView::PlanetChanged: planet not found");

  bool is_owner = planet->OwnedBy(HumanClientApp::GetApp()->PlayerID());

  is_owner?m_construction->Show():m_construction->Hide();
  is_owner?m_radio_btn_primary_focus->Show():m_radio_btn_primary_focus->Hide();
  is_owner?m_radio_btn_secondary_focus->Show():m_radio_btn_secondary_focus->Hide();
  is_owner?m_construction->Show():m_construction->Hide();
  is_owner?m_construction->Show():m_construction->Hide();
  is_owner?m_construction->Show():m_construction->Hide();
  is_owner?m_construction->Show():m_construction->Hide();
}

void SidePanel::PlanetView::PlanetProdCenterChanged()
{
  Planet *planet = dynamic_cast<Planet*>(GetUniverse().Object(m_planet_id));
  if(planet==0)
    throw std::runtime_error("SidePanel::PlanetView::PlanetChanged: planet not found");

  boost::shared_ptr<GG::Texture> texture;
  switch(planet->CurrentlyBuilding())
  {
    case ProdCenter::COLONY_SHIP: texture=GetTexture(ClientUI::ART_DIR + "misc/colony1.png");break;
    case ProdCenter::SCOUT      : texture=GetTexture(ClientUI::ART_DIR + "misc/scout1.png");break;
    case ProdCenter::MARKI      : texture=GetTexture(ClientUI::ART_DIR + "misc/mark1.png");break;
    case ProdCenter::MARKII     : texture=GetTexture(ClientUI::ART_DIR + "misc/mark2.png");break;
    case ProdCenter::MARKIII    : texture=GetTexture(ClientUI::ART_DIR + "misc/mark3.png");break;
    case ProdCenter::MARKIV     : texture=GetTexture(ClientUI::ART_DIR + "misc/mark4.png");break;
    case ProdCenter::DEF_BASE   : texture=GetTexture(ClientUI::ART_DIR + "misc/base1.png");break;
    default                     : texture=GetTexture(ClientUI::ART_DIR + "misc/missing.png");break;
  }

  m_build_image = GG::SubTexture(texture,0,0,texture->DefaultWidth(),texture->DefaultHeight());

  int i;
  for(i=0; i< m_construction->NumRows();i++)
    if(static_cast<ConstructionRow&>(m_construction->GetRow(i)).m_build_type ==  planet->CurrentlyBuilding() )
    {
      m_construction->Select(i);
      break;
    }

  switch(planet->PrimaryFocus())
  {
    case ProdCenter::FARMING  : m_radio_btn_primary_focus->SetCheck(0);break;
    case ProdCenter::MINING   : m_radio_btn_primary_focus->SetCheck(1);break;
    case ProdCenter::SCIENCE  : m_radio_btn_primary_focus->SetCheck(2);break;
    case ProdCenter::INDUSTRY : m_radio_btn_primary_focus->SetCheck(3);break;
    case ProdCenter::BALANCED : m_radio_btn_primary_focus->SetCheck(4);break;
  }
  switch(planet->SecondaryFocus())
  {
    case ProdCenter::FARMING  : m_radio_btn_secondary_focus->SetCheck(0);break;
    case ProdCenter::MINING   : m_radio_btn_secondary_focus->SetCheck(1);break;
    case ProdCenter::SCIENCE  : m_radio_btn_secondary_focus->SetCheck(2);break;
    case ProdCenter::INDUSTRY : m_radio_btn_secondary_focus->SetCheck(3);break;
    case ProdCenter::BALANCED : m_radio_btn_secondary_focus->SetCheck(4);break;
  }
}

void SidePanel::PlanetView::Show(bool children)
{
  GG::Wnd::Show(children);

  const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));
  if(!planet)
    throw std::runtime_error("SidePanel::PlanetView::Show: planet not found!");
  enum OWNERSHIP {OS_NONE,OS_FOREIGN,OS_SELF} owner = OS_NONE;

  std::string text;
  if(planet->Owners().size()==0) 
    owner = OS_NONE;
  else 
    if(!planet->OwnedBy(HumanClientApp::GetApp()->PlayerID()))
      owner = OS_FOREIGN;
    else
      owner = OS_SELF;

  // visibility
  (m_bShowUI && owner==OS_SELF)?m_construction             ->Show():m_construction             ->Hide();
  (m_bShowUI && owner==OS_SELF)?m_radio_btn_primary_focus  ->Show():m_radio_btn_primary_focus  ->Hide();
  (m_bShowUI && owner==OS_SELF)?m_radio_btn_secondary_focus->Show():m_radio_btn_secondary_focus->Hide();
}

void SidePanel::PlanetView::FadeIn()
{
  double fade_in = static_cast<double>((GG::App::GetApp()->Ticks()-m_fadein_start))/static_cast<double>(m_fadein_span);

  if(fade_in>=1.0)
  {
    m_transparency=255;

    if(!m_bShowUI)
    {
      m_bShowUI = true;
      if(Visible())
        Show();
    }
  }
  else
    m_transparency = static_cast<int>(fade_in*255);
}

void SidePanel::PlanetView::SetFadeInPlanetView  (int start, int span)
{
  m_bShowUI=false;
  m_fadein_start=start;m_fadein_span=span;

  m_transparency = 0;

  if(Visible())
    Show();
}

void SidePanel::PlanetView::SetFadeInPlanetViewUI(int start, int span)
{
  m_bShowUI=true;
  m_fadein_start=start;m_fadein_span=span;

  m_transparency = 255;

  if(Visible())
    Show();
}

void SidePanel::PlanetView::BuildSelected(int idx) const
{
  const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));
  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::BuildSelected planet not found!");

  HumanClientApp::Orders().IssueOrder(new PlanetBuildOrder(*planet->Owners().begin(), planet->ID(),static_cast<ConstructionRow&>(m_construction->GetRow(idx)).m_build_type));
}

void SidePanel::PlanetView::PrimaryFocusClicked(int idx)
{
  m_connection_planet_production_changed.disconnect();
  m_connection_btn_primary_focus_changed.disconnect();

  Planet::FocusType ft=Planet::FOCUS_UNKNOWN;
  switch(idx)
  {
    case  0:ft=Planet::FARMING      ;break;
    case  1:ft=Planet::MINING       ;break;
    case  2:ft=Planet::SCIENCE      ;break;
    case  3:ft=Planet::INDUSTRY     ;break;
    case  4:ft=Planet::BALANCED     ;break;
    default:ft=Planet::FOCUS_UNKNOWN;break;
  }
  Planet *planet = dynamic_cast<Planet*>(GetUniverse().Object(m_planet_id));
  if(planet->PrimaryFocus()!=ft)
    planet->SetPrimaryFocus(ft);
  
  m_connection_btn_primary_focus_changed = GG::Connect(m_radio_btn_primary_focus->ButtonChangedSignal(), &SidePanel::PlanetView::PrimaryFocusClicked, this);
  m_connection_planet_production_changed=GG::Connect(planet->ProdCenterChangedSignal(), &SidePanel::PlanetView::PlanetProdCenterChanged, this);
}

void SidePanel::PlanetView::SecondaryFocusClicked(int idx)
{
  m_connection_planet_production_changed.disconnect();
  m_connection_btn_secondary_focus_changed.disconnect();

  Planet::FocusType ft=Planet::FOCUS_UNKNOWN;
  switch(idx)
  {
    case  0:ft=Planet::FARMING      ;break;
    case  1:ft=Planet::MINING       ;break;
    case  2:ft=Planet::SCIENCE      ;break;
    case  3:ft=Planet::INDUSTRY     ;break;
    case  4:ft=Planet::BALANCED     ;break;
    default:ft=Planet::FOCUS_UNKNOWN;break;
  }
  Planet *planet = dynamic_cast<Planet*>(GetUniverse().Object(m_planet_id));
  if(planet->SecondaryFocus()!=ft)
    planet->SetSecondaryFocus(ft);

  m_connection_btn_secondary_focus_changed = GG::Connect(m_radio_btn_secondary_focus->ButtonChangedSignal(), &SidePanel::PlanetView::SecondaryFocusClicked, this);
  m_connection_planet_production_changed=GG::Connect(planet->ProdCenterChangedSignal(), &SidePanel::PlanetView::PlanetProdCenterChanged, this);
}

bool SidePanel::PlanetView::Render()
{
  FadeIn();

  const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_planet_id));
  if(!planet)
    throw std::runtime_error("SidePanel::PlanetPanel::BuildSelected planet not found!");

  GG::Pt ul = UpperLeft(), lr = LowerRight();
  GG::Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

  GG::BeginScissorClipping(ClientUpperLeft(), ClientLowerRight());
  int height = (Width() * m_bg_image.Height()) / m_bg_image.Width();
  
  GG::Clr border_color=GG::CLR_WHITE;

  if(!m_bShowUI)
  {
    glColor4ubv(GG::Clr(m_transparency,m_transparency,m_transparency,255).v);
    border_color = GG::Clr(m_transparency,m_transparency,m_transparency,255);
  }

  m_bg_image.OrthoBlit(ul.x,ul.y+(Height()-height)/2,lr.x,ul.y+(Height()-height)/2+height,false);
  GG::EndScissorClipping();
  
  AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, border_color, 1,0,0,0,0);

  if(!m_bShowUI)
    return true;

  if(!planet->OwnedBy(HumanClientApp::GetApp()->PlayerID()))    
    return true;

  GG::Clr alpha_color(GG::CLR_WHITE);
  if(m_bShowUI)
  {
    alpha_color=GG::Clr(m_transparency,m_transparency,m_transparency,255).v;
  }

  AngledCornerRectangle(ul.x+5, ul.y+40,ul.x+245, ul.y+81, GG::Clr(0,0,0,100), GG::CLR_ZERO,0,0,0,0,0);
  AngledCornerRectangle(ul.x+5, ul.y+80,ul.x+245, lr.y- 5, GG::Clr(0,0,0,200), GG::CLR_ZERO,0,0,0,0,0);
  glColor4ubv(alpha_color.v);

  m_foci_image.OrthoBlit(ul+GG::Pt(60,140),ul+GG::Pt(60+m_foci_image.Width(),150+130),false);

  boost::shared_ptr<GG::Font> font;std::string text; int y;
  Uint32 format = GG::TF_LEFT | GG::TF_VCENTER;

  glColor4ubv(alpha_color.v);

  y = ul.y+40;
  text = planet->Name();
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.3));
  font->RenderText(ul.x+30,y,ul.x+500,y+font->Height(), text, format, 0, false);
  y+=font->Height();

  text = GetPlanetSizeName(*planet);if(text.length()>0) text+=" "; text+= GetPlanetTypeName(*planet);
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
  font->RenderText(ul.x+30,y,ul.x+500,y+font->Height(), text, format, 0, false);
  y+=font->Height();

  y = ul.y+80;
  text = "Population";
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
  font->RenderText(ul.x+10,y,ul.x+500,y+font->Height(), text, format, 0, false);

  if(planet->MaxPop()==0) text= ClientUI::String("PL_UNHABITABLE");
  else                    text= " "+boost::lexical_cast<std::string>(planet->PopPoints())+"/"+boost::lexical_cast<std::string>(planet->MaxPop()) + " Million";

  text+="  ";
  text+= planet->PopGrowth()<0?(GG::RgbaTag(GG::CLR_RED)+"(-"):(GG::RgbaTag(GG::CLR_GREEN)+"(+");
  text+=boost::lexical_cast<std::string>(planet->PopGrowth());
  text+=")</rgba>";

  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
  font->RenderText(ul.x+10+80,y,ul.x+500,y+font->Height(), text, format, 0, true);
  y+=font->Height();


  text = "Immigration";
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*0.9));
  font->RenderText(ul.x+10,y,ul.x+500,y+font->Height(), text, format, 0, false);
  y+=font->Height();

  y+=15;

  text = "Focus";
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
  font->RenderText(ul.x+10,y,ul.x+500,y+font->Height(), text, format, 0, false);
  
  text = "Production";
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
  font->RenderText(ul.x+10+80,y,ul.x+500,y+font->Height(), text, format, 0, false);
  y+=font->Height();

  text = "pri";
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*0.9));
  font->RenderText(ul.x+10,y,ul.x+500,y+font->Height(), text, format, 0, false);

  text = "sec";
  font->RenderText(ul.x+30,y,ul.x+500,y+font->Height(), text, format, 0, false);
  
  y+=font->Height();

  text = "Shared Focus";
  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*0.9));
  font->RenderText(ul.x+60,ul.y+150+130,ul.x+500,ul.y+150+130+font->Height(), text, format, 0, false);
  y+=font->Height();


  font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));

  int farming=0,mining=0,research=0,industry=0,defense=0,population=0;

  //farming   +=;
  //mining    +=;
  //research  +=;
  industry  +=static_cast<int>(planet->ProdPoints());;
  //defense   +=;
  population+=static_cast<int>(planet->PopPoints());

  int x,icon_dim = static_cast<int>(font->Height()*1.5);  boost::shared_ptr<GG::Texture> icon;

  x = UpperLeft().x+150;
  //farming
  y = UpperLeft().y+150;
  icon=IconFarming(); icon->OrthoBlit(x,y,x+icon_dim,y+icon_dim, 0, false);
  text = (farming<0?"-":"+") + boost::lexical_cast<std::string>(farming);
  font->RenderText(x+icon_dim,y,x + 500, y+icon_dim, text, format, 0, true);

  //mining
  y = UpperLeft().y+150+35;
  icon=IconMining(); icon->OrthoBlit(x,y,x+icon_dim,y+icon_dim, 0, false);
  text = (mining<0?"-":"+") + boost::lexical_cast<std::string>(mining);
  font->RenderText(x+icon_dim,y,x + 500, y+icon_dim, text, format, 0, true);

  //research
  y = UpperLeft().y+150+70;
  icon=IconResearch(); icon->OrthoBlit(x,y,x+icon_dim,y+icon_dim, 0, false);
  text = (research<0?"-":"+") + boost::lexical_cast<std::string>(research);
  font->RenderText(x+icon_dim,y,x + 500, y+icon_dim, text, format, 0, true);

  //industy
  y = UpperLeft().y+150+105;
  icon=IconIndustry(); icon->OrthoBlit(x,y,x+icon_dim,y+icon_dim, 0, false);
  text = (industry<0?"-":"+") + boost::lexical_cast<std::string>(industry);
  font->RenderText(x+icon_dim,y,x + 500, y+icon_dim, text, format, 0, true);

  GG::Rect rc_right_rect(255,5,495,295);
  FlatRectangle(UpperLeft().x+rc_right_rect.UpperLeft().x,UpperLeft().y+rc_right_rect.UpperLeft().y,
                UpperLeft().x+rc_right_rect.LowerRight().x,UpperLeft().y+rc_right_rect.LowerRight().y,GG::Clr(0,0,0,200),GG::CLR_ZERO, 1);
  glColor4ubv(alpha_color.v);

  GG::Rect rc_build_image_border(260,45,490,175);
  rc_build_image_border+=UpperLeft();

  height = (rc_build_image_border.Width() * m_build_image.Height()) / m_build_image.Width();
  
  GG::BeginScissorClipping(rc_build_image_border.UpperLeft(),rc_build_image_border.LowerRight());
  m_build_image.OrthoBlit(rc_build_image_border.Left (),rc_build_image_border.Top()+(rc_build_image_border.Height()-height)/2,
                          rc_build_image_border.Right(),rc_build_image_border.Top()+(rc_build_image_border.Height()-height)/2+height,false);
  GG::EndScissorClipping();
  
  AngledCornerRectangle(rc_build_image_border.Left (),rc_build_image_border.Top   (),
                        rc_build_image_border.Right(),rc_build_image_border.Bottom(), GG::CLR_ZERO,GG::CLR_WHITE, 1,0,0,0,0);
  glColor4ubv(alpha_color.v);
  
  if(ProdCenter::SCOUT <= planet->CurrentlyBuilding())
  {
    // construction progress bar
    // TODO : get the costs of the item from the list of available technologies
    const int PROD_COSTS[] = {0, 0, 0, 50, 250, 100, 200, 375, 700, 200};
    int cost = PROD_COSTS[planet->CurrentlyBuilding()];
    double percent_complete = cost ? (planet->BuildProgress()+planet->Rollover()) / cost : 0.0;

    int x1 = m_construction->UpperLeft ().x;
    int x2 = m_construction->LowerRight().x;
    int y1,y2;
    y1 = m_construction->LowerRight().y-2;
    y2 = y1 + 5;
    GG::FlatRectangle(x1, y1, x2, y2, GG::CLR_ZERO, ClientUI::CTRL_BORDER_COLOR, 1);
    GG::FlatRectangle(x1, y1, x1 + static_cast<int>((x2 - x1 - 2) * percent_complete), y2,
                      ClientUI::SIDE_PANEL_BUILD_PROGRESSBAR_COLOR, LightColor(ClientUI::SIDE_PANEL_BUILD_PROGRESSBAR_COLOR), 1);

    // construction progress text
    font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
    Uint32 format = GG::TF_LEFT | GG::TF_VCENTER;
    text = "";
    if(cost && !planet->ProdPoints()) text = ClientUI::String("PL_PRODUCTION_TIME_NEVER");
    else
      if(cost)
        text = Format(ClientUI::String("PL_PRODUCTION_TIME_TURNS").c_str(),static_cast<int>(std::ceil((cost - planet->BuildProgress()) / planet->ProdPoints())));

    x1 = m_construction->LowerRight().x;
    y1 = m_construction->UpperLeft ().y;
    y2 = m_construction->LowerRight().y;
    glColor4ubv(ClientUI::TEXT_COLOR.v);
    font->RenderText(x1, y1, x1+500, y2, text, format, 0, false);

    ShipDesign::V01DesignID design_id=static_cast<ShipDesign::V01DesignID>(0);

    switch(planet->CurrentlyBuilding())
    {
      case ProdCenter::BUILD_UNKNOWN  : break;
      case ProdCenter::NOT_BUILDING   : break;
      case ProdCenter::INDUSTRY_BUILD : break;
      case ProdCenter::RESEARCH_BUILD : break;
      case ProdCenter::SCOUT          : design_id = ShipDesign::SCOUT;break;
      case ProdCenter::COLONY_SHIP    : design_id = ShipDesign::COLONY;break;
      case ProdCenter::MARKI          : design_id = ShipDesign::MARK1;break;
      case ProdCenter::MARKII         : design_id = ShipDesign::MARK2;break;
      case ProdCenter::MARKIII        : design_id = ShipDesign::MARK3;break;
      case ProdCenter::MARKIV         : design_id = ShipDesign::MARK4;break;
      case ProdCenter::DEF_BASE       : break;
    }

    Empire *empire=HumanClientApp::Empires().Lookup(*(planet->Owners().begin()));
    Empire::ShipDesignItr itr;
   
    for(itr=empire->ShipDesignBegin();itr != empire->ShipDesignEnd();++itr)
      if((*itr).first==design_id)
        break;
    if(itr != empire->ShipDesignEnd())
    {
      ShipDesign design = (*itr).second;

      text = design.name;
      font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.3));
      font->RenderText(ul.x+260,ul.y+180,ul.x+500,ul.y+180+font->Height(), text, format, 0, false);

      y = ul.y+180;
      font = HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.0));
     
      text = "Attack rating";
      font->RenderText(ul.x+375,y,ul.x+500,y+font->Height(), text, format, 0, false);

      text = boost::lexical_cast<std::string>(design.attack);
      font->RenderText(lr.x-30,y,ul.x+500,y+font->Height(), text, format, 0, false);

      y+=font->Height();

      text = "Defence rating";
      font->RenderText(ul.x+375,y,ul.x+500,y+font->Height(), text, format, 0, false);

      text = boost::lexical_cast<std::string>(design.defense);
      font->RenderText(lr.x-30,y,ul.x+500,y+font->Height(), text, format, 0, false);
      
      y+=font->Height();

      text = "Cost";
      font->RenderText(ul.x+375,y,ul.x+500,y+font->Height(), text, format, 0, false);

      text = boost::lexical_cast<std::string>(design.cost);
      font->RenderText(lr.x-30,y,ul.x+500,y+font->Height(), text, format, 0, false);
    }
  }

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
    m_planet_panel_container(new PlanetPanelContainer(0,100,w,h-100-30)),
    m_next_pltview_fade_in(0),m_next_pltview_planet_id(UniverseObject::INVALID_OBJECT_ID),m_next_pltview_fade_out(-1),
    m_planet_view(0)
{
  SetText(ClientUI::String("SIDE_PANEL"));

  m_system_name->DisableDropArrow();
  m_system_name->SetStyle(GG::LB_CENTER);

  m_button_prev->SetUnpressedGraphic(GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/leftarrownormal.png"   ), 0, 0, 32, 32));
  m_button_prev->SetPressedGraphic  (GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/leftarrowclicked.png"  ), 0, 0, 32, 32));
  m_button_prev->SetRolloverGraphic (GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/leftarrowmouseover.png"), 0, 0, 32, 32));

  m_button_next->SetUnpressedGraphic(GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/rightarrownormal.png"  ), 0, 0, 32, 32));
  m_button_next->SetPressedGraphic  (GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/rightarrowclicked.png"   ), 0, 0, 32, 32));
  m_button_next->SetRolloverGraphic (GG::SubTexture(GetTexture( ClientUI::ART_DIR + "icons/rightarrowmouseover.png"), 0, 0, 32, 32));

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
  PlanetViewFadeIn();

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
    delete m_planet_view;m_planet_view=0;
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

      graphic = GetTexture(star_image);
      
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
      {
        GG::Connect(plt_vec[i]->StateChangedSignal(), &SidePanel::PlanetsChanged, this);
      }

      m_planet_panel_container->SetPlanets(plt_vec);
      for(int i = 0; i < m_planet_panel_container->PlanetPanels(); i++) 
        GG::Connect(m_planet_panel_container->GetPlanetPanel(i)->PlanetImageLClickedSignal(),&SidePanel::PlanetLClicked,this);

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
    m_planet_panel_container->GetPlanetPanel(i)->Update();
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

void SidePanel::PlanetLClicked(int planet_id)
{
  if(   planet_id != UniverseObject::INVALID_OBJECT_ID
     && (!m_planet_view || m_planet_view->PlanetID()!=planet_id))
  {
    if(m_planet_view)
    {
      GG::App::GetApp()->Remove(m_planet_view);
      delete m_planet_view;m_planet_view=0;
    }

    const Planet* planet = dynamic_cast<const Planet*>(GetUniverse().Object(planet_id));

    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();

    int app_width = GetOptionsDB().Get<int>("app-width") - MapWnd::SIDE_PANEL_WIDTH, 
        app_height= GetOptionsDB().Get<int>("app-height");

    int pltview_width = 500, 
        pltview_height= 300;

    m_planet_view = new PlanetView((app_width-pltview_width)/2,(app_height-pltview_height)/2,pltview_width,pltview_height,*planet);
    m_planet_view->SetFadeInPlanetView(GG::App::GetApp()->Ticks(),/*40*/0);
    GG::App::GetApp()->Register(m_planet_view);

    m_next_pltview_planet_id=-1;
    m_next_pltview_fade_in=-1;
  }
}

void SidePanel::PlanetViewFadeIn()
{
  GG::Pt mouse_pos = GG::App::GetApp()->MousePosition();
  int plt_idx=-1; int planet_id=UniverseObject::INVALID_OBJECT_ID;
  
  // check if mouse is on top of a planet panel
  for(plt_idx=0;plt_idx<m_planet_panel_container->PlanetPanels();plt_idx++)
    if(m_planet_panel_container->GetPlanetPanel(plt_idx)->InWindow(mouse_pos))
    {
      planet_id = m_planet_panel_container->GetPlanetPanel(plt_idx)->PlanetID();
      break;
    }

  if(m_planet_view && m_planet_view->InWindow(mouse_pos))
    planet_id = m_planet_view->PlanetID();

  // set fadeout time index or cancel fade out
  if(!m_planet_view)
  {
    m_next_pltview_fade_out=-1;
  }
  else
    if(planet_id == m_planet_view->PlanetID())
    {
      m_next_pltview_fade_in =-1;
      m_next_pltview_fade_out=-1;
    }
    else
    {
      if(m_next_pltview_fade_out==-1)
        m_next_pltview_fade_out = GG::App::GetApp()->Ticks()+200;
    }

  if(m_next_pltview_fade_out!=-1 && m_next_pltview_fade_out<GG::App::GetApp()->Ticks())
  {
    GG::App::GetApp()->Remove(m_planet_view);
    delete m_planet_view;m_planet_view=0;
  }

// cancel fade in for now
return;

  if(   planet_id != UniverseObject::INVALID_OBJECT_ID
     && planet_id != m_next_pltview_planet_id
     && !(m_planet_view && m_planet_view->PlanetID()==planet_id))
  {
    m_next_pltview_planet_id = planet_id;
    m_next_pltview_fade_in = GG::App::GetApp()->Ticks()+1000;
  }

  if(   m_next_pltview_planet_id != UniverseObject::INVALID_OBJECT_ID 
     && m_next_pltview_fade_in!=-1 && m_next_pltview_fade_in < GG::App::GetApp()->Ticks())
  {
    if(m_planet_view)
    {
      GG::App::GetApp()->Remove(m_planet_view);
      delete m_planet_view;m_planet_view=0;
    }

    const Planet* planet = dynamic_cast<const Planet*>(GetUniverse().Object(m_next_pltview_planet_id));

    MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();

    int app_width = GetOptionsDB().Get<int>("app-width") - MapWnd::SIDE_PANEL_WIDTH, 
        app_height= GetOptionsDB().Get<int>("app-height");

    int pltview_width = 500, 
        pltview_height= 300;

    m_planet_view = new PlanetView((app_width-pltview_width)/2,(app_height-pltview_height)/2,pltview_width,pltview_height,*planet);
    m_planet_view->SetFadeInPlanetView(GG::App::GetApp()->Ticks(),400);
    GG::App::GetApp()->Register(m_planet_view);

    m_next_pltview_planet_id=-1;
    m_next_pltview_fade_in=-1;
  }
}