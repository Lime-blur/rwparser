#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <jni.h>
#include "utils/utils.cpp"
#include "utils/jlogs.h"
#include <ConverterGLTF.h>

using namespace std;

extern "C" jboolean Java_ru_limedev_rwparser_ModelParser_convertDffWithTxdToGltfNative(
    JNIEnv* env,
    jobject,
    jstring jInDffFilePath,
    jstring jOutFilePath,
    jstring jInTxdFilePath,
    jint jRx,
    jint jRy,
    jint jRz
) {
    ConverterGLTF converter;
    char *inDffFile = jniutils::to_char_ptr(env, jInDffFilePath);
    char *inTxdFile = jniutils::to_char_ptr(env, jInTxdFilePath);
    char *outFile = jniutils::to_char_ptr(env, jOutFilePath);
    bool isCorrectFile;
    converter.setRotation(jRx, jRy, jRz);
    isCorrectFile = converter.convert(outFile, inDffFile, inTxdFile, true);
    if (!utils::exists(outFile)) isCorrectFile = false;
    return isCorrectFile;
}

extern "C" jboolean Java_ru_limedev_rwparser_ModelParser_convertDffToGltfNative(
    JNIEnv* env,
    jobject,
    jstring jInFilePath,
    jstring jOutFilePath,
    jint jRx,
    jint jRy,
    jint jRz
) {
    ConverterGLTF converter;
    char *inFile = jniutils::to_char_ptr(env, jInFilePath);
    char *outFile = jniutils::to_char_ptr(env, jOutFilePath);
    bool isCorrectFile;
    converter.setRotation(jRx, jRy, jRz);
    isCorrectFile = converter.convert(outFile, inFile);
    if (!utils::exists(outFile)) isCorrectFile = false;
    return isCorrectFile;
}
