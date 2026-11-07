# PS5 native title (cmake/toolchains/PS5.cmake of the outer repo): SDL2 and OpenGL come from the
# ps5-opengl SDK prefixes; the GL archive itself is linked when the title is staged.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# ========= ImGui =========
find_package(SDL2 CONFIG REQUIRED)
# The executable is linked with `ld.lld -r`; the GL archive and the system stub libraries SDL2 lists
# (PS5OpenGLCore33, ScePad, SceUserService, SceSystemService) are linked when the title is staged.
set_target_properties(SDL2::SDL2-static PROPERTIES INTERFACE_LINK_LIBRARIES "")
target_link_libraries(ImGui PUBLIC SDL2::SDL2-static)
target_include_directories(ImGui PUBLIC ${PS5_OPENGL_PREFIX}/include)
# ImGui's built-in gl3w loader dlopens libGL, which a native title cannot do; GL is linked statically.
target_compile_definitions(ImGui PRIVATE IMGUI_IMPL_OPENGL_LOADER_CUSTOM GL_GLEXT_PROTOTYPES=1)
target_compile_options(ImGui PRIVATE "SHELL:-include GL/gl.h" "SHELL:-include GL/glext.h")
#=================== nlohmann-json ===================
find_package(nlohmann_json QUIET)
if (NOT ${nlohmann_json_FOUND})
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

#=================== tinyxml2 ===================
find_package(tinyxml2 QUIET)
if (NOT ${tinyxml2_FOUND})
    set(tinyxml2_BUILD_TESTING OFF)
    FetchContent_Declare(
        tinyxml2
        GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
        GIT_TAG 10.0.0
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(tinyxml2)
endif()

#=================== spdlog ===================
# spdlog's registry builds a colored stdout logger on first use; the title has no console and that
# sink crashed it during startup, so only the file logger Context::InitLogging installs exists.
add_compile_definitions(SPDLOG_DISABLE_DEFAULT_LOGGER)
find_package(spdlog QUIET)
if (NOT ${spdlog_FOUND})
    add_compile_definitions(SPDLOG_PREVENT_CHILD_FD) # disables fcntl(FD_CLOEXEC)
    set(SPDLOG_BUILD_EXAMPLE OFF)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(spdlog)
endif()


#=================== zlib ===================
# The payload SDK ships no zlib; libzip needs one, so build it statically from source.
FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.3.1
)
set(ZLIB_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(zlib)
add_library(ZLIB::ZLIB ALIAS zlibstatic)
set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} CACHE PATH "" FORCE)
set(ZLIB_LIBRARY zlibstatic CACHE STRING "" FORCE)
target_include_directories(zlibstatic INTERFACE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})

#=================== libzip ===================
find_package(libzip QUIET)
if (NOT ${libzip_FOUND})
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(BUILD_TOOLS OFF)
    set(BUILD_REGRESS OFF)
    set(BUILD_EXAMPLES OFF)
    set(BUILD_DOC OFF)
    set(BUILD_OSSFUZZ OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(ENABLE_ZSTD OFF)
    set(LIBZIP_DO_INSTALL OFF)
    # The PS5 toolchain's try_compile builds static libraries, so libzip's function checks cannot
    # fail at link time. Pin every optional function it probes to "absent" (libzip has a portable
    # fallback for each); the ones the libc does have are just used through that fallback.
    foreach(missing_function IN ITEMS HAVE__CLOSE HAVE__DUP HAVE__FDOPEN HAVE__FILENO HAVE__SETMODE
            HAVE__SNPRINTF HAVE__SNPRINTF_S HAVE__SNWPRINTF_S HAVE__STRDUP HAVE__STRICMP HAVE__STRTOI64
            HAVE__STRTOUI64 HAVE__UNLINK HAVE_ARC4RANDOM HAVE_CLONEFILE HAVE_EXPLICIT_BZERO HAVE_EXPLICIT_MEMSET
            HAVE_GETPROGNAME HAVE_LOCALTIME_S HAVE_MEMCPY_S HAVE_SETMODE HAVE_SNPRINTF_S HAVE_STRERROR_S
            HAVE_STRERRORLEN_S HAVE_STRICMP HAVE_STRNCPY_S HAVE_FTS_OPEN)
        set(${missing_function} OFF CACHE INTERNAL "")
    endforeach()
    FetchContent_Declare(
        libzip
        GIT_REPOSITORY https://github.com/nih-at/libzip.git
        GIT_TAG v1.10.1
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(libzip)
    list(APPEND ADDITIONAL_LIB_INCLUDES ${libzip_SOURCE_DIR}/lib ${libzip_BINARY_DIR})
endif()
