#ifndef TESTAPPIMPL_H
#define TESTAPPIMPL_H

class TestAppImpl {
    public:
        TestAppImpl (TestApp* q, int argc, char** argv, unsigned width, unsigned height);
        virtual ~TestAppImpl();

        /// The given window will be made visible.
        /// Then the event pump is started.
        /// This method only returns once the application quits
        void Run(GG::Wnd* wnd);

    private:
        class TestApp* const m_front;
        class MinimalGGApp* m_app;
};

#endif // TESTAPPIMPL_H
