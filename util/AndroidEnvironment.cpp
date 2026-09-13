#include "AndroidEnvironment.h"

namespace {
    thread_local JNIEnv* s_jni_env = nullptr;
    jweak                s_context;
    JavaVM*              s_java_vm;
    bool                 s_copy_python_lib;
}

void SetAndroidEnvironment(JNIEnv* env, jobject context, bool copy_python_lib)
{
    s_jni_env = env;
    s_jni_env->GetJavaVM(&s_java_vm);
    s_context = env->NewWeakGlobalRef(context);
    s_copy_python_lib = copy_python_lib;
}

std::string GetAndroidLang()
{
    std::string retval;

    ScopedJNIEnv env;

    jclass locale_class = env->FindClass("java/util/Locale");
    jmethodID get_default_mid = env->GetStaticMethodID(locale_class, "getDefault", "()Ljava/util/Locale;");
    jobject locale = env->CallStaticObjectMethod(locale_class, get_default_mid);

    jmethodID get_language_mid = env->GetMethodID(locale_class, "getLanguage", "()Ljava/lang/String;");
    jstring language = reinterpret_cast<jstring>(env->CallObjectMethod(locale, get_language_mid));

    const char *language_chars = env->GetStringUTFChars(language, nullptr);
    retval = std::string(language_chars);
    env->ReleaseStringUTFChars(language, language_chars);

    return retval;
}

ScopedJNIEnv::ScopedJNIEnv() {
    if (s_jni_env != nullptr)
        m_env = s_jni_env;
    else if (s_java_vm != nullptr)
        s_java_vm->GetEnv(reinterpret_cast<void**>(&m_env), JNI_VERSION_1_6);

    if (m_env == nullptr && s_java_vm != nullptr) {
        s_java_vm->AttachCurrentThreadAsDaemon(&m_env, nullptr);
        m_attached = true;
    }

    if (!m_env)
        throw std::runtime_error("ScopedJNIEnv: Failed to obtain JNIEnv or attach thread to JVM");
}

ScopedJNIEnv::~ScopedJNIEnv() {
    if (m_attached && s_java_vm != nullptr)
        s_java_vm->DetachCurrentThread();
}

bool ScopedJNIEnv::CopyPythonLib()
{ return s_copy_python_lib; }

jweak ScopedJNIEnv::Context()
{ return s_context; }

