#ifndef Application_h
#define Application_h

#include <GG/Wnd.h>


/// This class is designed to help you make GiGi
/// tests. You just create one of these,
/// and give it your test window and this takes care
/// of creating the application context where it can run happily.
class Application {
public:
    /// Create the app using command line options.
    /// An option database will be initialized from them.
    /// You need to register your options.
    Application(int argc, char** argv, unsigned width = 400, unsigned height = 300);

    /// The given window will be made visible.
    /// Then the event pump is started.
    /// This method only returns once the application quits
    void Run(std::shared_ptr<GG::Wnd> wnd);

private:
    class Impl;
    Impl* const self;
};

#endif // Application_h
