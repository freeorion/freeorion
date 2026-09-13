#ifndef _util_AndroidEnvironment_h_
#define _util_AndroidEnvironment_h_

#include <string>

#include <jni.h>

#include "Export.h"

/** The thread may already be attached to the VM (e.g. the GLThread that runs
  * Godot's rendering). Only attach here if it is not, and only detach it if
  * we attached it ourselves; detaching a runtime-attached thread aborts. */
FO_COMMON_API class ScopedJNIEnv {
public:
    ScopedJNIEnv();
    ~ScopedJNIEnv();

    ScopedJNIEnv(const ScopedJNIEnv&) = delete;
    ScopedJNIEnv& operator=(const ScopedJNIEnv&) = delete;

    JNIEnv* get() const { return m_env; }

    operator JNIEnv*() const { return m_env; }

    JNIEnv* operator->() const { return m_env; }

    static bool CopyPythonLib();

    static jweak Context();
private:
    JNIEnv* m_env = nullptr;
    bool m_attached = false;
};

//! Sets android environment to access android.content.Context
FO_COMMON_API void SetAndroidEnvironment(JNIEnv* env, jobject context, bool copy_python_lib);

//! Gets locale language from android anvironment
FO_COMMON_API std::string GetAndroidLang();

#endif

