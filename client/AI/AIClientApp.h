#ifndef _AIClientApp_h_
#define _AIClientApp_h_

#include "../ClientApp.h"
#include <vector>

class AIBase;
class PythonAI;

/** the application framework for an AI player FreeOrion client.*/
class AIClientApp : public ClientApp {
public:
    /** \name Structors */ //@{
    AIClientApp(const std::vector<std::string>& args);
    ~AIClientApp();
    //@}

    /** \name Mutators */ //@{
    void                operator()();   ///< external interface to Run()
    void                Exit(int code); ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()
    void                SetPlayerName(const std::string& player_name) { m_player_name = player_name; }
    //@}

    /** \name Accessors */ //@{
    const std::string&  PlayerName() const { return m_player_name; }
    virtual int         EffectsProcessingThreads() const;
    //@}

    static AIClientApp* GetApp();       ///< returns a AIClientApp pointer to the singleton instance of the app
    const AIBase*       GetAI();        ///< returns pointer to AIBase implementation of AI for this client

private:
    void                Run();          ///< initializes app state, then executes main event handler/render loop (PollAndRender())
    void                ConnectToServer();
    void                StartPythonAI();
    void                HandlePythonAICrash();
    void                HandleMessage(const Message& msg);

    std::unique_ptr<PythonAI> m_AI;       ///< implementation of AI logic
    std::string         m_player_name;
    int                 m_max_aggression;
};

#endif // _AIClientApp_h_

