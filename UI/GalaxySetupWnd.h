#ifndef _GalaxySetupWnd_h_
#define _GalaxySetupWnd_h_

#include <GG/GGFwd.h>
#include <GG/ListBox.h>

#include "../universe/Universe.h"
#include "ClientUI.h"
#include "CUIWnd.h"

class EmpireColorSelector;
class SpeciesSelector;
struct GalaxySetupData;


/** Displays game rules options */
class GameRulesPanel : public GG::Control {
public:
    static const GG::X DefaultWidth();

    /** \name Structors*/ //!@{
    GameRulesPanel(GG::X w = GG::X(FontBasedUpscale(305)), GG::Y h = GG::Y(330));
    //!@}

    /** \name Accessors*/ //!@{
    std::vector<std::pair<std::string, std::string>> GetRulesAsStrings() const;
    //!@}

    mutable boost::signals2::signal<void ()> SettingsChangedSignal;

    /** \name Mutators*/ //!@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void Render() override {}
    void Disable(bool b = true) override;
    //!@}

private:
    void DoLayout();
    void SettingChanged();

    void BoolRuleChanged(const GG::StateButton* button,
                         const std::string& rule_name);
    void IntRuleChanged(const GG::Spin<int>* spin,
                        const std::string& rule_name);

    std::map<std::string, std::string>  m_rules;

    GG::StateButton*    m_cheap_build_toggle = nullptr;
    GG::StateButton*    m_cheap_ships_toggle = nullptr;
    GG::StateButton*    m_cheap_techs_toggle = nullptr;
    GG::Label*          m_combat_rounds_label = nullptr;
    GG::Spin<int>*      m_combat_rounds_spin = nullptr;
};

/** Encapsulates the galaxy setup options so that they may be reused in the
  * GalaxySetupWnd and the MultiPlayerLobbyWnd. */
class GalaxySetupPanel : public GG::Control {
public:
    static const GG::X DefaultWidth();

    /** \name Structors*/ //!@{
    GalaxySetupPanel(GG::X w = GG::X(FontBasedUpscale(305)), GG::Y h = GG::Y(330));
    //!@}

    /** \name Accessors*/ //!@{
    const std::string&              GetSeed() const;                //!< Returns string version of seed. This value is converted to a number or (if that fails) hashed to get the actual seed value.
    int                             Systems() const;                //!< Returns the number of star systems to use in generating the galaxy
    Shape                           GetShape() const;               //!< Returns the shape of the galaxy
    GalaxySetupOption               GetAge() const;                 //!< Returns the age of the galaxy
    GalaxySetupOption               GetStarlaneFrequency() const;   //!< Returns the frequency of starlanes in the galaxy
    GalaxySetupOption               GetPlanetDensity() const;       //!< Returns the density of planets within systems
    GalaxySetupOption               GetSpecialsFrequency() const;   //!< Returns the rarity of planetary and system specials
    GalaxySetupOption               GetMonsterFrequency() const;    //!< Returns the frequency of space monsters
    GalaxySetupOption               GetNativeFrequency() const;     //!< Returns the frequency of natives
    Aggression                      GetAIAggression() const;        //!< Returns the  maximum AI aggression level 

    /** Returns the current preview image texture. */
    std::shared_ptr<GG::Texture> PreviewImage() const;

    /** the settings changed signal object for this GalaxySetupPanel */
    mutable boost::signals2::signal<void ()>  SettingsChangedSignal;

    /** the image changed signal object for this GalaxySetupPanel */
    mutable boost::signals2::signal<void (std::shared_ptr<GG::Texture>)> ImageChangedSignal;
    //!@}

    /** \name Mutators*/ //!@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void Render() override {}
    void Disable(bool b = true) override;
    void SetFromSetupData(const GalaxySetupData& setup_data); ///< sets the controls from a GalaxySetupData
    void GetSetupData(GalaxySetupData& setup_data) const;     ///< fills values in \a setup_data from the panel's current state
    //!@}

private:
    void DoLayout();
    void RandomClicked();
    void SettingChanged();
    void ShapeChanged(GG::ListBox::iterator it);

    GG::Label*          m_seed_label = nullptr;
    GG::Edit*           m_seed_edit = nullptr;            //!< The seed used in the generation of the galaxy
    GG::Button*         m_random = nullptr;               //!< Random seed button
    GG::Label*          m_stars_label = nullptr;
    GG::Spin<int>*      m_stars_spin = nullptr;           //!< The number of stars to include in the galaxy
    GG::Label*          m_galaxy_shapes_label = nullptr;
    GG::DropDownList*   m_galaxy_shapes_list = nullptr;   //!< The possible shapes for the galaxy
    GG::Label*          m_galaxy_ages_label = nullptr;
    GG::DropDownList*   m_galaxy_ages_list = nullptr;     //!< The possible ages for the galaxy
    GG::Label*          m_starlane_freq_label = nullptr;
    GG::DropDownList*   m_starlane_freq_list = nullptr;   //!< The frequency of starlanes in the galaxy
    GG::Label*          m_planet_density_label = nullptr;
    GG::DropDownList*   m_planet_density_list = nullptr;  //!< The density of planets in each system
    GG::Label*          m_specials_freq_label = nullptr;
    GG::DropDownList*   m_specials_freq_list = nullptr;   //!< The frequency of specials in systems and on planets
    GG::Label*          m_monster_freq_label = nullptr;
    GG::DropDownList*   m_monster_freq_list = nullptr;    //!< The frequency of monsters
    GG::Label*          m_native_freq_label = nullptr;
    GG::DropDownList*   m_native_freq_list = nullptr;     //!< The frequency of natives
    GG::Label*          m_ai_aggression_label = nullptr;
    GG::DropDownList*   m_ai_aggression_list = nullptr;   //!< The max aggression choices for AI opponents

    /** Textures for galaxy previews. */
    std::vector<std::shared_ptr<GG::Texture>> m_textures;
};

//! This class is the Galaxy Setup window.  It is a modal window
//! that allows the user to choose a galaxy style, size, etc.
class GalaxySetupWnd : public CUIWnd {
public:
    /** \name Structors*/ //!@{
    GalaxySetupWnd();
    //!@}

    /** \name Accessors*/ //!@{
    /** returns true iff the dialog is finished running and it was closed with the "OK" button */
    bool                    EndedWithOk() const {return m_done && m_ended_with_ok;}

    /** returns the panel containing all the user-chosen options. */
    //const GalaxySetupPanel& Panel()      const  {return *m_galaxy_setup_panel;}
    const std::string&      EmpireName() const;
    GG::Clr                 EmpireColor() const;
    const std::string&      StartingSpeciesName() const;
    int                     NumberAIs() const;
    std::vector<std::pair<std::string, std::string>>
                            GetRulesAsStrings() const;
    //!@}

    /** \name Mutators*/ //!@{
    void Render() override;
    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    //!@}

protected:
    GG::Rect CalculatePosition() const override;

private:
    void DoLayout();
    void PreviewImageChanged(std::shared_ptr<GG::Texture> new_image);
    void EmpireNameChanged(const std::string& name);
    void PlayerNameChanged(const std::string& name);
    void OkClicked();
    void CancelClicked();

    bool m_ended_with_ok = false;   //!< indicates whether or not we ended the dialog with OK or not

    GalaxySetupPanel*       m_galaxy_setup_panel = nullptr;    //!< The GalaxySetupPanel that does most of the work of the dialog
    GameRulesPanel*         m_game_rules_panel = nullptr;
    GG::Label*              m_player_name_label = nullptr;
    GG::Edit*               m_player_name_edit = nullptr;
    GG::Label*              m_empire_name_label = nullptr;
    GG::Edit*               m_empire_name_edit = nullptr;
    GG::Label*              m_empire_color_label = nullptr;
    EmpireColorSelector*    m_empire_color_selector = nullptr;
    SpeciesSelector*        m_starting_secies_selector = nullptr;
    GG::Label*              m_starting_species_label = nullptr;
    GG::Label*              m_number_ais_label = nullptr;
    GG::Spin<int>*          m_number_ais_spin = nullptr;
    GG::StaticGraphic*      m_preview_image = nullptr;         //!< The galaxy shape preview image
    GG::Button*             m_ok = nullptr;                    //!< OK button
    GG::Button*             m_cancel = nullptr;                //!< Cancel button

    GG::Pt                  m_preview_ul;
};

#endif // _GalaxySetupWnd_h_
