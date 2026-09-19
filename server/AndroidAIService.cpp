#include "AndroidAIService.h"

#include "../util/AndroidEnvironment.h"
#include "../util/Logger.h"

namespace {
    void ControlAndroidService(const std::string& class_name, bool start, const std::vector<std::string>& args = {}) {
        ScopedJNIEnv env;
        jobject context = env->NewLocalRef(ScopedJNIEnv::Context());
        jclass service_cls = env->FindClass(class_name.c_str());
        jclass intent_cls = env->FindClass("android/content/Intent");
        jmethodID intent_ctor_mid = env->GetMethodID(intent_cls, "<init>", "(Landroid/content/Context;Ljava/lang/Class;)V");
        jobject intent = env->NewObject(intent_cls, intent_ctor_mid, context, service_cls);

        if (!args.empty()) {
            jclass string_cls = env->FindClass("java/lang/String");
            jobjectArray args_array = env->NewObjectArray(args.size(), string_cls, nullptr);
            for (size_t i = 0; i < args.size(); ++i) {
                jstring str = env->NewStringUTF(args[i].c_str());
                env->SetObjectArrayElement(args_array, i, str);
                env->DeleteLocalRef(str);
            }

            jmethodID put_extra_mid = env->GetMethodID(intent_cls, "putExtra", "(Ljava/lang/String;[Ljava/lang/String;)Landroid/content/Intent;");
            jstring key = env->NewStringUTF("args");
            env->CallObjectMethod(intent, put_extra_mid, key, args_array);
            env->DeleteLocalRef(key);
            env->DeleteLocalRef(args_array);
        }

        jclass context_cls = env->GetObjectClass(context);
        if (start) {
            jmethodID start_service_mid = env->GetMethodID(context_cls, "startService", "(Landroid/content/Intent;)Landroid/content/ComponentName;");
            env->CallObjectMethod(context, start_service_mid, intent);
            DebugLogger() << "AndroidAIService: startService for " << class_name;
        } else {
            jmethodID stop_service_mid = env->GetMethodID(context_cls, "stopService", "(Landroid/content/Intent;)Z");
            env->CallBooleanMethod(context, stop_service_mid, intent);
            DebugLogger() << "AndroidAIService: stopService for " << class_name;
        }

        env->DeleteLocalRef(intent);
    }
}

AndroidAIService::AndroidAIService(int slot_id, const std::vector<std::string>& args)
    : m_slot_id(slot_id)
{ ControlAndroidService("org/freeorion/godot/FreeOrionAIService" + std::to_string(m_slot_id), true, args); }

AndroidAIService::~AndroidAIService()
{ Kill(); }

AndroidAIService::AndroidAIService(AndroidAIService&& rhs) noexcept
    : m_slot_id(rhs.m_slot_id)
{ rhs.m_killed = true; }

AndroidAIService& AndroidAIService::operator=(AndroidAIService&& rhs) noexcept {
    if (this != &rhs) {
        m_slot_id = rhs.m_slot_id;
        m_killed = false;
        rhs.m_killed = true;
    }
    return *this;
}

void AndroidAIService::Kill() {
    if (!m_killed) {
        DebugLogger() << "AndroidAIService: killing slot " << m_slot_id;
        ControlAndroidService("org/freeorion/godot/FreeOrionAIService" + std::to_string(m_slot_id), false);
        m_killed = true;
    }
}

void AndroidAIService::Free()
{ DebugLogger() << "AndroidAIService: freeing slot " << m_slot_id; m_killed = true; }