#include <jni.h>
#include <string>

class jexception {

    public: static jint throwIOException(JNIEnv* env, const char *message) {
        char const *className = "java/io/IOException";
        jclass exClass = env->FindClass(className);
        if (exClass == nullptr) return -1;
        return env->ThrowNew(exClass, message);
    }

    public: static jint throwIOException(JNIEnv* env, const std::string& message) {
        return throwIOException(env, message.c_str());
    }
};
