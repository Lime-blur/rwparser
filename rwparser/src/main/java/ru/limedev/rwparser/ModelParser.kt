package ru.limedev.rwparser

import java.io.IOException

class ModelParser {

    /**
     * Parses the dff file and places the dump in the specified file.
     * @param jInFilePath dff file
     * @param jOutFilePath the file to place dff's dump
     * @return 0 - if the operation was successful, -1 - if a failure occurred.
     * @throws IOException if there is a failure while reading and writing files.
     */
    external fun putDffDumpIntoFile(jInFilePath: String, jOutFilePath: String): Int

    /**
     * Parses the txd file and places the dump in the specified file.
     * @param jInFilePath txd file
     * @param jOutFilePath the file to place txd's dump
     * @return 0 - if the operation was successful, -1 - if a failure occurred.
     * @throws IOException if there is a failure while reading and writing files.
     */
    external fun putTxdDumpIntoFile(jInFilePath: String, jOutFilePath: String): Int

    companion object {
        init { System.loadLibrary("rwparser") }
    }
}