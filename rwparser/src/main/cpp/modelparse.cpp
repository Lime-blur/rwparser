#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <jni.h>
#include "renderware/renderware.h"
#include "utils/utils.cpp"
#include "utils/jlogs.h"
#include <ConverterGLTF.h>

using namespace std;
using namespace rw;

extern "C" jint Java_ru_limedev_rwparser_ModelParser_putDffDumpIntoFileNative(
	JNIEnv* env,
	jobject,
	jstring jInFilePath,
	jstring jOutFilePath,
	jboolean jIsDetailedDump
) {
	HeaderInfo header{};
	char *inFile = jniutils::to_char_ptr(env, jInFilePath);
	char *outFile = jniutils::to_char_ptr(env, jOutFilePath);
	ifstream in(inFile, ios::binary);
	if (!utils::isStreamFailed(env, in, jInFilePath)) return -1;
	string dump;
	while (header.read(in) && header.type != CHUNK_NAOBJECT) {
		if (header.type == CHUNK_CLUMP) {
			in.seekg(-12, ios::cur);
			auto *clump = new Clump;
			clump->read(in);
			dump += clump->getDump((bool) (jIsDetailedDump == JNI_TRUE));
			delete clump;
		}
	}
	in.close();
	ofstream out(outFile, ios::binary);
	if (!utils::isStreamFailed(env, out, jOutFilePath)) return -1;
	out << dump;
	out.close();
	return 0;
}

extern "C" jint Java_ru_limedev_rwparser_ModelParser_putTxdDumpIntoFileNative(
    JNIEnv* env,
    jobject,
    jstring jInFilePath,
    jstring jOutFilePath
) {
    HeaderInfo header{};
    char *inFile = jniutils::to_char_ptr(env, jInFilePath);
    char *outFile = jniutils::to_char_ptr(env, jOutFilePath);
    ifstream in(inFile, ios::binary);
    if (!utils::isStreamFailed(env, in, jInFilePath)) return -1;
    string dump;
    while (header.read(in) && header.type != CHUNK_NAOBJECT) {
        if (header.type == CHUNK_TEXDICTIONARY) {
            in.seekg(-12, ios::cur);
            auto *textureDictionary = new TextureDictionary;
			textureDictionary->read(in);
            dump += textureDictionary->getDump();
            delete textureDictionary;
        }
    }
    in.close();
    ofstream out(outFile, ios::binary);
    if (!utils::isStreamFailed(env, out, jOutFilePath)) return -1;
    out << dump;
    out.close();
    return 0;
}

extern "C" jint Java_ru_limedev_rwparser_ModelParser_convertDffWithTxdToGltfNative(
    JNIEnv* env,
    jobject,
    jstring jInDffFilePath,
    jstring jOutFilePath,
    jstring jInTxdFilePath
) {
    ConverterGLTF converter;
    char *inDffFile = jniutils::to_char_ptr(env, jInDffFilePath);
    char *inTxdFile = jniutils::to_char_ptr(env, jInTxdFilePath);
    char *outFile = jniutils::to_char_ptr(env, jOutFilePath);
    ifstream inDff(inDffFile, ios::binary);
    if (!utils::isStreamFailed(env, inDff, jInDffFilePath)) return -1;
    ifstream inTxd(inTxdFile, ios::binary);
    if (!utils::isStreamFailed(env, inTxd, jInTxdFilePath)) return -1;
    rw::Clump clump;
    rw::TextureDictionary textureDictionary;
    clump.read(inDff);
    textureDictionary.read(inTxd);
    converter.convert(outFile, clump, textureDictionary);
    inDff.close();
    inTxd.close();
    return 0;
}

extern "C" jint Java_ru_limedev_rwparser_ModelParser_convertDffToGltfNative(
    JNIEnv* env,
    jobject,
    jstring jInFilePath,
    jstring jOutFilePath
) {
    ConverterGLTF converter;
    char *inFile = jniutils::to_char_ptr(env, jInFilePath);
    char *outFile = jniutils::to_char_ptr(env, jOutFilePath);
    ifstream in(inFile, ios::binary);
    if (!utils::isStreamFailed(env, in, jInFilePath)) return -1;
    rw::Clump clump;
    clump.read(in);
    converter.convert(outFile, clump);
    in.close();
    return 0;
}