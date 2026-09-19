include(FetchContent)

# alandtse/CommonLibSSE-NG v8.1.0: 1.7.x layouts, Address Library formats 1/2/5,
# and the corrected SKSEPluginInfo v5 compatibility flag. Keep builds reproducible.
FetchContent_Declare(CommonLibSSE
    GIT_REPOSITORY https://github.com/alandtse/CommonLibSSE-NG.git
    GIT_TAG 3c0f5a87c3b166c9a6712d5c3bd180e9ac5ad0fd
    GIT_SUBMODULES extern/openvr
)

# SearchUI has always targeted all three runtime families.
set(ENABLE_SKYRIM_SE ON CACHE BOOL "Build with Skyrim SE support")
set(ENABLE_SKYRIM_AE ON CACHE BOOL "Build with Skyrim AE support")
set(ENABLE_SKYRIM_VR ON CACHE BOOL "Build with Skyrim VR support")

# CommonLib uses the same BUILD_TESTS option as the consumer. Its own extensive
# test suite is not part of the SearchUI target.
set(_searchui_build_tests ${BUILD_TESTS})
set(BUILD_TESTS OFF)
FetchContent_MakeAvailable(CommonLibSSE)
set(BUILD_TESTS ${_searchui_build_tests})
include(${commonlibsse_SOURCE_DIR}/cmake/CommonLibSSE.cmake)
