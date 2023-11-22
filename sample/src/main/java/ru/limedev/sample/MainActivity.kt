package ru.limedev.sample

import android.app.Activity
import android.content.Intent
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

    private var resultLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            val uri = result.data?.data
            val fileName = uri?.getName(this)
            val fileExtension = uri?.getExtension(this)
            val documentsFile = uri?.toInternalDocumentsFile(this, fileName)
            when (currentOperation) {
                ParseOperation.PARSE -> parse(
                    fileName = fileName,
                    fileExtension = fileExtension,
                    documentsFile = documentsFile,
                    detailed = true
                )
                ParseOperation.SHORT_PARSE -> parse(
                    fileName = fileName,
                    fileExtension = fileExtension,
                    documentsFile = documentsFile,
                    detailed = false
                )
                else -> Unit
            }
            currentOperation = null
            documentsFile?.delete()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.button.setOnClickListener {
            currentOperation = ParseOperation.PARSE
            handleOpeningFile()
        }
        binding.button2.setOnClickListener {
            currentOperation = ParseOperation.SHORT_PARSE
            handleOpeningFile()
        }
//        binding.button3.setOnClickListener { handleOpeningFileToConvert() }
    }

    private fun handleOpeningFile() {
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply { type = "*/*" }
        resultLauncher.launch(intent)
    }

    private fun parse(
        fileName: String?,
        fileExtension: String?,
        documentsFile: File?,
        detailed: Boolean
    ) {
        if (fileName == null || fileExtension == null || documentsFile == null) return
        val inFilePath = getInternalDocumentsFilePath(fileName) ?: return
        val outFilePath = getInternalDocumentsFilePath("result.txt") ?: return
        val modelParser = ModelParser()
        val parseResult = when (fileExtension) {
            ".dff" -> modelParser.putDffDumpIntoFile(inFilePath, outFilePath, detailed)
            ".txd" -> modelParser.putTxdDumpIntoFile(inFilePath, outFilePath)
            else -> ParseResult.ERROR
        }
        handleParseResult(parseResult, outFilePath)
    }

//    private fun convert(fileName: String?, fileExtension: String?, documentsFile: File?) {
//        if (fileName == null || fileExtension == null || documentsFile == null) return
//        val inFilePath = getInternalDocumentsFilePath(fileName) ?: return
//        val outFilePath = getInternalDocumentsFilePath("result.gltf") ?: return
//        val modelParser = ModelParser()
//        val parseResult = when (fileExtension) {
//            ".dff" -> modelParser.convertDffToGltf(inFilePath, outFilePath)
//            else -> ParseResult.ERROR
//        }
//        handleParseResult(parseResult, outFilePath)
//    }

    private fun handleParseResult(parseResult: ParseResult, outFilePath: String) {
        binding.sampleText.text = when (parseResult) {
            ParseResult.SUCCESS -> "Saved in: $outFilePath"
            ParseResult.ERROR -> "Couldn't parse the file!"
        }
    }

    private fun getInternalDocumentsFilePath(fileName: String): String? {
        val docsDir = getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS)?.absolutePath ?: return null
        return "$docsDir${separator}$fileName"
    }
}