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

class CUIDropDownList;
class CUIScroll;
class System;

class CUIIconButton;
namespace GG {class TextControl;}


class SidePanel : public GG::Wnd
{
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
        PlanetPanel(int x, int y, int w, int h,const Planet &planet); ///< basic ctor
        ~PlanetPanel();
        //@}

        /** \name Accessors */ //@{
        virtual bool InWindow(const GG::Pt& pt) const;
        virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)

        LeftClickedSignalType& LeftClickedSignal() const {return m_left_clicked_sig;} ///< returns the left clicked signal object for this Planet panel
        //@}

        /** \name Mutators */ //@{
        virtual bool Render();
        virtual void LClick(const GG::Pt& pt, Uint32 keys);
        //@}

    protected:
        friend class SidePanel;
        void Reset();
        void Reset(const Planet &planet);
    private:

        void ConstructUnhabited(const Planet &planet);
        void ConstructInhabited(const Planet &planet);
        void ConstructOwned    (const Planet &planet);

        bool RenderUnhabited(const Planet &planet);
        bool RenderInhabited(const Planet &planet);
        bool RenderOwned    (const Planet &planet);

        void UpdateControls(const Planet &planet);

        int  PlanetDiameter() const;
        bool InPlanet(const GG::Pt& pt) const;
        void BuildSelected(int idx) const;

        void SetPrimaryFocus  (Planet::FocusType focus); 
        void SetSecondaryFocus(Planet::FocusType focus); 

        void LClickFarming () {SetPrimaryFocus(Planet::FARMING );}
        void LClickMining  () {SetPrimaryFocus(Planet::MINING  );}
        void LClickIndustry() {SetPrimaryFocus(Planet::INDUSTRY);}
        void LClickResearch() {SetPrimaryFocus(Planet::SCIENCE );}
        void LClickBalanced() {SetPrimaryFocus(Planet::BALANCED);}

        void RClickFarming () {SetSecondaryFocus(Planet::FARMING );}
        void RClickMining  () {SetSecondaryFocus(Planet::MINING  );}
        void RClickIndustry() {SetSecondaryFocus(Planet::INDUSTRY);}
        void RClickResearch() {SetSecondaryFocus(Planet::SCIENCE );}
        void RClickBalanced() {SetSecondaryFocus(Planet::BALANCED);}

        void ClickColonize();

        int m_planet_id;
        GG::TextControl     *m_planet_name;
        GG::TextControl     *m_planet_info;
        CUIButton           *m_button_colonize;
        GG::DynamicGraphic  *m_planet_graphic;
        CUIIconButton       *m_button_food,*m_button_mining,*m_button_industry,*m_button_research;
        CUIIconButton       *m_button_balanced;

        CUIDropDownList     *m_construction;

        mutable LeftClickedSignalType m_left_clicked_sig;
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
    void FleetsChanged();
    void PlanetsChanged();
    void PrevButtonClicked();
    void NextButtonClicked();

    const System        *m_system;
    CUIDropDownList     *m_system_name;
    GG::TextControl     *m_system_name_unknown;
    GG::Button          *m_button_prev,*m_button_next;
    GG::DynamicGraphic  *m_star_graphic;
    GG::TextControl     *m_static_text_systemproduction;


    std::vector<GG::SubTexture> m_fleet_icons;

    class PlanetPanelContainer : public GG::Wnd
    {
      public:
        /** \name Structors */ //@{
        PlanetPanelContainer(int x, int y, int w, int h);
        //@}

        void Clear();
        void SetPlanets(const std::vector<const Planet*> &plt_vec);

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
    PlanetPanelContainer  *m_planet_panel_container;
    SystemResourceSummary *m_system_resource_summary;
};

#endif // _SidePanel_h_
