#!/usr/bin/env sh
# =============================================================================
# build_web.sh - Build Assignment 2 "3D Fan" for WebGL using Emscripten
#
# Prerequisites:
#   1. Install the Emscripten SDK:  https://emscripten.org/docs/getting_started/
#   2. Activate it in your shell:   source /path/to/emsdk/emsdk_env.sh
#   3. Run this script from the project root:  ./build_web.sh
#
# Output: web/index.html  (+ index.js, index.wasm, index.data)
# Serve:  cd web && python3 -m http.server 8080
#
# GLM is header-only and vendored nowhere -- emcc is pointed at a local
# checkout fetched the first time this script runs (cached in
# .emscripten_deps/glm) so no network access is required afterwards.
# =============================================================================

set -e

mkdir -p web

# --- Fetch GLM once (header-only; Scene/Transform.h needs it on every
#     platform, including this Emscripten build) ----------------------------
GLM_DIR=".emscripten_deps/glm"
if [ ! -d "$GLM_DIR" ]; then
    echo "Fetching GLM (header-only) ..."
    mkdir -p .emscripten_deps
    git clone --depth 1 --branch 1.0.1 https://github.com/g-truc/glm.git "$GLM_DIR"
fi

# --- GLFW backend (default) -------------------------------------------------
#
# Key flags:
#   -DUSE_GLFW            Select the GLFW backend in main.cpp / Platform.h
#   -s USE_GLFW=3         Pull in GLFW 3 from Emscripten ports (window + input)
#   -s USE_WEBGL2=1       Enable WebGL 2.0 (= OpenGL ES 3.0)
#   -s FULL_ES3=1         Full OpenGL ES 3.0 feature set (VAOs, layout qualifiers)
#   -s WASM=1             Emit WebAssembly (not asm.js)
#   --preload-file        Pack shader directory into the Emscripten virtual FS
# ---------------------------------------------------------------------------
emcc -std=c++17 \
    Scene/main.cpp \
    Scene/Renderer.cpp \
    Scene/Fan.cpp \
    Scene/Transform.cpp \
    -IScene \
    -I"$GLM_DIR" \
    -DUSE_GLFW \
    -s USE_GLFW=3 \
    -s USE_WEBGL2=1 \
    -s FULL_ES3=1 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file android/app/src/main/assets/shader@assets/shader \
    -o web/index.html

echo ""
echo "Build succeeded."
echo "Run:  cd web && python3 -m http.server 8080"
echo "Open: http://localhost:8080/"
