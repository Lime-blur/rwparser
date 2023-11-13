package ru.limedev.sample

import android.content.Context
import android.net.Uri
import android.os.Environment
import android.provider.OpenableColumns
import java.io.File
import java.io.FileOutputStream

const val EMPTY_STRING = ""
val separator: String = File.separator

fun createFile(pathname: String, replace: Boolean = true): File {
    val file = File(pathname)
    if (replace && file.exists()) file.delete()
    return file
}

fun Uri.toInternalDocumentsFile(context: Context?, fileName: String?): File? {
    if (context == null || fileName == null) return null
    val docsDir = context.getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS)?.absolutePath ?: return null
    val file = createFile("$docsDir$separator$fileName")
    writeFile(context, file)
    return file
}

fun Uri.getName(context: Context?): String? {
    val cursorToReturn = context?.contentResolver?.query(
        this, null, null, null, null
    )
    cursorToReturn?.let { cursor ->
        val nameIndex = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME)
        cursor.moveToFirst()
        val fileName = cursor.getString(nameIndex)
        cursor.close()
        return fileName
    }
    return getAlternativeName()
}

fun Uri.getExtension(context: Context?): String? {
    val name = getName(context) ?: return null
    val extension = name.lastIndexOf('.')
    return if (extension > 0) {
        name.substring(extension).lowercase()
    } else {
        EMPTY_STRING
    }
}

private fun Uri.getAlternativeName(): String? {
    val p = path ?: return null
    return p.substring(p.lastIndexOf("/") + 1)
}

private fun Uri.writeFile(context: Context?, outputFile: File) {
    context?.contentResolver?.openInputStream(this)?.use { inputStream ->
        FileOutputStream(outputFile).use { fileOutputStream ->
            inputStream.copyTo(fileOutputStream)
        }
    }
}