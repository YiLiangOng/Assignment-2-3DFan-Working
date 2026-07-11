# ===========================================================================
# ImportDependencies.cmake
#
# Downloads and configures third-party libraries needed for the desktop
# build. Android and Emscripten have their own GL implementations and do
# not use this file (Android fetches its own copy of GLM separately, see
# android/app/src/main/cpp/CMakeLists.txt).
#
# Dependencies:
#   GLFW  3.3.8  - window creation and input
#   GLEW  2.2.0  - OpenGL extension loader (static build)
#   GLM   1.0.1  - header-only math library used by Scene/Transform.*
#
# Mirrors the pattern from Chapter 2 tutorial 1's cmake/ImportDependencies.cmake.
# ===========================================================================

include(FetchContent)

function(importDependencies)

    if(ANDROID)
        # Android has GLESv3 built-in and fetches GLM on its own; nothing
        # to do here.
        return()
    endif()

    # -----------------------------------------------------------------------
    # GLFW - window and input management
    # -----------------------------------------------------------------------
    message(STATUS "[ImportDependencies] Fetching GLFW 3.3.8 ...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)

    # -----------------------------------------------------------------------
    # GLEW - OpenGL extension loader (static)
    # -----------------------------------------------------------------------
    message(STATUS "[ImportDependencies] Fetching GLEW 2.2.0 ...")
    FetchContent_Declare(
        glew
        GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
    set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(glew-cmake_BUILD_STATIC ON  CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glew)

    # -----------------------------------------------------------------------
    # GLM - header-only math library backing Scene/Transform.*
    # -----------------------------------------------------------------------
    message(STATUS "[ImportDependencies] Fetching GLM 1.0.1 ...")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(glm)

    message(STATUS "[ImportDependencies] All desktop dependencies ready.")

endfunction()
