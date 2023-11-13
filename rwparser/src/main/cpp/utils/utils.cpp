#include <jni.h>
#include <fstream>
#include <sstream>
#include "jniutils.cpp"
#include "../exceptions/jexception.cpp"

class utils {

    public: static bool isStreamFailed(JNIEnv *env, std::ios &ios, jstring path) {
        if (ios.fail()) {
            jexception::throwIOException(env, jniutils::to_string(env, path));
            return false;
        }
        return true;
    }
};