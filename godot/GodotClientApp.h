#ifndef _GodotClientApp_h_
#define _GodotClientApp_h_

#include "../client/ClientApp.h"

class GodotClientApp : public ClientApp {
public:
    GodotClientApp();

    GodotClientApp(const GodotClientApp&) = delete;
    GodotClientApp(GodotClientApp&&) = delete;
    ~GodotClientApp() override;

    const GodotClientApp& operator=(const GodotClientApp&) = delete;
    GodotClientApp& operator=(const GodotClientApp&&) = delete;

    int EffectsProcessingThreads() const override;

    static GodotClientApp* GetApp();
};

#endif

