// -*- C++ -*-
#ifndef _Tech_h_
#define _Tech_h_

#include <string>

/**
* This is a class to represent a level of technological advancement
*  It's fields are an identifying number, a name, and the minimum RP
*  investment needed in order to acquire it
*
*  Only the TechManager should create Tech objects
*
*  For Version 0.1, the TechManager will create four hardcoded tech
* levels.
*/
class Tech
{
    /// TechManager is a friend so that it, and only it, can
    /// create Tech objects
    friend class TechManager;
    
public:
    
    /**
     * this enum is only to define contants for the different
     * technology types that will appear in V0.1 (these will be 
     * hardcoded until some kind of tech tree data structure is built)
     * The TechManager will create techlevels for each element of this
     * enum when the LoadTechTree method is called.  
     */
    enum TechType {INVALID_TECH = 0, 
                   TECH_BASE = 1,       ///< Defense bases
                   TECH_MARK2 = 2,      ///< Mark 2 ships
                   TECH_MARK3 = 3,      ///< Mark 3 ships
                   TECH_MARK4 = 4       ///< Mark 4 ships
    };
 
    /** \name Accessors */ //{@
    
    /** 
     * GetID returns a unique numeric ID associated with this
     * technology level.  For version 0.1, return value is guaranteed
     * to be an element of the above enum
     */
    int GetID() const;
    
    /**
     * GetMinPts returns the minimum number of accumulated
     * research points required for an empire to possess this tech
     */        
    int GetMinPts() const;
    
    /** 
     * GetName returns the name of this technology level
     */
    const std::string& GetName() const;
    
    //@}
    
protected:
    // protected is used so that derived classes have 
    // access to base class constructor
      
    /** \name Constructors */ //{@
    Tech(int ID, std::string name, int MinPts);
    //@}

private:
    int m_id;
    int m_min_pts;
    std::string m_name;
};

#endif
