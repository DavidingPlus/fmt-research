cmake_minimum_required (VERSION 3.12)

project (fmt-search)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED true)

# configuring compiler cross platform
if (CMAKE_HOST_WIN32)
    include (config-win32.cmake)

elseif (CMAKE_HOST_UNIX)
    include (config-unix.cmake)

else ()
    message (FATAL_ERROR "Unknown platform.")

endif ()

# add snippet
add_subdirectory (snippet)
