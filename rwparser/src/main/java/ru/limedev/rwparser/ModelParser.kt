package ru.limedev.rwparser

class ModelParser {

    /**
     * Parses the dff file and places the dump in the specified file.
     * @param jInFilePath dff file
     * @param jOutFilePath the file to place dff's dump
     * @param jIsDetailedDump true - for a detailed dump, false - for a non-detailed dump. Default is false
     * @return [ParseResult.SUCCESS] - if the operation was successful,
     * [ParseResult.ERROR] - if a failure occurred.
     */
    fun putDffDumpIntoFile(
        jInFilePath: String,
        jOutFilePath: String,
        jIsDetailedDump: Boolean = false
    ): ParseResult {
        return try {
            val parseResult = putDffDumpIntoFileNative(jInFilePath, jOutFilePath, jIsDetailedDump)
            if (parseResult == 0) ParseResult.SUCCESS else ParseResult.ERROR
        } catch (e: Exception) {
            ParseResult.ERROR
        }
    }

    /**
     * Parses the txd file and places the dump in the specified file.
     * @param jInFilePath txd file
     * @param jOutFilePath the file to place txd's dump
     * @return [ParseResult.SUCCESS] - if the operation was successful,
     * [ParseResult.ERROR] - if a failure occurred.
     */
    fun putTxdDumpIntoFile(jInFilePath: String, jOutFilePath: String): ParseResult {
        return try {
            val parseResult = putTxdDumpIntoFileNative(jInFilePath, jOutFilePath)
            if (parseResult == 0) ParseResult.SUCCESS else ParseResult.ERROR
        } catch (e: Exception) {
            ParseResult.ERROR
        }
    }

//    /**
//     * Converts the dff file to gltf file.
//     * @param jInFilePath dff file
//     * @param jOutFilePath gltf file
//     * @return [ParseResult.SUCCESS] - if the operation was successful,
//     * [ParseResult.ERROR] - if a failure occurred.
//     */
//    fun convertDffToGltf(jInFilePath: String, jOutFilePath: String): ParseResult {
//        return try {
//            val parseResult = convertDffToGltfNative(jInFilePath, jOutFilePath)
//            if (parseResult == 0) ParseResult.SUCCESS else ParseResult.ERROR
//        } catch (e: Exception) {
//            ParseResult.ERROR
//        }
//    }

    private external fun putDffDumpIntoFileNative(
        jInFilePath: String,
        jOutFilePath: String,
        jIsDetailedDump: Boolean
    ): Int

    private external fun putTxdDumpIntoFileNative(jInFilePath: String, jOutFilePath: String): Int

//    private external fun convertDffToGltfNative(jInFilePath: String, jOutFilePath: String): Int

    companion object {
        init { System.loadLibrary("rwparser") }
    }
}