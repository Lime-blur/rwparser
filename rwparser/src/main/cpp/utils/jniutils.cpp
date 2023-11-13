#include <string>
#include <jni.h>

class jniutils {

    public: static std::string to_string(JNIEnv *env, jstring jStr) {
        if (!jStr) return "";
        jclass stringClass = env->GetObjectClass(jStr);
        jmethodID getBytes = env->GetMethodID(
                stringClass,
                "getBytes",
                "(Ljava/lang/String;)[B"
        );
        const auto jStringBytes = (jbyteArray) env->CallObjectMethod(
                jStr,
                getBytes,
                env->NewStringUTF("UTF-8")
        );
        auto length = (size_t) env->GetArrayLength(jStringBytes);
        jbyte *pBytes = env->GetByteArrayElements(jStringBytes, nullptr);
        std::string ret = std::string((char *) pBytes, length);
        env->ReleaseByteArrayElements(jStringBytes, pBytes, JNI_ABORT);
        env->DeleteLocalRef(jStringBytes);
        env->DeleteLocalRef(stringClass);
        return ret;
    }

    public: static char* to_char_ptr(JNIEnv *env, jstring jStr) {
        return const_cast<char *>(env->GetStringUTFChars(jStr, nullptr));
    }
};