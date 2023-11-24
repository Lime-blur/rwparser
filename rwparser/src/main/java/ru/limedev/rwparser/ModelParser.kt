package ru.limedev.rwparser

import android.util.Log

class ModelParser {

    /**
     * Parses the dff file and places the dump in the specified file.
     * @param inFilePath path to dff file
     * @param outFilePath path to the file to place dff's dump
     * @param isDetailedDump true - for a detailed dump, false - for a non-detailed dump. Default is false
     * @return [ParseResult.SUCCESS] - if the operation was successful,
     * [ParseResult.ERROR] - if a failure occurred.
     */
    fun putDffDumpIntoFile(
        inFilePath: String,
        outFilePath: String,
        isDetailedDump: Boolean = false
    ): ParseResult {
        return try {
            val parseResult = putDffDumpIntoFileNative(inFilePath, outFilePath, isDetailedDump)
            if (parseResult == 0) ParseResult.SUCCESS else ParseResult.ERROR
        } catch (e: Exception) {
            Log.e("ModelParser", e.toString())
            ParseResult.ERROR
        }
    }

    /**
     * Parses the txd file and places the dump in the specified file.
     * @param inFilePath path to txd file
     * @param outFilePath path to the file to place txd's dump
     * @return [ParseResult.SUCCESS] - if the operation was successful,
     * [ParseResult.ERROR] - if a failure occurred.
     */
    fun putTxdDumpIntoFile(inFilePath: String, outFilePath: String): ParseResult {
        return try {
            val parseResult = putTxdDumpIntoFileNative(inFilePath, outFilePath)
            if (parseResult == 0) ParseResult.SUCCESS else ParseResult.ERROR
        } catch (e: Exception) {
            Log.e("ModelParser", e.toString())
            ParseResult.ERROR
        }
    }

    /**
     * Converts the dff file to gltf file.
     * @param inDffFilePath path to dff file
     * @param outFilePath path to output gltf file
     * @param inTxdFilePath path to txd file (optional)
     * @return [ParseResult.SUCCESS] - if the operation was successful,
     * [ParseResult.ERROR] - if a failure occurred.
     */
    fun convertDffToGltf(
        inDffFilePath: String,
        outFilePath: String,
        inTxdFilePath: String? = null
    ): ParseResult {
        return try {
            val parseResult = if (inTxdFilePath != null) {
                convertDffWithTxdToGltfNative(inDffFilePath, outFilePath, inTxdFilePath)
            } else {
                convertDffToGltfNative(inDffFilePath, outFilePath)
            }
            if (parseResult == 0) ParseResult.SUCCESS else ParseResult.ERROR
        } catch (e: Exception) {
            Log.e("ModelParser", e.toString())
            ParseResult.ERROR
        }
    }

    private external fun putDffDumpIntoFileNative(
        jInFilePath: String,
        jOutFilePath: String,
        jIsDetailedDump: Boolean
    ): Int

    private external fun putTxdDumpIntoFileNative(jInFilePath: String, jOutFilePath: String): Int

    private external fun convertDffWithTxdToGltfNative(
        jInFilePath: String,
        jOutFilePath: String,
        jInTxdFilePath: String
    ): Int

    private external fun convertDffToGltfNative(
        jInFilePath: String,
        jOutFilePath: String
    ): Int

    companion object {
        init { System.loadLibrary("rwparser") }
    }
}