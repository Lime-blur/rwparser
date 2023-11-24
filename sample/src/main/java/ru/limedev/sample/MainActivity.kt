package ru.limedev.sample

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.os.Environment
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import ru.limedev.rwparser.ModelParser
import ru.limedev.rwparser.ParseResult
import ru.limedev.sample.databinding.ActivityMainBinding
import java.io.File

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private var currentOperation: ParseOperation? = null
    private var currentDffUri: Uri? = null
    private var isAsync: Boolean? = null
    private var modelParser: ModelParser? = null

    private var parseResultLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) launcher@ { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            val uri = result.data?.data
            val fileName = uri?.getName(this) ?: return@launcher
            val fileExtension = uri.getExtension(this) ?: return@launcher
            val documentsFile = uri.toInternalDocumentsFile(this, fileName) ?: return@launcher
            parse(fileName, fileExtension, documentsFile, currentOperation == ParseOperation.PARSE)
        }
    }

    private var convertDffResultLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) launcher@ { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            val uri = result.data?.data
            if (uri?.getExtension(this) != DFF_EXTENSION) return@launcher
            currentDffUri = uri
            val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply { type = "*/*" }
            convertTxdResultLauncher.launch(intent)
        }
    }

    private var convertTxdResultLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) launcher@ { result ->
        var currentTxdUri: Uri? = null
        if (result.resultCode == Activity.RESULT_OK) {
            val uri = result.data?.data
            currentTxdUri = uri
        }
        val dffFileName = currentDffUri?.getName(this) ?: return@launcher
        val dffFile = currentDffUri?.toInternalDocumentsFile(this, dffFileName) ?: return@launcher
        val txdFileName = currentTxdUri?.getName(this)
        val txdFile = currentTxdUri?.toInternalDocumentsFile(this, txdFileName)
        convert(dffFileName, dffFile, txdFileName, txdFile)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        modelParser = ModelParser()
        binding.button.setOnClickListener {
            currentOperation = ParseOperation.PARSE
            handleOpeningFile()
        }
        binding.button2.setOnClickListener {
            currentOperation = ParseOperation.SHORT_PARSE
            handleOpeningFile()
        }
        binding.button3.setOnClickListener {
            currentOperation = ParseOperation.CONVERT
            handleOpeningFile()
        }
        binding.button4.setOnClickListener {
            isAsync = true
            currentOperation = ParseOperation.PARSE
            handleOpeningFile()
        }
        binding.button5.setOnClickListener {
            isAsync = true
            currentOperation = ParseOperation.SHORT_PARSE
            handleOpeningFile()
        }
        binding.button6.setOnClickListener {
            isAsync = true
            currentOperation = ParseOperation.CONVERT
            handleOpeningFile()
        }
    }

    override fun onDestroy() {
        modelParser?.destroy()
        super.onDestroy()
    }

    private fun handleOpeningFile() {
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply { type = "*/*" }
        when (currentOperation) {
            ParseOperation.PARSE, ParseOperation.SHORT_PARSE -> parseResultLauncher.launch(intent)
            ParseOperation.CONVERT -> convertDffResultLauncher.launch(intent)
            else -> Unit
        }
        currentOperation = null
    }

    private fun parse(fileName: String, fileExtension: String, documentsFile: File, detailed: Boolean) {
        val inFilePath = getInternalDocumentsFilePath(fileName) ?: return
        val outFilePath = getInternalDocumentsFilePath("$RESULT_FILE_NAME$DUMP_EXTENSION") ?: return
        if (isAsync == true) {
            when (fileExtension) {
                DFF_EXTENSION -> modelParser?.putDffDumpIntoFileAsync(
                    inFilePath = inFilePath,
                    outFilePath = outFilePath,
                    isDetailedDump = detailed
                ) { parseResult ->
                    handleParseResult(parseResult, outFilePath)
                    documentsFile.delete()
                }
                TXD_EXTENSION -> modelParser?.putTxdDumpIntoFileAsync(
                    inFilePath = inFilePath,
                    outFilePath = outFilePath
                ) { parseResult ->
                    handleParseResult(parseResult, outFilePath)
                    documentsFile.delete()
                }
                else -> handleParseResult(ParseResult.ERROR, outFilePath)
            }
        } else {
            val parseResult = when (fileExtension) {
                DFF_EXTENSION -> modelParser?.putDffDumpIntoFile(inFilePath, outFilePath, detailed)
                TXD_EXTENSION -> modelParser?.putTxdDumpIntoFile(inFilePath, outFilePath)
                else -> ParseResult.ERROR
            }
            handleParseResult(parseResult, outFilePath)
            documentsFile.delete()
        }
    }

    private fun convert(dffFileName: String, dffFile: File, txdFileName: String?, txdFile: File?) {
        val dffFilePath = getInternalDocumentsFilePath(dffFileName) ?: return
        val txdFilePath = if (txdFileName == null) null else getInternalDocumentsFilePath(txdFileName)
        val gltfFilePath = getInternalDocumentsFilePath("$RESULT_FILE_NAME$GLTF_EXTENSION") ?: return
        if (isAsync == true) {
            modelParser?.convertDffToGltfAsync(dffFilePath, gltfFilePath, txdFilePath) { parseResult ->
                handleParseResult(parseResult, gltfFilePath)
                dffFile.delete()
                txdFile?.delete()
            }
        } else {
            val parseResult = modelParser?.convertDffToGltf(dffFilePath, gltfFilePath, txdFilePath)
            handleParseResult(parseResult, gltfFilePath)
            dffFile.delete()
            txdFile?.delete()
        }
    }

    private fun handleParseResult(parseResult: ParseResult?, outFilePath: String) {
        binding.sampleText.text = when (parseResult) {
            ParseResult.SUCCESS -> "Saved in: $outFilePath"
            else -> "Couldn't parse the file!"
        }
        isAsync = null
    }

    private fun getInternalDocumentsFilePath(fileName: String): String? {
        val docsDir = getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS)?.absolutePath ?: return null
        return "$docsDir${separator}$fileName"
    }
}