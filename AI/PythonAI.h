#include "AIInterface.h"

#include <boost/python.hpp>

#include <string>

class PythonAI : public AIBase
{
public:
    /** \name structors */ //@{
    PythonAI();
    ~PythonAI();
    //@}

    void GenerateOrders();
    void HandleChatMessage(int sender_id, const std::string& msg);
};
