#include <jni.h>
#include <fstream>
#include <sstream>
#include "jniutils.cpp"
#include <sys/stat.h>
#include "../exceptions/jexception.cpp"

class utils {

    public: static bool isStreamFailed(JNIEnv *env, std::ios &ios, jstring path) {
        if (ios.fail()) {
            jexception::throwIOException(env, "Failed load: " + jniutils::to_string(env, path));
            return false;
        }
        return true;
    }

    public: static inline bool exists(const std::string& fileName) {
        struct stat buffer{};
        return (stat (fileName.c_str(), &buffer) == 0);
    }
};