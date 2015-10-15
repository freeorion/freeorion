#ifndef __FreeOrion__Python__AIWrapper__
#define __FreeOrion__Python__AIWrapper__

#include <string>

namespace FreeOrionPython {
    // getter and setter for static s_save_state_string to be exposed to Python
    const std::string& GetStaticSaveStateString();
    void  SetStaticSaveStateString(const std::string& new_state_string);
    void  ClearStaticSaveStateString();

    // AI interface wrapper
    void WrapAI();
}

#endif /* defined(__FreeOrion__Python__AIWrapper__) */
