package ru.limedev.rwparser

import android.util.Log
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class ModelParser {

    private var parseResourcesJob = CoroutineScope(Job() + Dispatchers.IO)

    /**
     * Asynchronously parses the dff file and places the dump in the specified file.
     * @param inFilePath path to dff file
     * @param outFilePath path to the file to place dff's dump
     * @param isDetailedDump true - for a detailed dump, false - for a non-detailed dump. Default is false
     * @param callback is triggered after parsing with the corresponding [ParseResult].
     */
    fun putDffDumpIntoFileAsync(
        inFilePath: String,
        outFilePath: String,
        isDetailedDump: Boolean = false,
        callback: (ParseResult) -> Unit
    ) {
        parseResourcesJob.launch {
            try {
                val parseResult = putDffDumpIntoFileNative(inFilePath, outFilePath, isDetailedDump)
                withContext(Dispatchers.Main) {
                    callback.invoke(if (parseResult == 0) ParseResult.SUCCESS else ParseResult.ERROR)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    Log.e(LOCAL_LOG_TAG, e.toString())
                    callback.invoke(ParseResult.ERROR)
                }
            }
        }
    }

    /**
     * Asynchronously parses the txd file and places the dump in the specified file.
     * @param inFilePath path to txd file
     * @param outFilePath path to the file to place txd's dump
     * @param callback is triggered after parsing with the corresponding [ParseResult].
     */
    fun putTxdDumpIntoFileAsync(
        inFilePath: String,
        outFilePath: String,
        callback: (ParseResult) -> Unit
    ) {
        parseResourcesJob.launch {
            try {
                val parseResult = putTxdDumpIntoFileNative(inFilePath, outFilePath)
                withContext(Dispatchers.Main) {
                    callback.invoke(if (parseResult == 0) ParseResult.SUCCESS else ParseResult.ERROR)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    Log.e(LOCAL_LOG_TAG, e.toString())
                    callback.invoke(ParseResult.ERROR)
                }
            }
        }
    }

    /**
     * Asynchronously converts the dff file to gltf file.
     * @param inDffFilePath path to dff file
     * @param outFilePath path to output gltf file
     * @param inTxdFilePath path to txd file (optional)
     * @param viewType the initial rotation of the object (optional)
     * @param callback is triggered after converting with the corresponding [ParseResult].
     */
    fun convertDffToGltfAsync(
        inDffFilePath: String,
        outFilePath: String,
        inTxdFilePath: String? = null,
        viewType: ViewType = ViewType.MODEL_ROOF,
        callback: (ParseResult) -> Unit
    ) {
        parseResourcesJob.launch {
            try {
                val parseResult = if (inTxdFilePath != null) {
                    convertDffWithTxdToGltfNative(
                        jInFilePath = inDffFilePath,
                        jOutFilePath = outFilePath,
                        jInTxdFilePath = inTxdFilePath,
                        jRx = viewType.rotationX,
                        jRy = viewType.rotationY,
                        jRz = viewType.rotationZ
                    )
                } else {
                    convertDffToGltfNative(
                        jInFilePath = inDffFilePath,
                        jOutFilePath = outFilePath,
                        jRx = viewType.rotationX,
                        jRy = viewType.rotationY,
                        jRz = viewType.rotationZ
                    )
                }
                withContext(Dispatchers.Main) {
                    callback.invoke(if (parseResult) ParseResult.SUCCESS else ParseResult.ERROR)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    Log.e(LOCAL_LOG_TAG, e.toString())
                    callback.invoke(ParseResult.ERROR)
                }
            }
        }
    }

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
            Log.e(LOCAL_LOG_TAG, e.toString())
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
            Log.e(LOCAL_LOG_TAG, e.toString())
            ParseResult.ERROR
        }
    }

    /**
     * Converts the dff file to gltf file.
     * @param inDffFilePath path to dff file
     * @param outFilePath path to output gltf file
     * @param inTxdFilePath path to txd file (optional)
     * @param viewType the initial rotation of the object (optional)
     * @return [ParseResult.SUCCESS] - if the operation was successful,
     * [ParseResult.ERROR] - if a failure occurred.
     */
    fun convertDffToGltf(
        inDffFilePath: String,
        outFilePath: String,
        inTxdFilePath: String? = null,
        viewType: ViewType = ViewType.MODEL_ROOF
    ): ParseResult {
        return try {
            val parseResult = if (inTxdFilePath != null) {
                convertDffWithTxdToGltfNative(
                    jInFilePath = inDffFilePath,
                    jOutFilePath = outFilePath,
                    jInTxdFilePath = inTxdFilePath,
                    jRx = viewType.rotationX,
                    jRy = viewType.rotationY,
                    jRz = viewType.rotationZ
                )
            } else {
                convertDffToGltfNative(
                    jInFilePath = inDffFilePath,
                    jOutFilePath = outFilePath,
                    jRx = viewType.rotationX,
                    jRy = viewType.rotationY,
                    jRz = viewType.rotationZ
                )
            }
            if (parseResult) ParseResult.SUCCESS else ParseResult.ERROR
        } catch (e: Exception) {
            Log.e(LOCAL_LOG_TAG, e.toString())
            ParseResult.ERROR
        }
    }

    /**
     * Cancels all asynchronous operations. Bind this method to the lifecycle of the component in
     * which parsing / conversion operations occur.
     */
    fun destroy() { parseResourcesJob.cancel() }

    private external fun putDffDumpIntoFileNative(
        jInFilePath: String,
        jOutFilePath: String,
        jIsDetailedDump: Boolean
    ): Int

    private external fun putTxdDumpIntoFileNative(jInFilePath: String, jOutFilePath: String): Int

    /**
     * Unfinished. If you put an empty or broken dff file, it will crash.
     * Needs to be fixed in version 1.1.5.
     */
    private external fun convertDffWithTxdToGltfNative(
        jInFilePath: String,
        jOutFilePath: String,
        jInTxdFilePath: String,
        jRx: Int,
        jRy: Int,
        jRz: Int
    ): Boolean

    private external fun convertDffToGltfNative(
        jInFilePath: String,
        jOutFilePath: String,
        jRx: Int,
        jRy: Int,
        jRz: Int
    ): Boolean

    companion object {
        init { System.loadLibrary("rwparser") }
        private const val LOCAL_LOG_TAG = "ModelParser"
    }
}