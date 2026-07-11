@echo off
REM =============================================================================
REM build_web.bat - Build Assignment 2 "3D Fan" for WebGL using Emscripten (Windows)
REM
REM Prerequisites:
REM   1. Install Emscripten SDK: https://emscripten.org/docs/getting_started/
REM   2. Run emsdk_env.bat in the same shell before this script.
REM   3. git must be in PATH (used once to fetch header-only GLM).
REM   4. Run this script from the project root.
REM
REM Output: web\index.html (+ index.js, index.wasm, index.data)
REM Serve:  cd web && python -m http.server 8080
REM =============================================================================

IF NOT EXIST web MKDIR web

REM -- Fetch GLM once (header-only; needed by Scene/Transform.h) --------------
IF NOT EXIST .emscripten_deps\glm (
    echo Fetching GLM ...
    IF NOT EXIST .emscripten_deps MKDIR .emscripten_deps
    git clone --depth 1 --branch 1.0.1 https://github.com/g-truc/glm.git .emscripten_deps\glm
)

REM --- GLFW backend (default) -----------------------------------------------
emcc -std=c++17 ^
     Scene/main.cpp ^
     Scene/Renderer.cpp ^
     Scene/Fan.cpp ^
     Scene/Transform.cpp ^
     -IScene ^
     -I.emscripten_deps/glm ^
     -DUSE_GLFW ^
     -s USE_GLFW=3 ^
     -s USE_WEBGL2=1 ^
     -s FULL_ES3=1 ^
     -s WASM=1 ^
     -s ALLOW_MEMORY_GROWTH=1 ^
     --preload-file android/app/src/main/assets/shader@assets/shader ^
     -o web/index.html

IF %ERRORLEVEL% NEQ 0 (
    echo Build FAILED.
    pause
    exit /b 1
)

echo.
echo Build succeeded.
echo Run:  cd web ^&^& python -m http.server 8080
echo Open: http://localhost:8080/
pause
