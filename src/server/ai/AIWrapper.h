#ifndef _AIWrapper_h_
#define _AIWrapper_h_

#include <string>

namespace FreeOrionPython {
    // getter and setter for static s_save_state_string to be exposed to Python
    const std::string& GetStaticSaveStateString();
    void  SetStaticSaveStateString(const std::string& new_state_string);
    void  ClearStaticSaveStateString();

    // AI interface wrapper
    void WrapAI();
}


#endif
