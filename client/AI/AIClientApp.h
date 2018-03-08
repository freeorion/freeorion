#ifndef _AIClientApp_h_
#define _AIClientApp_h_

#include "../ClientApp.h"

#include <memory>
#include <vector>


class AIBase;
class PythonAI;

/** the application framework for an AI player FreeOrion client.*/
class AIClientApp : public ClientApp {
public:
    AIClientApp() = delete;

    AIClientApp(const std::vector<std::string>& args);

    AIClientApp(const AIClientApp&) = delete;

    AIClientApp(AIClientApp&&) = delete;

    ~AIClientApp() override;

    const AIClientApp& operator=(const AIClientApp&) = delete;

    AIClientApp& operator=(const AIClientApp&&) = delete;

    /** \name Mutators */ //@{
    void                operator()();   ///< external interface to Run()
    void                ExitApp(int code = 0); ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()
    void                SetPlayerName(const std::string& player_name) { m_player_name = player_name; }
    //@}

    /** \name Accessors */ //@{
    int EffectsProcessingThreads() const override;

    const std::string& PlayerName() const
    { return m_player_name; }
    //@}

    static AIClientApp* GetApp();       ///< returns a AIClientApp pointer to the singleton instance of the app
    const AIBase*       GetAI();        ///< returns pointer to AIBase implementation of AI for this client

private:
    void                Run();          ///< initializes app state, then executes main event handler/render loop (PollAndRender())
    void                ConnectToServer();
    void                StartPythonAI();
    void                HandlePythonAICrash();
    void                HandleMessage(const Message& msg);


    /** Implementation of AI logic. */
    std::unique_ptr<PythonAI> m_AI;

    std::string         m_player_name;
    int                 m_max_aggression;
};

#endif // _AIClientApp_h_

