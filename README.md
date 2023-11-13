# rwparser
**rwparser** is Android library for parsing RenderWare dff and txd models of Grand Theft Auto. 
## Gradle Dependency
Add this dependency to your module's `build.gradle` file:

```gradle
dependencies {
  ..
  implementation 'com.github.Lime-blur:rwparser:latest_version'
}
```

## Usage
In order to parse a `.dff` file, you simply need to create a `ModelParser()` object and call the appropriate method:

```kotlin
val modelParser = ModelParser()
modelParser.putDffDumpIntoFile(inFilePath, outFilePath)
```

Same for `.txd` file:

```kotlin
val modelParser = ModelParser()
modelParser.putTxdDumpIntoFile(inFilePath, outFilePath)
```

See the sample application, where an example of using the library is implemented: https://github.com/Lime-blur/rwparser/tree/main/sample
