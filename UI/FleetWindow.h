
#ifndef _FleetWindow_h_
#define _FleetWindow_h_

#ifndef _CUI_Wnd_h_
#include "./CUI_Wnd.h"
#endif


#include "GGListBox.h"
#include "GGButton.h"
#include "GGEdit.h"


class FleetWindow : public CUI_Wnd
{
public:

    enum DialogMode
    {
        FLEET_VIEW = 0,
        FLEET_MERGE = 1,
        FLEET_SPLIT = 2
    };
    

    FleetWindow(int x, int y, int active_fleet, std::vector<int> other_fleets);
    FleetWindow(const GG::XMLElement& elem);
    ~FleetWindow();
    
    /// Callback method which is invoked when user switches selected fleet
    void OnSwitchFleet();
    
    /// Callback method which is invoked when user clicks 'merge fleets' button
    void MergeButtonClicked();
    
    /// Callback method which is invoked when user clicks 'split fleet' button
    void SplitButtonClicked();
    
    /// Callback method which is invoked when user clicks 'move' button
    void MoveButtonClicked();
    
    /// Callback method which is invoked when user clicks the 'done' button
    void DoneButtonClicked();
    
    /// Sets the current dialog mode for the fleet dialog.  
    void SetDialogMode(DialogMode mode);
    
private:
    void DisplayFleet(int fleet);

    void DestroyChild(GG::ListBox*& child);
    void DestroyChild(GG::Button*& child);
    void DestroyChild(GG::TextControl*& child);

    /// listbox containing ships in current fleet (sorted by design)
    GG::ListBox* m_ship_list;
    GG::TextControl* m_ship_list_label;
    
    /// listbox for ships in the 'other' fleet, during a merge or split.
    GG::ListBox* m_receiving_fleet_list;
    GG::TextControl* m_receiving_fleet_label;
    
    /// listbox containing other fleets in same system
    GG::ListBox* m_fleets_list;
    GG::TextControl* m_fleets_list_label;
    
    /// buttons to issue orders to fleets
    GG::Button* m_split_button;
    GG::Button* m_merge_button;
    GG::Button* m_move_button;
    
    /// a 'done' button to go back to browsing mode
    GG::Button* m_done_button;
    
    
    /// fleet currently being browsed in the fleet window
    int m_active_fleet;
    
    /// other fleets in the same system
    std::vector<int> m_fleets;

};


#endif
