# rwparser
**rwparser** is an android library for parsing and converting RenderWare dff and txd models of Grand Theft Auto. 
## Gradle Dependency
Add this dependency to your module's `build.gradle` file:

```gradle
dependencies {
  ..
  implementation 'com.github.Lime-blur:rwparser:latest_version'
}
```

Add it in your root build.gradle at the end of repositories (or in settings.gradle):

```gradle
dependencyResolutionManagement {
  repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
  repositories {
    mavenCentral()
    maven { url 'https://jitpack.io' }
  }
}
```

## Usage
### Parsing
In order to parse a `.dff` file, you simply need to create a `ModelParser()` object and call the appropriate method:

```kotlin
val modelParser = ModelParser()
val parseResult = modelParser.putDffDumpIntoFile(inFilePath, outFilePath, /* optional */ true)
```

For asynchronous parsing, you need to call the appropriate function and bind ModelParser to the component's lifecycle:

```kotlin
class MainActivity : AppCompatActivity() {

    private var modelParser: ModelParser? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        ..
        modelParser = ModelParser()
    }

    override fun onDestroy() {
        modelParser?.destroy()
        super.onDestroy()
    }

    private fun parse() {
        modelParser?.putDffDumpIntoFileAsync(inFilePath, outFilePath, /* optional */ true) { parseResult ->
            // Handling the callback
        }
    }
}
```

Same for `.txd` file:

```kotlin
val modelParser = ModelParser()
val parseResult = modelParser.putTxdDumpIntoFile(inFilePath, outFilePath)
```

For asynchronous `.txd`'s parsing:

```kotlin
class MainActivity : AppCompatActivity() {

    private var modelParser: ModelParser? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        ..
        modelParser = ModelParser()
    }

    override fun onDestroy() {
        modelParser?.destroy()
        super.onDestroy()
    }

    private fun parse() {
        modelParser?.putTxdDumpIntoFileAsync(inFilePath, outFilePath) { parseResult ->
            // Handling the callback
        }
    }
}
```

### Converting
The library can also convert `.dff` to `.gltf` format. To convert you need:


```kotlin
val modelParser = ModelParser()
val parseResult = modelParser.convertDffToGltf(inDffFilePath, outFilePath, /* optional */ inTxdFilePath, /* optional */ viewType)
```

For asynchronous `.dff`'s converting:

```kotlin
class MainActivity : AppCompatActivity() {

    private var modelParser: ModelParser? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        ..
        modelParser = ModelParser()
    }

    override fun onDestroy() {
        modelParser?.destroy()
        super.onDestroy()
    }

    private fun convert() {
        modelParser?.convertDffToGltfAsync(dffFilePath, gltfFilePath, /* optional */ txdFilePath, /* optional */ viewType) { parseResult ->
            // Handling the callback
        }
    }
}
```

See the sample application, where an example of using the library is implemented: https://github.com/Lime-blur/rwparser/tree/main/sample

## Rights
- rwtools: https://github.com/aap/rwtools
- dff_converter: https://github.com/SimoSbara/dff_converter
