// -*- C++ -*-
#ifndef _SidePanel_h_
#define _SidePanel_h_

#ifndef _CUIControls_h_
#include "CUIControls.h"
#endif

#ifndef _GGDynamicGraphic_h_
#include "GGDynamicGraphic.h"
#endif

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGSignalsAndSlots_h_
#include "GGSignalsAndSlots.h"
#endif

#ifndef _GGTexture_h_
#include "GGTexture.h"
#endif

#ifndef _ProdCenter_h_
#include "../universe/ProdCenter.h"
#endif

#ifndef _Planet_h_
#include "../universe/Planet.h"
#endif

#ifndef _System_h_
#include "../universe/System.h"
#endif

#include <vector>

class CUI_CloseButton;
class CUIDropDownList;
class CUIIconButton;
class CUIScroll;
class CUITextureButton;
namespace GG {class TextControl;}
class RotatingPlanetControl;

class SidePanel : public GG::Wnd
{
  class PlanetView;
  public:
    /** a single planet's info and controls; several of these may appear at any one time in a SidePanel */
    class PlanetPanel : public GG::Wnd
    {
    public:
        /** \name Signal Types */ //@{
        typedef boost::signal<void (int)> LeftClickedSignalType; ///< emitted when the planet graphic is left clicked by the user
        //@}
   
        /** \name Slot Types */ //@{
        typedef LeftClickedSignalType::slot_type LeftClickedSlotType; ///< type of functor(s) invoked on a LeftClickedSignalType
        //@}

        /** \name Structors */ //@{
        PlanetPanel(int x, int y, int w, int h, const Planet &planet, StarType star_type); ///< basic ctor
        ~PlanetPanel();
        //@}

        /** \name Accessors */ //@{
        virtual bool InWindow(const GG::Pt& pt) const;
        virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
        virtual void MouseEnter(const GG::Pt& pt, Uint32 keys);            ///< respond to cursor entering window's coords
        virtual void MouseLeave(const GG::Pt& pt, Uint32 keys);            ///< respond to cursor leaving window's coords

        int PlanetID() const {return m_planet_id;}

        LeftClickedSignalType& PlanetImageLClickedSignal() const {return m_planet_image_lclick_sig;} ///< returns the left clicked signal object for this Planet panel
        //@}

        /** \name Mutators */ //@{
        virtual bool Render();
        virtual void LClick(const GG::Pt& pt, Uint32 keys);
        virtual void RClick(const GG::Pt& pt, Uint32 keys);
        void Update();
        //@}

    private:
        /** some of the elements at planet panel are only used if a specific
            planet ownership state is present, some others are only used if
            additional conditions applies. If a control is being enabled, it's
            moved from the list of disabled controls (m_vec_unused_controls) to
            the child list of planet panel, if the control isn't found at m_vec_unused_controls
            is assumed that it is already enable. after that control->Show() is called. Disableing a
            control is done in reverse.
            for example: colonize btn is only enable/visible if there is a colony ship in orbit 
                         and the planet is unowned and inhabitable*/
        void EnableControl(GG::Wnd *control,bool enable);


        bool RenderUnhabited(const Planet &planet); ///< it's call if the planet isn't inhabited
        bool RenderInhabited(const Planet &planet); ///< it's call if the planet is inhabited by someone else
        bool RenderOwned    (const Planet &planet); ///< it's call if the planet is inhabited by te player

        int  PlanetDiameter() const;
        bool InPlanet(const GG::Pt& pt) const;///< returns true if pt is within the planet image

        void PlanetChanged();                 ///< called when a planet was changed to handle rendering and wich control are enabled
        void PlanetProdCenterChanged();       ///< called when a planet production was changed
        void BuildSelected(int idx) const;    ///< called when a planet production was changed

        void SetPrimaryFocus  (FocusType focus); ///< set the primary focus of the planet to focus
        void SetSecondaryFocus(FocusType focus); ///< set the secondary focus of the planet to focus

        void LClickFarming () {SetPrimaryFocus(FOCUS_FARMING );}///< set the primary focus of the planet to farming
        void LClickMining  () {SetPrimaryFocus(FOCUS_MINING  );}///< set the primary focus of the planet to mining
        void LClickIndustry() {SetPrimaryFocus(FOCUS_INDUSTRY);}///< set the primary focus of the planet to industry
        void LClickResearch() {SetPrimaryFocus(FOCUS_RESEARCH );}///< set the primary focus of the planet to science
        void LClickBalanced() {SetPrimaryFocus(FOCUS_BALANCED);}///< set the primary focus of the planet to balanced

        void RClickFarming () {SetSecondaryFocus(FOCUS_FARMING );}///< set the secondary focus of the planet to farming
        void RClickMining  () {SetSecondaryFocus(FOCUS_MINING  );}///< set the secondary focus of the planet to mining
        void RClickIndustry() {SetSecondaryFocus(FOCUS_INDUSTRY);}///< set the secondary focus of the planet to industry
        void RClickResearch() {SetSecondaryFocus(FOCUS_RESEARCH );}///< set the secondary focus of the planet to science
        void RClickBalanced() {SetSecondaryFocus(FOCUS_BALANCED);}///< set the secondary focus of the planet to balanced

        void ClickColonize();///< called if btn colonize is pressed

              Planet* GetPlanet(); ///< returns the planet with ID m_planet_id
        const Planet* GetPlanet() const;

        int                   m_planet_id;                ///< id for the planet with is representet by this planet panel
        GG::TextControl       *m_planet_name;             ///< planet name
        GG::TextControl       *m_planet_info;             ///< planet size and type info
        CUIButton             *m_button_colonize;         ///< btn which can be pressed to colonize this planet
        GG::DynamicGraphic    *m_planet_graphic;          ///< image of the planet (can be a frameset); this is now used only for asteroids
        RotatingPlanetControl *m_rotating_planet_graphic; ///< a realtime-rendered planet that rotates, with a textured surface mapped onto it
        CUIIconButton         *m_button_food,             ///< food focus btn (lclick - set primary focus,rclick - ser secondary focus)
                              *m_button_mining,           ///< mining focus btn (lclick - set primary focus,rclick - ser secondary focus)
                              *m_button_industry,         ///< industry focus btn (lclick - set primary focus,rclick - ser secondary focus)
                              *m_button_research,         ///< research focus btn (lclick - set primary focus,rclick - ser secondary focus)
                              *m_button_balanced;         ///< balanced focus btn (lclick - set primary focus,rclick - ser secondary focus)

        CUIDropDownList       *m_construction;            ///< drop down list which hold planet build projects

        boost::signals::connection m_connection_system_changed;           ///< stores connection used to handle a system change
        boost::signals::connection m_connection_planet_changed;           ///< stores connection used to handle a planet change
        boost::signals::connection m_connection_planet_production_changed;///< stores connection used to handle a planet production change

        /** planet panel is constructed without taking care of which controls
            are needed by current planet ownership state. All control which aren't
            needed by current planet ownership state are stored in m_vec_unused_controls
            and can be used when for instance planet ownership changes
        */
        std::vector<GG::Wnd*> m_vec_unused_controls;


        mutable LeftClickedSignalType m_planet_image_lclick_sig;///< fired if planet image get an left click
    };

    /** \name Structors */ //@{
    SidePanel(int x, int y, int w, int h);
    //@}

    /** \name Accessors */ //@{
    virtual bool InWindow(const GG::Pt& pt) const;

    int                PlanetPanels() const        {return m_planet_panel_container->PlanetPanels();}
    const PlanetPanel* GetPlanetPanel(int n) const {return m_planet_panel_container->GetPlanetPanel(n);}
    int                SystemID() const;
    //@}

    /** \name Mutators */ //@{
    virtual bool  Render();

    void         SetSystem(int system_id); ///< sets the system currently being viewed in the side panel
    //@}

private:
    void AdjustScrolls();

    void SystemSelectionChanged(int selection);
    void UniverseObjectDelete(const UniverseObject *);

    void FleetsChanged();
    void PlanetsChanged();
    void PrevButtonClicked();
    void NextButtonClicked();

    void PlanetLClicked(int planet_id);

    const System        *m_system;
    CUIDropDownList     *m_system_name;
    GG::TextControl     *m_system_name_unknown;
    GG::Button          *m_button_prev,*m_button_next;
    GG::DynamicGraphic  *m_star_graphic;
    GG::TextControl     *m_static_text_systemproduction;

    void PlanetViewFadeIn();

    int                 m_next_pltview_fade_in;
    int                 m_next_pltview_planet_id;
    int                 m_next_pltview_fade_out;
    PlanetView          *m_planet_view;

    std::vector<GG::SubTexture> m_fleet_icons;

    class PlanetPanelContainer : public GG::Wnd
    {
      public:
        /** \name Structors */ //@{
        PlanetPanelContainer(int x, int y, int w, int h);
        //@}

        void Clear();
        void SetPlanets(const std::vector<const Planet*> &plt_vec, StarType star_type);

        /** \name Accessors */ //@{
        virtual bool InWindow(const GG::Pt& pt) const;
        virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)

        int                PlanetPanels() const        {return m_planet_panels.size();}
        const PlanetPanel* GetPlanetPanel(int n) const {return m_planet_panels[n];}
        //@}

        PlanetPanel* GetPlanetPanel(int n) {return m_planet_panels[n];}

      private:
        std::vector<PlanetPanel*>   m_planet_panels;
    
        void VScroll(int,int,int,int);
        CUIScroll*        m_vscroll; ///< the vertical scroll (for viewing all the planet panes)
    };

    class SystemResourceSummary : public GG::Wnd
    {
      public:
        /** \name Structors */ //@{
        SystemResourceSummary(int x, int y, int w, int h);
        //@}

        /** \name Mutators */ //@{
        virtual bool  Render();

        void SetFarming (int farming ) {m_farming = farming;}
        void SetMining  (int mining  ) {m_mining  = mining;}
        void SetResearch(int research) {m_research= research;}
        void SetIndustry(int industry) {m_industry= industry;}
        void SetDefense (int defense ) {m_defense = defense;}
        //@}

      private:
        int m_farming,m_mining,m_research,m_industry,m_defense;
    };

    class PlanetView : public GG::Wnd
    {
      public:
        PlanetView(int x, int y, int w, int h,const Planet &planet);

        virtual bool Render();
        virtual void Show(bool children = true);

        int PlanetID() const {return m_planet_id;}

        bool ShowUI() const {return m_bShowUI;}

        void SetFadeInPlanetView  (int start, int span);
        void SetFadeInPlanetViewUI(int start, int span);

      protected:

        void PlanetChanged();
        void PlanetProdCenterChanged();

        void BuildSelected(int idx) const;

        void PrimaryFocusClicked(int idx);
        void SecondaryFocusClicked(int idx);

        int m_planet_id;

        bool m_bShowUI;
        int m_fadein_start,m_fadein_span;
        
        void FadeIn();

        int m_transparency;

        GG::SubTexture      m_bg_image;
        GG::SubTexture      m_build_image;
        GG::SubTexture      m_foci_image;
        CUIDropDownList     *m_construction;

        GG::RadioButtonGroup *m_radio_btn_primary_focus,*m_radio_btn_secondary_focus;
        CUITextureButton *m_btn_fullscreen;


        boost::signals::connection m_connection_btn_primary_focus_changed;
        boost::signals::connection m_connection_btn_secondary_focus_changed;
        boost::signals::connection m_connection_planet_production_changed;

    };

    PlanetPanelContainer  *m_planet_panel_container;
    SystemResourceSummary *m_system_resource_summary;
};

inline std::pair<std::string, std::string> SidePanelRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _SidePanel_h_
