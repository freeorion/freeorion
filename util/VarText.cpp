#include "VarText.h"

#include "AppInterface.h"
#include "../universe/UniverseObject.h"
#include "../Empire/TechManager.h"


const std::string VarText::PLANET_ID_TAG = "m_planet";
const std::string VarText::SYSTEM_ID_TAG = "m_system";
const std::string VarText::TECH_ID_TAG = "m_tech";
const std::string VarText::SHIP_ID_TAG = "m_ship";

VarText::VarText(const GG::XMLElement& elem)
{
  // copy variables
  for ( int i = 0; i < elem.NumChildren(); i++ )
  {
     m_variables.AppendChild( elem.Child( i ) );
  }   
}


GG::XMLElement VarText::XMLEncode() const
{
  GG::XMLElement retval;

  for ( int i = 0; i < m_variables.NumChildren(); i++ )
  {
    retval.AppendChild( m_variables.Child( i ) );
  }   

  return retval;
}


void VarText::GenerateVarText( std::string template_str )
{
  // generates a string complete with substituted variables and hyperlinks
  // the procedure here is to replace any tokens within %% with variables of the same name in the SitRep XML data

  // get template string
  std::string final_str;

  // begin parsing
  int len = template_str.size();
  std::string::iterator next_token;
  std::string::iterator end_token;
  std::string::iterator cur_pos = template_str.begin();
  GG::XMLElement token_elem;
  Universe* universe = &GetUniverse();
    
  for ( int i = 0; i < len; i++ )
  {
    next_token = std::find( cur_pos, template_str.end(), '%');

    if ( next_token == template_str.end() )
    {
       final_str += std::string( cur_pos, template_str.end() );
       break;
    }

    // copy what we have, update cur pos
    final_str += std::string( cur_pos, next_token);

    cur_pos = next_token+1;

    end_token = std::find( cur_pos, template_str.end(), '%');

    if ( end_token == template_str.end() )
    {
       throw std::runtime_error("Missing closing % in VarText template string.");
    }


    // update cur pos 
    cur_pos = end_token + 1;
    
    // extract token
    std::string token( next_token+1, end_token );    

    // look up child
    if (!m_variables.ContainsChild( token )) 
    {
      throw std::runtime_error("The variable defined in template string does not exist in VarText data");
    }

    token_elem = m_variables.Child( token );

    // extract string based on token type
    if ( token == PLANET_ID_TAG || token == SYSTEM_ID_TAG || token == SHIP_ID_TAG )
    {
      // it's a universe object
      int object_id = boost::lexical_cast<int>( token_elem.Attribute("value") );
      
      // look up object
      UniverseObject* the_object = universe->Object( object_id );

      // set name
      if ( !the_object )
      {      
	throw std::runtime_error("The VarText variable does not exist in game data");
      }
      final_str += the_object->Name();      
    }
    else if ( token == TECH_ID_TAG )
    {
      int tech_id = boost::lexical_cast<int>( token_elem.Attribute("value") );
      
      TechLevel *tech_level = TechManager::instance().Lookup( tech_id );

      if ( !tech_level )
      {
	throw std::runtime_error("The VarText variable does not exist in game data");
      }

      // set name
      final_str += tech_level->GetName();      
      	
    }

  }
  
  // set string
  m_text = final_str;


}
