# Assignment 2 -- "3D Fan"

This is a 3D fan with a base, pole, hub, and four blades. It is made out of one shared cube. It works on Android, Windows, and WebGL using OpenGL ES 3.0. The C++ code is shared in the `Scene/` folder.

- Tap / click: toggle the fan **ON** / **OFF** (logged as `Fan ON` / `Fan OFF`)
- Drag / swipe: spin the fan faster, proportional to swipe speed; release to
  return to base speed

See `Scene/Fan.h` and `Scene/Fan.cpp` for the implementation and a full
explanation of the part hierarchy and gesture logic.

## Project layout

```
CMakeLists.txt                  <- Desktop entry point (GLFW + GLEW + GLM)
script_build_and_run.bat        <- Desktop convenience wrapper
build_web.sh / build_web.bat    <- Web entry point (Emscripten)
cmake/ImportDependencies.cmake
.github/workflows/
  desktop.yml                   <- builds the desktop target (Linux + Windows)
  android.yml                   <- builds the Android debug APK
  pages.yml                     <- builds the web target, deploys to GitHub Pages
Scene/                          <- shared by ALL THREE platforms
  Platform.h, Model.h, Renderer.h/.cpp, ShaderHelper.h
  Transform.h/.cpp              <- provided, unmodified matrix stack
  Fan.h/.cpp                    <- the assignment implementation
  main.cpp                      <- Desktop/Web entry point + mouse->touch mapping
android/                        <- Android Studio / Gradle project
  app/src/main/assets/shader/FanVertex.glsl, FanFragment.glsl
```


---

## 1. Desktop (Windows, GLFW + GLEW + GLM)

**Prerequisites:** CMake 3.15+, Visual Studio 2022 (or another CMake
generator), internet access on first configure (GLFW, GLEW, and GLM are
fetched automatically via `FetchContent` -- nothing to install manually).

**Build & run (one step):**

```bat
script_build_and_run.bat
```

**Or manually:**

```bat
cmake -S . -B build_desktop -G "Visual Studio 17 2022" -A x64
cmake --build build_desktop --config Debug
build_desktop\Debug\Fan3D.exe
```

The first configure downloads dependencies and can take a few minutes.
Output binary: `build_desktop/Debug/Fan3D.exe` (shaders are copied next to
it automatically as a post-build step). Controls: click to toggle the fan,
click-drag to spin it faster, `ESC` to quit.

> Non-Windows desktops (Linux/macOS) can use the same `CMakeLists.txt` with
> a Makefile/Ninja generator, e.g. `cmake -S . -B build && cmake --build build`.

---

## 2. Android

**Prerequisites:** Android Studio (current stable), Android SDK
(compileSdk/targetSdk 36, minSdk 34), NDK + CMake components installed via
the SDK Manager.

**Build & run:**

1. Open the `android/` folder in Android Studio as a project.
2. Let Gradle sync (this also configures the NDK/CMake build for
   `android/app/src/main/cpp/CMakeLists.txt`, which fetches GLM the same
   way the desktop build does).
3. Press **Run** with a connected device or emulator (API 34+, OpenGL ES
   3.0 capable).

**Or from the command line** (requires a system-installed Gradle matching
`android/gradle/wrapper/gradle-wrapper.properties` -- this repo does not
commit the `gradlew` wrapper binaries; see `.github/workflows/android.yml`
for the exact CI equivalent):

```bash
cd android
gradle assembleDebug
```

Output: `android/app/build/outputs/apk/debug/app-debug.apk`. Install with
`adb install -r app/build/outputs/apk/debug/app-debug.apk` or run it
directly from Android Studio. Logs: `adb logcat -s Fan3D`.

---

## 3. WebGL (Emscripten)

**Prerequisites:** [Emscripten SDK](https://emscripten.org/docs/getting_started/)
installed and activated in your shell (`source /path/to/emsdk/emsdk_env.sh`
on Linux/macOS, or `emsdk_env.bat` on Windows), plus `git` in `PATH` (used
once to fetch header-only GLM into `.emscripten_deps/glm`) and Python 3 to
serve the output.

**Build:**

```bash
./build_web.sh        # Linux/macOS
build_web.bat          # Windows
```

**Serve & run:**

```bash
cd web
python3 -m http.server 8080
```

Then open <http://localhost:8080/> in a browser. Controls: click to toggle
the fan, click-drag to spin it faster.

---

## Notes

- `Scene/Transform.h` / `Scene/Transform.cpp` are provided and must not be
  modified -- see the header comment for the matrix-stack conventions
  (post-multiply, parent-first, radians).
- Build output directories (`build_desktop/`, `web/`, `.emscripten_deps/`,
  `android/app/build/`, `android/app/.cxx/`, `android/.gradle/`,
  `android/.idea/`) are generated and are not committed to this repository.
